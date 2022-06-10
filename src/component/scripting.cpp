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
			//g_shutdown_game_hook.create(SELECT(0x60DCF0, 0x688A40), g_shutdown_game_stub);
		}
	};
}

//REGISTER_COMPONENT(scripting::component)
