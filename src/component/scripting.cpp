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
		utils::hook::detour g_shutdown_game_hook;

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
			g_shutdown_game_hook.create(SELECT_VALUE(0x607700, 0x540950), g_shutdown_game_stub);
		}
	};
}

REGISTER_COMPONENT(scripting::component)
