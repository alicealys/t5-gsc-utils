#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"

#include <utils/hook.hpp>

namespace patches
{
	int g_get_client_score_stub(const int clientNum)
	{
		game::Com_Printf(game::CON_CHANNEL_DONT_FILTER, "%5i ", utils::hook::invoke<int>(0x4691D0, clientNum));

		game::Com_Printf(game::CON_CHANNEL_DONT_FILTER, "%3i ", game::SV_IsTestClient(clientNum));

		return 0;
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (game::environment::is_sp())
			{
				return;
			}

			utils::hook::call(0x876158, g_get_client_score_stub);
			utils::hook::nop(0x876165, 5); // G_GetClientScore print (done in stub ^^)

			utils::hook::set<const char*>(0x8760E8, "num score bot ping guid   name            lastmsg address               qport rate\n");
			utils::hook::set<const char*>(0x8760F4, "--- ----- --- ---- ---------- --------------- ------- --------------------- ------ -----\n");
		}
	};
}

REGISTER_COMPONENT(patches::component)
