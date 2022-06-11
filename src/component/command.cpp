#include "stdinc.hpp"
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include "command.hpp"
#include "gsc.hpp"
#include "scripting.hpp"

#include <utils/string.hpp>
#include <utils/memory.hpp>

namespace command
{
	std::unordered_map<std::string, std::function<void(params&)>> handlers;

	std::vector<std::string> script_commands;
	utils::memory::allocator allocator;

	game::CmdArgs* get_cmd_args()
	{
		return reinterpret_cast<game::CmdArgs*>(game::Sys_GetValue(4));
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

	void add_script_command(const std::string& name, const std::function<void(const params&)>& callback)
	{
		script_commands.push_back(name);
		const auto name_ = allocator.duplicate_string(name);
		add(name_, callback);
	}

	void clear_script_commands()
	{
		for (const auto& name : script_commands)
		{
			handlers.erase(name);
			game::Cmd_RemoveCommand(name.data());
		}

		allocator.clear();
		script_commands.clear();
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
		void post_unpack() override
		{
			scripting::on_shutdown(clear_script_commands);

			const auto execute_command = [](const std::string& command)
			{
				execute(command, false);
			};

			const auto add_command = [](const std::string& name, const scripting::function& function)
			{
				command::add_script_command(name, [function](const command::params& params)
				{
					scripting::array array;

					for (auto i = 0; i < params.size(); i++)
					{
						array.push(params[i]);
					}

					function({array});
				});
			};

			gsc::function::add_multiple([](const std::string& command)
			{
				execute(command, false);
			}, "executecommand", "command::execute");


			gsc::function::add_multiple([](const std::string& name, const scripting::function& function)
			{
				command::add_script_command(name, [function](const command::params& params)
				{
					scripting::array array;

					for (auto i = 0; i < params.size(); i++)
					{
						array.push(params[i]);
					}

					function({array});
				});
			}, "addcommand", "command::add");
		}
	};
}

REGISTER_COMPONENT(command::component)
