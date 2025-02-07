#include "stdinc.hpp"
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "command.hpp"
#include "gsc.hpp"
#include "scripting.hpp"
#include "scheduler.hpp"

#include <utils/string.hpp>
#include <utils/memory.hpp>
#include <utils/hook.hpp>

namespace command
{
	namespace
	{
		std::unordered_map<std::string, std::function<void(params&)>> handlers;
		std::unordered_map<std::string, std::function<void(int, params_sv&)>> handlers_sv;

		std::vector<std::string> script_commands;
		std::vector<std::string> script_sv_commands;
		utils::memory::allocator allocator;

		utils::hook::detour client_command_hook;

		std::vector<scripting::function> chat_callbacks;

		game::CmdArgs* get_cmd_args()
		{
			return static_cast<game::CmdArgs*>(game::Sys_GetValue(4));
		}

		void main_handler()
		{
			params params = {};

			const auto command = utils::string::to_lower(params[0]);

			if (handlers.find(command) != handlers.end())
			{
				handlers[command](params);
			}
		}

		bool can_add_callback = true;
		bool handle_chat_command(const int client_num, const params_sv& params)
		{
			const auto _0 = gsl::finally([]
			{
				can_add_callback = true;
			});
			can_add_callback = false;

			auto hide = false;
			const scripting::entity player = game::Scr_GetEntityId(game::SCRIPTINSTANCE_SERVER, client_num, 0, 0);

			const auto command = utils::string::to_lower(params[0]);
			const auto message = params.join(1).substr(1);

			for (const auto& callback : chat_callbacks)
			{
				const auto res = callback(player, {message, command == "say_team"});
				if (res.is<bool>() && !res.as<bool>())
				{
					hide = true;
				}
			}

			return hide;
		}

		void client_command_stub(const int client_num)
		{
			params_sv params = {};
			if (params.size() < 1)
			{
				client_command_hook.invoke<void>(client_num);
				return;
			}

			const auto command = utils::string::to_lower(params[0]);
			if (params.size() > 1 && (command == "say" || command == "say_team") &&
				handle_chat_command(client_num, params))
			{
				return;
			}

			if (handlers_sv.find(command) != handlers_sv.end())
			{
				handlers_sv[command](client_num, params);
			}

			client_command_hook.invoke<void>(client_num);
		}

		void add_script_command(const std::string& name, const std::function<void(const params&)>& callback)
		{
			script_commands.push_back(name);
			const auto name_ = allocator.duplicate_string(name);
			add(name_, callback);
		}

		void add_script_sv_command(const std::string& name, const std::function<void(int, const params_sv&)>& callback)
		{
			script_sv_commands.push_back(name);
			add_sv(name, callback);
		}

		void clear()
		{
			for (const auto& name : script_commands)
			{
				handlers.erase(name);
				game::Cmd_RemoveCommand(name.data());
			}

			for (const auto& name : script_sv_commands)
			{
				handlers_sv.erase(name);
			}

			allocator.clear();
			script_commands.clear();
			script_sv_commands.clear();
			chat_callbacks.clear();
		}
	}

	params::params()
		: nesting_(get_cmd_args()->nesting)
	{
	}

	int params::size() const
	{
		const auto cmd_args = get_cmd_args();
		return cmd_args->argc[cmd_args->nesting];
	}

	const char* params::get(int index) const
	{
		if (index >= this->size())
		{
			return "";
		}

		const auto cmd_args = get_cmd_args();
		return cmd_args->argv[this->nesting_][index];
	}

	std::string params::join(int index) const
	{
		std::string result = {};

		for (auto i = index; i < this->size(); i++)
		{
			if (i > index)
			{
				result.append(" ");
			}

			result.append(this->get(i));
		}

		return result;
	}

	std::vector<std::string> params::get_all() const
	{
		std::vector<std::string> params_;
		for (auto i = 0; i < this->size(); i++)
		{
			params_.push_back(this->get(i));
		}
		return params_;
	}

	params_sv::params_sv()
		: nesting_(game::sv_cmd_args->nesting)
	{
	}

