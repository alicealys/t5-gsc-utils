#include <stdinc.hpp>
#include "array.hpp"
#include "execution.hpp"

namespace scripting
{
	array_value::array_value(unsigned int parent_id, unsigned int id)
		: id_(id)
		, parent_id_(parent_id)
	{
		if (!this->id_)
		{
			return;
		}

		const auto value = game::scr_VarGlob->variableList[this->id_];
		game::VariableValue variable{};
		variable.u = value.u.u;
		variable.type = value.w.type & 0x1F;

		this->value_ = variable;
	}

	void array_value::operator=(const script_value& value)
	{
		if (!this->id_)
		{
			return;
		}

		const auto& value_0 = value.get_raw();

		const auto variable = &game::scr_VarGlob->variableList[this->id_];
		game::VariableValue variable_{};
		variable_.type = variable->w.type & 0x1F;
		variable_.u = variable->u.u;

		game::AddRefToValue(game::SCRIPTINSTANCE_SERVER, &value_0);
		game::RemoveRefToValue(game::SCRIPTINSTANCE_SERVER, variable_.type, variable_.u);

		variable->w.type = value_0.type & 0x1F;
		variable->u.u = value_0.u;

		this->value_ = value_0;
	}

	array::array(const unsigned int id)
		: id_(id)
	{
		this->add();
	}

	array::array(const array& other)
	{
		this->operator=(other);
	}

	array::array(array&& other) noexcept
	{
		this->id_ = other.id_;
		other.id_ = 0;
	}

	array::array()
	{
		this->id_ = game::Scr_AllocArray(game::SCRIPTINSTANCE_SERVER);
	}

	array::~array()
	{
		this->release();
	}

	array& array::operator=(const array& other)
	{
		if (&other != this)
		{
			this->release();
			this->id_ = other.id_;
			this->add();
		}

		return *this;
	}

	array& array::operator=(array&& other) noexcept
	{
		if (&other != this)
		{
			this->release();
			this->id_ = other.id_;
			other.id_ = 0;
		}

		return *this;
	}

	void array::add() const
	{
		if (this->id_)
		{
			game::VariableValue value{};
			value.u.uintValue = this->id_;
			value.type = game::SCRIPT_OBJECT;

			game::AddRefToValue(game::SCRIPTINSTANCE_SERVER, &value);
		}
	}

	void array::release() const
	{
		if (this->id_)
		{
			game::VariableValue value{};
			value.u.uintValue = this->id_;
			value.type = game::SCRIPT_OBJECT;

			game::RemoveRefToValue(game::SCRIPTINSTANCE_SERVER, value.type, value.u);
		}
	}

	std::vector<script_value> array::get_keys() const
	{
		std::vector<script_value> result;

		auto current = game::scr_VarGlob->variableList[this->id_ + 1].nextSibling;

		while (current)
		{
			const auto var = &game::scr_VarGlob->variableList[current + 0x8000];
			const auto key_value = game::Scr_GetArrayIndexValue(game::SCRIPTINSTANCE_SERVER, var->w.status >> 8);
			result.push_back(key_value);

			const auto next_sibling = game::scr_VarGlob->variableList[current + 0x8000].nextSibling;
			if (!next_sibling)
			{
				break;
			}

			current = game::scr_VarGlob->variableList[next_sibling + 0x8000].hash.id;
		}

		return result;
	}

	int array::size() const
	{
		return static_cast<int>(game::Scr_GetSelf(game::SCRIPTINSTANCE_SERVER, this->id_));
	}

	unsigned int array::push(const script_value& value) const
	{
		this->set(this->size(), value);
		return this->size();
	}

	void array::erase(const unsigned int index) const
	{
		const auto variable_id = game::FindArrayVariable(game::SCRIPTINSTANCE_SERVER, this->id_, index);
		if (variable_id)
		{
			game::RemoveVariableValue(game::SCRIPTINSTANCE_SERVER, this->id_, variable_id);
		}
	}

