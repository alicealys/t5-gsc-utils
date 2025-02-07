#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "gsc.hpp"

#include <utils/hook.hpp>
#include <utils/info_string.hpp>

namespace user_info
{
	namespace
	{
		using user_info_map = std::unordered_map<std::string, std::string>;
		std::unordered_map<int, user_info_map> user_info_overrides;

		utils::hook::detour scr_shutdown_system_hook;

		void clear_client_overrides(const int client_num)
		{
			user_info_overrides[client_num].clear();
		}

		void clear_all_overrides()
		{
			user_info_overrides.clear();
		}

		void client_disconnect_stub(const int client_num)
		{
			clear_client_overrides(client_num);
			game::ClientDisconnect(client_num);
		}

		void scr_shutdown_system_stub(const game::scriptInstance_t inst, const unsigned char sys, const int b_complete)
		{
			clear_all_overrides();
			scr_shutdown_system_hook.invoke<void>(inst, sys, b_complete);
		}

		void sv_get_user_info_stub(const int index, char* buffer, const int buffer_size)
		{
			game::SV_GetUserinfo(index, buffer, buffer_size);

			utils::info_string map(buffer);

			if (!user_info_overrides.contains(index))
			{
				user_info_overrides[index] = {};
			}

			for (const auto& [key, val] : user_info_overrides[index])
			{
				if (val.empty())
				{
					map.remove(key);
				}
				else
				{
					map.set(key, val);
				}
			}

			const auto user_info = map.build();
			strncpy_s(buffer, buffer_size, user_info.data(), _TRUNCATE);
		}
	}

	class component final : public component_interface
	{
	public:
		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			utils::hook::call(SELECT_VALUE(0x5D38EB, 0x4A75E2), sv_get_user_info_stub);
			utils::hook::call(SELECT_VALUE(0x67FFE9, 0x548DB0), sv_get_user_info_stub);

			utils::hook::call(SELECT_VALUE(0x4F3931, 0x5DC953), client_disconnect_stub);

			scr_shutdown_system_hook.create(SELECT_VALUE(0x596D40, 0x540780), scr_shutdown_system_stub);

			gsc::method::add_multiple([](const scripting::entity& player, const std::string& name) -> void
			{
				const auto entref = player.get_entity_reference();
				if (entref.classnum)
				{
					throw std::runtime_error("Not a player entity");
				}

				if (name.empty())
				{
					throw std::runtime_error("set_name: Illegal parameter!");
				}

				user_info_overrides[entref.entnum]["name"] = name;
				game::ClientUserinfoChanged(entref.entnum);
			}, "user_info::set_name", "set_name");

			gsc::method::add_multiple([](const scripting::entity& player) -> void
			{
				const auto entref = player.get_entity_reference();
				if (entref.classnum)
				{
					throw std::runtime_error("Not a player entity");
				}

				user_info_overrides[entref.entnum].erase("name");
				game::ClientUserinfoChanged(entref.entnum);
			}, "user_info::reset_name", "reset_name");

			gsc::method::add_multiple([](const scripting::entity& player, const std::string& tag) -> void
			{
				const auto entref = player.get_entity_reference();
				if (entref.classnum)
				{
					throw std::runtime_error("Not a player entity");
				}

				if (tag.empty())
				{
					throw std::runtime_error("set_clantag: Illegal parameter!");
				}

				user_info_overrides[entref.entnum]["clanAbbrev"] = tag;
				game::ClientUserinfoChanged(entref.entnum);
			}, "user_info::set_clantag", "set_clantag");

			gsc::method::add_multiple([](const scripting::entity& player) -> void
			{
				const auto entref = player.get_entity_reference();
				if (entref.classnum)
				{
					throw std::runtime_error("Not a player entity");
				}

				user_info_overrides[entref.entnum].erase("clanAbbrev");
				game::ClientUserinfoChanged(entref.entnum);
			}, "user_info::reset_clantag", "reset_clantag");
		}
	};
}

REGISTER_COMPONENT(user_info::component)
