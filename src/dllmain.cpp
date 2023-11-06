#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

namespace
{
	void printf_stub(const char* fmt, ...)
	{
		char buffer[2048]{};

		va_list ap;
		va_start(ap, fmt);

		vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, ap);

		va_end(ap);

		game::Com_Printf(0, "%s", buffer);
	}
}

BOOL APIENTRY DllMain(HMODULE /*module*/, DWORD ul_reason_for_call, LPVOID /*reserved*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		utils::hook::jump(reinterpret_cast<size_t>(&printf), printf_stub);
		component_loader::post_unpack();
	}

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		component_loader::pre_destroy();
	}

	return TRUE;
}