	int params_sv::size() const
	{
		return game::sv_cmd_args->argc[this->nesting_];
	}

	const char* params_sv::get(const int index) const
	{
		if (index >= this->size())
		{
			return "";
		}

		return game::sv_cmd_args->argv[this->nesting_][index];
	}

	std::string params_sv::join(const int index) const
	{
		std::string result = {};

		for (auto i = index; i < this->size(); i++)
		{
			if (i > index)
			{
				result.append(" ");
			}

			result.append(this->get(i));
		}

		return result;
	}

	std::vector<std::string> params_sv::get_all() const
	{
		std::vector<std::string> params_;
		for (auto i = 0; i < this->size(); i++)
		{
			params_.push_back(this->get(i));
		}
		return params_;
	}

	void add_raw(const char* name, void (*callback)())
	{
		game::Cmd_AddCommandInternal(name, callback, utils::memory::get_allocator()->allocate<game::cmd_function_t>());
	}

	void add(const char* name, std::function<void(params&)> callback)
	{
		const auto command = utils::string::to_lower(name);

		if (handlers.find(command) == handlers.end())
		{
			add_raw(name, main_handler);
		}

		handlers[command] = callback;
	}

	void add_sv(const std::string& name, std::function<void(int, const params_sv&)> callback)
	{
		const auto command = utils::string::to_lower(name);
		if (handlers_sv.find(command) == handlers_sv.end())
		{
			handlers_sv[command] = std::move(callback);
		}
	}

	void execute(std::string command, const bool sync)
	{
		command += "\n";

		if (sync)
		{
			game::Cmd_ExecuteSingleCommand(0, 0, command.data());
		}
		else
		{
			game::Cbuf_AddText(0, command.data());
		}
	}

	class component final : public component_interface
	{
	public:
		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			scripting::on_shutdown(clear);
			client_command_hook.create(SELECT_VALUE(0x4AF770, 0x63DB70), client_command_stub);

			gsc::function::add_multiple([](const std::string& command)
			{
				execute(command, false);
			}, "executecommand", "command::execute");

			gsc::function::add_multiple([](const std::string& name, const scripting::function& function)
			{
				command::add_script_command(name, [function](const command::params& params)
				{
					const auto params_ = params.get_all();
					scheduler::once([=]()
					{
						scripting::array array;

						for (auto i = 0; i < params.size(); i++)
						{
							array.push(params[i]);
						}

						function({array});
					});
				});
			}, "addcommand", "command::add");

			gsc::function::add_multiple([](const std::string& name, const scripting::function& function)
			{
				command::add_script_sv_command(name, [function](const int client_num, const command::params_sv& params)
				{
					const auto params_ = params.get_all();
					scheduler::once([=]()
					{
						const scripting::entity player = game::Scr_GetEntityId(game::SCRIPTINSTANCE_SERVER, client_num, 0, 0);

						scripting::array array;

						for (auto i = 0; i < params.size(); i++)
						{
							array.push(params[i]);
						}

						function(player, {array});
					}, scheduler::pipeline::server);
				});
			}, "addclientcommand", "command::add_sv");

			gsc::method::add("tell", [](const scripting::entity& player, const std::string& msg)
			{
				const auto entref = player.get_entity_reference();
				if (entref.classnum)
				{
					throw std::runtime_error("Not a player entity");
				}

				game::SV_GameSendServerCommand(entref.entnum, 0, utils::string::va("h \"%s\"", msg.data()));
			});

			gsc::function::add("say", [](const std::string& msg)
			{
				game::SV_GameSendServerCommand(-1, 0, utils::string::va("h \"%s\"", msg.data()));
			});

			gsc::function::add("sendservercommand", game::SV_GameSendServerCommand.get());

			gsc::function::add_multiple([](const scripting::function& callback)
			{
				if (!can_add_callback)
				{
					throw std::runtime_error("Cannot add a callback in this context");
				}

				chat_callbacks.push_back(callback);
			}, "onplayersay", "command::on_player_say");
		}
	};
}

REGISTER_COMPONENT(command::component)
