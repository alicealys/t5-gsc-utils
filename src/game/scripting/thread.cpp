#include <stdinc.hpp>
#include "thread.hpp"
#include "execution.hpp"
#include "../../component/scripting.hpp"

namespace scripting
{
	thread::thread(unsigned int id)
		: id_(id)
		 , type_(game::scr_VarGlob->variableList[id].w.type & 0x7F)
	{
	}

	script_value thread::get_raw() const
	{
		game::VariableValue value;
		value.type = game::SCRIPT_OBJECT;
		value.u.uintValue = this->id_;

		return value;
	}

	unsigned int thread::get_entity_id() const
	{
		return this->id_;
	}

	unsigned int thread::get_type() const
	{
		return this->type_;
	}

	unsigned int thread::get_wait_time() const
	{
		return game::scr_VarGlob->variableList[this->id_].w.waitTime >> 8;
	}

	unsigned int thread::get_notify_name_id() const
	{
		return game::scr_VarGlob->variableList[this->id_].w.notifyName >> 8;
	}

	unsigned int thread::get_self() const
	{
		return game::Scr_GetSelf(game::SCRIPTINSTANCE_SERVER, this->id_);
	}

	std::string thread::get_notify_name() const
	{
		return game::SL_ConvertToString(this->get_notify_name_id(), game::SCRIPTINSTANCE_SERVER);
	}

	const char* thread::get_pos() const
	{
		return 0;
	}

	const char* thread::get_start_pos() const
	{
		return 0;
	}

	void thread::kill() const
	{

	}
}
