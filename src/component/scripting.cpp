#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "game/scripting/event.hpp"
#include "game/scripting/execution.hpp"

#include "scripting.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace scripting
{
	std::unordered_map<int, std::unordered_map<std::string, int>> fields_table;

	std::unordered_map<std::string, game::BuiltinMethodDef> method_map;
	std::unordered_map<std::string, game::BuiltinFunctionDef> function_map;

	std::unordered_map<std::string, std::unordered_map<std::string, const char*>> script_function_table;
	std::unordered_map<std::string, std::vector<std::pair<std::string, const char*>>> script_function_table_sort;
	std::unordered_map<const char*, std::pair<std::string, std::string>> script_function_table_rev;

	namespace
	{
		utils::hook::detour scr_add_class_field_hook;
		utils::hook::detour g_shutdown_game_hook;

		void scr_add_class_field_stub(game::scriptInstance_t inst, unsigned int classnum, char const* name, unsigned int offset)
		{
			if (fields_table[classnum].find(name) == fields_table[classnum].end())
			{
				fields_table[classnum][name] = offset;
			}

			scr_add_class_field_hook.invoke<void>(inst, classnum, name, offset);
		}

		game::BuiltinMethodDef get_method(const std::string& name)
		{
			game::BuiltinMethodDef method{};

			auto pName = name.data();
			int arg = 0;

			const auto func = game::Scr_GetMethod(&pName, &arg, &arg, &arg);
		
			if (func)
			{
				method.actionFunc = reinterpret_cast<script_function>(func);
				method.actionString = pName;
			}

			return method;
		}

		void load_functions()
		{
			
		}

		utils::hook::detour scr_load_script_hook;

		std::vector<std::function<void()>> shutdown_callbacks;
		void g_shutdown_game_stub(const int free_scripts)
		{
			for (const auto& callback : shutdown_callbacks)
			{
				callback();
			}

			return g_shutdown_game_hook.invoke<void>(free_scripts);
		}
	}

	void on_shutdown(const std::function<void()>& callback)
	{
		shutdown_callbacks.push_back(callback);
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			//g_shutdown_game_hook.create(SELECT(0x60DCF0, 0x688A40), g_shutdown_game_stub);

			//scr_add_class_field_hook.create(SELECT(0x6B7620, 0x438AD0), scr_add_class_field_stub);
			//scr_post_load_scripts_hook.create(SELECT(0x642EB0, 0x425F80), post_load_scripts_stub);

			//load_functions();
		}
	};
}

//REGISTER_COMPONENT(scripting::component)
