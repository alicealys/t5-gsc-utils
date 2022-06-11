#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"

#include "gsc.hpp"

#include "scheduler.hpp"
#include "scripting.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>

namespace gsc
{
    namespace
    {
        utils::hook::detour get_function_hook;
        utils::hook::detour get_method_hook;

        std::unordered_map<std::string, function_t> functions;
        std::unordered_map<std::string, function_t> methods;

        std::unordered_map<std::string, void*> function_wraps;
        std::unordered_map<std::string, void*> method_wraps;

        std::vector<scripting::script_value> get_arguments()
        {
            std::vector<scripting::script_value> args;

            for (auto i = 0; static_cast<unsigned int>(i) < game::scr_VmPub->outparamcount; i++)
            {
                const auto value = game::scr_VmPub->top[-i];
                args.push_back(value);
            }

            return args;
        }

        void return_value(const scripting::script_value& value)
        {
            if (game::scr_VmPub->outparamcount)
            {
                game::Scr_ClearOutParams(game::SCRIPTINSTANCE_SERVER);
            }

            scripting::push_value(value);
        }

        void call_function(const char* name)
        {
            if (functions.find(name) == functions.end())
            {
                return;
            }

            const auto args = get_arguments();
            const auto& function = functions[name];

            try
            {
                const auto value = function(args);
                return_value(value);
            }
            catch (const std::exception& e)
            {
                game::Scr_Error(game::SCRIPTINSTANCE_SERVER, e.what(), false);
            }
        }

        void call_method(const char* name, const game::scr_entref_t entref)
        {
            if (methods.find(name) == methods.end())
            {
                return;
            }

            const auto args = get_arguments();
            const auto& method = methods[name];

            try
            {
                const scripting::entity entity = game::Scr_GetEntityId(
                    game::SCRIPTINSTANCE_SERVER, entref.entnum, entref.classnum, 0);

                std::vector<scripting::script_value> args_{};
                args_.push_back(entity);
                for (const auto& arg : args)
                {
                    args_.push_back(arg);
                }

                const auto value = method(args_);
                return_value(value);
            }
            catch (const std::exception& e)
            {
                game::Scr_Error(game::SCRIPTINSTANCE_SERVER, e.what(), false);
            }
        }

        void* wrap_function_call(const std::string& name)
        {
            const auto name_ = utils::memory::get_allocator()->duplicate_string(name);
            return utils::hook::assemble([name_](utils::hook::assembler& a)
            {
                a.pushad();
                a.push(name_);
                a.call(call_function);
                a.add(esp, 0x4);
                a.popad();

                a.ret();
            });
        }

        void* wrap_method_call(const std::string& name)
        {
            const auto name_ = utils::memory::get_allocator()->duplicate_string(name);
            return utils::hook::assemble([name_](utils::hook::assembler& a)
            {
                a.pushad();
                a.push(dword_ptr(esp, 0x24));
                a.push(name_);
                a.call(call_method);
                a.add(esp, 0x8);
                a.popad();

                a.ret();
            });
        }

        script_function get_function_stub(const char** name, int* type)
        {
            if (function_wraps.find(*name) != function_wraps.end())
            {
                return reinterpret_cast<script_function>(function_wraps[*name]);
            }

            return get_function_hook.invoke<script_function>(name, type);
        }

        script_function get_method_stub(const char** name, int* type)
        {
            if (method_wraps.find(*name) != method_wraps.end())
            {
                return reinterpret_cast<script_function>(method_wraps[*name]);
            }

            return get_method_hook.invoke<script_function>(name, type);
        }

        void print(int, const char* fmt, ...)
        {
            char buffer[2048];

            va_list ap;
            va_start(ap, fmt);

            vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, ap);

            va_end(ap);

            printf("%s", buffer);
        }

