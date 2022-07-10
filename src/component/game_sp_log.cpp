#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

namespace game_sp_log
{
	namespace
	{
		// Use these dvars for SP binary only
		const game::dvar_t** g_log = reinterpret_cast<const game::dvar_t**>(0x3023B20);
		const game::dvar_t** g_logSync = reinterpret_cast<const game::dvar_t**>(0x3023B2C);

		void g_init_game_stub()
		{
			// G_RegisterDvars
			utils::hook::invoke<void>(0x7E1C10);

			game::Com_Printf(game::CON_CHANNEL_SERVER, "------- Game Initialization -------\n");

			const std::string log_file = (*g_log)->current.string;

			if (log_file.empty())
			{
				game::Com_Printf(game::CON_CHANNEL_SERVER, "Not logging to disk.\n");
				return;
			}

			const auto mode = (*g_logSync)->current.enabled ? game::FS_APPEND_SYNC : game::FS_APPEND;

			game::FS_FOpenFileByMode(log_file.data(), game::logFile, mode);

			if (*game::logFile == 0)
			{
				game::Com_PrintWarning(game::CON_CHANNEL_SERVER, "WARNING: Couldn't open logfile: %s\n", log_file.data());
				return;
			}

			char info[1024]{};
			game::SV_GetServerinfo(info, sizeof(info));

			game::G_LogPrintf("------------------------------------------------------------\n");
			game::G_LogPrintf("InitGame: %s\n", info);

		}

		void g_shutdown_game_stub(int free_scripts)
		{
			utils::hook::invoke<void>(0x607700, free_scripts); // G_ShutdownGame

			if (*game::logFile != 0)
			{
				game::G_LogPrintf("ShutdownGame:\n");
				game::G_LogPrintf("------------------------------------------------------------\n");
				game::FS_FCloseFile(*game::logFile);
				*game::logFile = 0;
			}
		}

	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (game::environment::is_mp())
			{
				return;
			}

			utils::hook::call(0x51E97F, g_init_game_stub);

			utils::hook::call(0x4FA28A, g_shutdown_game_stub);
			utils::hook::call(0x57EF4B, g_shutdown_game_stub);
		}
	};
}

REGISTER_COMPONENT(game_sp_log::component)
