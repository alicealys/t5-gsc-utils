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

	std::unordered_map<unsigned int, std::string> canonical_string_table;

	namespace
	{
		utils::hook::detour g_shutdown_game_hook;
		utils::hook::detour sl_get_canonical_string_hook;

		std::vector<std::function<void()>> shutdown_callbacks;
		void g_shutdown_game_stub(const int free_scripts)
		{
			if (free_scripts)
			{
				canonical_string_table.clear();
			}

			for (const auto& callback : shutdown_callbacks)
			{
				callback();
			}

			return g_shutdown_game_hook.invoke<void>(free_scripts);
		}

		unsigned int sl_transfer_canonical_string_stub(game::scriptInstance_t inst, unsigned int string_value)
		{
			const auto result = sl_get_canonical_string_hook.invoke<unsigned int>(inst, string_value);
			const auto str = game::SL_ConvertToString(string_value, inst);
			if (str)
			{
				canonical_string_table[result] = game::SL_ConvertToString(string_value, inst);
			}

			return result;
		}
	}

	void on_shutdown(const std::function<void()>& callback)
	{
		shutdown_callbacks.push_back(callback);
	}

	class component final : public component_interface
	{
	public:
		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			g_shutdown_game_hook.create(SELECT_VALUE(0x607700, 0x540950), g_shutdown_game_stub);
			sl_get_canonical_string_hook.create(SELECT_VALUE(0x5F3F40, 0x622530), sl_transfer_canonical_string_stub);
		}
	};
}

REGISTER_COMPONENT(scripting::component)