        utils::hook::detour scr_settings_hook;
        void scr_settings_stub(int /*developer*/, int developer_script, int /*abort_on_error*/, int inst)
        {
            scr_settings_hook.invoke<void>(developer_script, developer_script, 0, inst);
        }

        utils::hook::detour scr_get_builtin_hook;
        unsigned int scr_get_builtin_stub(int inst, game::sval_u func_name)
        {
            const auto type = *reinterpret_cast<uint8_t*>(func_name.block);
            if (type != 28)
            {
                return scr_get_builtin_hook.invoke<unsigned int>(inst, func_name);
            }

            const auto func_namea = *reinterpret_cast<void**>(reinterpret_cast<size_t>(func_name.block) + 4);
            const auto typea = *reinterpret_cast<uint8_t*>(func_namea);
            if (typea != 20)
            {
                return scr_get_builtin_hook.invoke<unsigned int>(inst, func_name);
            }

            const auto func_nameb = *reinterpret_cast<void**>(reinterpret_cast<size_t>(func_namea) + 4);
            const auto typeb = *reinterpret_cast<uint8_t*>(func_nameb);

            if (typeb == 23) // script::function type call
            {
                const auto namespace_ = game::SL_ConvertToString(
                    *reinterpret_cast<unsigned int*>(reinterpret_cast<size_t>(func_nameb) + 4), game::SCRIPTINSTANCE_SERVER);
                const auto name = game::SL_ConvertToString(
                    *reinterpret_cast<unsigned int*>(reinterpret_cast<size_t>(func_nameb) + 8), game::SCRIPTINSTANCE_SERVER);

                const auto full_name = utils::string::va("%s::%s", namespace_, name);
                if (functions.find(full_name) != functions.end())
                {
                    return game::SL_GetString(full_name, 0, game::SCRIPTINSTANCE_SERVER);
                }
            }

            return scr_get_builtin_hook.invoke<unsigned int>(inst, func_name);
        }
    }

    namespace function
    {
        void add_internal(const std::string& name, const function_t& function)
        {
            const auto name_ = utils::string::to_lower(name);
            functions[name_] = function;
            const auto call_wrap = wrap_function_call(name_);
            function_wraps[name_] = call_wrap;
        }
    }

    namespace method
    {
        void add_internal(const std::string& name, const function_t& method)
        {
            const auto name_ = utils::string::to_lower(name);
            methods[name_] = method;
            const auto call_wrap = wrap_method_call(name_);
            method_wraps[name_] = call_wrap;
        }
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            // Don't com_error on gsc errors
            utils::hook::nop(SELECT_VALUE(0x5A17E1, 0x4D9BB1), 5);
            utils::hook::jump(SELECT_VALUE(0x5DFC40, 0x568B90), print);

            scr_settings_hook.create(SELECT_VALUE(0x4CEEA0, 0x55D010), scr_settings_stub);

            get_function_hook.create(utils::hook::extract<size_t>(SELECT_VALUE(0x8A02FB, 0x8DE11B) + 1), get_function_stub);
            get_method_hook.create(utils::hook::extract<size_t>(SELECT_VALUE(0x8A052E, 0x8DE34E) + 1), get_method_stub);

            scr_get_builtin_hook.create(SELECT_VALUE(0x4ACAC0, 0x411490), scr_get_builtin_stub);

            // \n******* script runtime error *******\n%s\n
            utils::hook::set<char>(SELECT_VALUE(0x9FC5C0 + 40, 0xAABA68 + 40), '\n');
            utils::hook::set<char>(SELECT_VALUE(0x9FC5C0 + 41, 0xAABA68 + 41), '\0');

            gsc::function::add("array", [](const scripting::variadic_args& va)
            {
                scripting::array array{};

                for (const auto& arg : va)
                {
                    array.push(arg);
                }

                return array;
            });

            gsc::function::add_multiple([](const scripting::script_value& value)
            {
                return value.type_name();
            }, "typeof", "type");
        }
    };
}

REGISTER_COMPONENT(gsc::component)
