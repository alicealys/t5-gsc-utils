#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "plugin.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/nt.hpp>

PLUTONIUM_API plutonium::sdk::plugin* PLUTONIUM_CALLBACK on_initialize()
{
	return plugin::get();
}

BOOL APIENTRY DllMain(HMODULE module, DWORD ul_reason_for_call, LPVOID /*reserved*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		utils::nt::library::set_current_handle(module);
	}

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		component_loader::on_shutdown();
	}

	return TRUE;
}
