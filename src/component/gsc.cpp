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

        std::unordered_map<std::string, std::function<scripting::script_value(const scripting::function_arguments& args)>> functions;
        std::unordered_map<std::string, std::function<scripting::script_value(const scripting::function_arguments& args)>> methods;

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
            if (*name == "print"s)
            {
                MessageBoxA(nullptr, "", "", 0);
            }

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
            scr_settings_hook.invoke<void>(SELECT_VALUE(0x0, 0x55D010), developer_script, developer_script, 0, inst);
        }
    }

    namespace function
    {
        template <typename F>
        void add(const std::string& name, F f)
        {
            const auto wrap = wrap_function(f);
            functions[name] = wrap;
            const auto call_wrap = wrap_function_call(name);
            function_wraps[name] = call_wrap;
        }
    }

    namespace method
    {
        template <typename F>
        void add(const std::string& name, F f)
        {
            const auto wrap = wrap_function(f);
            methods[name] = wrap;
            const auto call_wrap = wrap_method_call(name);
            method_wraps[name] = call_wrap;
        }
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            // Don't com_error on gsc errors
            utils::hook::nop(SELECT_VALUE(0x0, 0x4D9BB1), 5);
            utils::hook::jump(SELECT_VALUE(0x0, 0x568B90), print);

            scr_settings_hook.create(SELECT_VALUE(0x0, 0x55D010), scr_settings_stub);
            get_function_hook.create(SELECT_VALUE(0x0, 0x465E20), get_function_stub);
            get_method_hook.create(SELECT_VALUE(0x0, 0x555580), get_method_stub);

            function::add("print_", [](const scripting::variadic_args& va)
            {
                for (const auto& arg : va)
                {
                    printf("%s\t", arg.to_string().data());
                }
                printf("\n");
            });

            function::add("writefile", [](const std::string& file, const std::string& data,
                const scripting::variadic_args& va)
            {
                auto append = false;

                if (va.size() > 0)
                {
                    append = va[0];
                }

                return utils::io::write_file(file, data, append);
            });

            function::add("appendfile", [](const std::string& file, const std::string& data)
            {
                return utils::io::write_file(file, data, true);
            });

            function::add("fileexists", utils::io::file_exists);
            function::add("movefile", utils::io::move_file);
            function::add("filesize", utils::io::file_size);
            function::add("createdirectory", utils::io::create_directory);
            function::add("directoryexists", utils::io::directory_exists);
            function::add("directoryisempty", utils::io::directory_is_empty);
            function::add("listfiles", utils::io::list_files);
            function::add("removefile", utils::io::remove_file);
            function::add("readfile", static_cast<std::string(*)(const std::string&)>(utils::io::read_file));
        }
    };
}

REGISTER_COMPONENT(gsc::component)