	void array::erase(const std::string& key) const
	{
		const auto string_value = game::SL_GetString(key.data(), 0, game::SCRIPTINSTANCE_SERVER);
		const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);
		if (variable_id)
		{
			game::RemoveVariableValue(game::SCRIPTINSTANCE_SERVER, this->id_, variable_id);
		}
	}

	script_value array::pop() const
	{
		const auto value = this->get(this->size() - 1);
		this->erase(this->size() - 1);
		return value;
	}

	script_value array::get(const std::string& key) const
	{
		const auto string_value = game::SL_GetString(key.data(), 0, game::SCRIPTINSTANCE_SERVER);
		const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);

		if (!variable_id)
		{
			return {};
		}

		const auto value = game::scr_VarGlob->variableList[variable_id + 0x8000];
		game::VariableValue variable{};
		variable.u = value.u.u;
		variable.type = value.w.type & 0x1F;

		return variable;
	}

	script_value array::get(const unsigned int index) const
	{
		const auto variable_id = game::FindArrayVariable(game::SCRIPTINSTANCE_SERVER, this->id_, index);

		if (!variable_id)
		{
			return {};
		}

		const auto value = game::scr_VarGlob->variableList[variable_id + 0x8000];
		game::VariableValue variable{};
		variable.u = value.u.u;
		variable.type = value.w.type & 0x1F;

		return variable;
	}

	script_value array::get(const script_value& key) const
	{
		if (key.is<int>())
		{
			this->get(key.as<int>());
		}

		if (key.is<std::string>())
		{
			this->get(key.as<std::string>());
		}

		return {};
	}

	void array::set(const std::string& key, const script_value& value) const
	{
		const auto& value_ = value.get_raw();
		const auto variable_id = this->get_value_id(key);

		if (!variable_id)
		{
			return;
		}

		const auto variable = &game::scr_VarGlob->variableList[variable_id + 0x8000];
		game::VariableValue variable_{};
		variable_.type = variable->w.type & 0x1F;
		variable_.u = variable->u.u;

		game::AddRefToValue(game::SCRIPTINSTANCE_SERVER, &value_);
		game::RemoveRefToValue(game::SCRIPTINSTANCE_SERVER, variable_.type, variable_.u);

		variable->w.type |= value_.type;
		variable->u.u = value_.u;
	}

	void array::set(const unsigned int index, const script_value& value) const
	{
		const auto& value_ = value.get_raw();
		const auto variable_id = this->get_value_id(index);

		if (!variable_id)
		{
			return;
		}

		auto variable_list = *game::scr_VarGlob;
		const auto variable = &game::scr_VarGlob->variableList[variable_id + 0x8000];
		game::VariableValue variable_{};
		variable_.type = variable->w.type & 0x1F;
		variable_.u = variable->u.u;

		game::AddRefToValue(game::SCRIPTINSTANCE_SERVER, &value_);
		game::RemoveRefToValue(game::SCRIPTINSTANCE_SERVER, variable_.type, variable_.u);

		variable->w.type |= value_.type;
		variable->u.u = value_.u;
	}

	void array::set(const script_value& key, const script_value& _value) const
	{
		if (key.is<int>())
		{
			this->set(key.as<int>(), _value);
		}

		if (key.is<std::string>())
		{
			this->set(key.as<std::string>(), _value);
		}
	}

	unsigned int array::get_entity_id() const
	{
		return this->id_;
	}

	unsigned int array::get_value_id(const std::string& key) const
	{
		const auto string_value = game::SL_GetString(key.data(), 0, game::SCRIPTINSTANCE_SERVER);
		const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);

		if (!variable_id)
		{
			return game::GetNewVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);
		}

		return variable_id;
	}

	unsigned int array::get_value_id(const unsigned int index) const
	{
		const auto variable_id = game::FindArrayVariable(game::SCRIPTINSTANCE_SERVER, this->id_, index);
		if (!variable_id)
		{
			return game::GetNewArrayVariable(game::SCRIPTINSTANCE_SERVER, this->id_, index);
		}

		return variable_id;
	}

	entity array::get_raw() const
	{
		return entity(this->id_);
	}
}
