#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"

#include "gsc.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

namespace gsc
{
	std::unordered_map<std::string, function_t> functions;
	std::unordered_map<std::string, function_t> methods;

	std::unordered_map<std::string, void*> function_wraps;
	std::unordered_map<std::string, void*> method_wraps;

	namespace
	{
		utils::hook::detour get_function_hook;
		utils::hook::detour get_method_hook;

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

		script_function get_function_stub(const char** name, int* type)
		{
			const auto iter = function_wraps.find(*name);
			if (iter != function_wraps.end())
			{
				return reinterpret_cast<script_function>(iter->second);
			}

			return get_function_hook.invoke<script_function>(name, type);
		}

		script_function get_method_stub(const char** name, int* type)
		{
			const auto iter = method_wraps.find(*name);
			if (iter != method_wraps.end())
			{
				return reinterpret_cast<script_function>(iter->second);
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

		// Scr_NotifyId doesn't exist, Scr_NotifyNum_Internal calls FindVariableId to get the variable id from entnum, classnum & clientNum
		// to not have to recreate Scr_NotifyId we simply make FindVariableId return `entnum` (which in this case will be the id) if `clientNum` == -1
		unsigned int find_variable_id_stub(int inst, int entnum, unsigned int classnum, int client_num)
		{
			if (client_num == -1)
			{
				return entnum;
			}

			return utils::hook::invoke<unsigned int>(SELECT_VALUE(0x5E96E0, 0x40BEF0), inst, entnum, classnum, client_num);
		}
	}

	void call_function(const function_t& function)
	{
		const auto args = get_arguments();

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

	void call_method(const function_t& method, const game::scr_entref_t entref)
	{
		const auto args = get_arguments();

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

	class component final : public component_interface
	{
	public:
		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
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

			utils::hook::call(SELECT_VALUE(0x41D2B5, 0x416325), find_variable_id_stub);

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

			gsc::function::add("debug::get_var_count", []()
			{
				auto count = 0;

				if (game::environment::is_sp())
				{
					for (auto i = 1; i < 0x5FFE; i++)
					{
						const auto var = game::scr_VarGlob->variableList_mp[i];
						count += var.w.status != 0;
					}
				}
				else
				{
					for (auto i = 1; i < 0x7FFE; i++)
					{
						const auto var = game::scr_VarGlob->variableList_mp[i];
						count += var.w.status != 0;
					}
				}

				return count;
			});

			gsc::function::add("toint", [](const std::string& str, const scripting::variadic_args& va)
			{
				auto radix = 10;
				if (!va.empty())
				{
					radix = va[0];
				}

				return static_cast<int>(std::strtoull(str.data(), nullptr, radix));
			});

			gsc::function::add("os::date", [](const scripting::variadic_args& va)
			{
				std::string format = "%Y-%m-%dT%H:%M:%S%z";
				if (!va.empty())
				{
					format = va[0].as<std::string>();
				}

				tm ltime{};
				char timestamp[MAX_PATH] = {0};
				const auto time = _time64(nullptr);

				_localtime64_s(&ltime, &time);
				std::strftime(timestamp, sizeof(timestamp) - 1, format.data(), &ltime);

				return std::string(timestamp);
			});
		}
	};
}

REGISTER_COMPONENT(gsc::component)
