#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

BOOL APIENTRY DllMain(HMODULE /*module_*/, DWORD ul_reason_for_call, LPVOID /*reserved_*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		utils::hook::jump(reinterpret_cast<size_t>(&printf), game::Com_Printf_NoFilter);
		component_loader::post_unpack();
	}

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		component_loader::pre_destroy();
	}

	return TRUE;
}