#include <stdinc.hpp>
#include "object.hpp"
#include "execution.hpp"

#include "../../component/scripting.hpp"

namespace scripting
{
	namespace
	{
		std::string get_string(unsigned int canonical_string)
		{
			if (canonical_string_table.find(canonical_string) != canonical_string_table.end())
			{
				return canonical_string_table[canonical_string];
			}

			return "";
		}

		unsigned int get_canonical_string(const std::string& str)
		{
			for (const auto& value : canonical_string_table)
			{
				if (value.second == str)
				{
					return value.first;
				}
			}

			return game::SL_GetCanonicalString(game::SCRIPTINSTANCE_SERVER, str.data());
		}

		std::vector<std::string> get_keys_sp(unsigned int id)
		{
			std::vector<std::string> result;

			auto current = game::scr_VarGlob->variableList_sp[id + 1].nextSibling;

			while (current)
			{
				const auto var = &game::scr_VarGlob->variableList_sp[current + 0x6000];
				const auto key_value = get_string(var->w.status >> 8);
				result.push_back(key_value);

				const auto next_sibling = game::scr_VarGlob->variableList_sp[current + 0x6000].nextSibling;
				if (!next_sibling)
				{
					break;
				}

				current = static_cast<unsigned __int16>(game::scr_VarGlob->variableList_sp[next_sibling + 0x6000].id);
			}

			return result;
		}

		std::vector<std::string> get_keys_mp(unsigned int id)
		{
			std::vector<std::string> result;

			auto current = game::scr_VarGlob->variableList_mp[id + 1].nextSibling;

			while (current)
			{
				const auto var = &game::scr_VarGlob->variableList_mp[current + 0x8000];
				const auto key_value = get_string(var->w.status >> 8);
				result.push_back(key_value);

				const auto next_sibling = game::scr_VarGlob->variableList_mp[current + 0x8000].nextSibling;
				if (!next_sibling)
				{
					break;
				}

				current = game::scr_VarGlob->variableList_mp[next_sibling + 0x8000].hash.id;
			}

			return result;
		}
	}

	object_value::object_value(unsigned int parent_id, unsigned int id)
		: id_(id)
		, parent_id_(parent_id)
	{
		if (!this->id_)
		{
			return;
		}

		game::VariableValue variable_{};

		if (game::environment::is_sp())
		{
			const auto variable = &game::scr_VarGlob->variableList_sp[this->id_ + 0x6000];
			variable_.type = variable->w.type & 0x1F;
			variable_.u = variable->u.u;
		}
		else
		{
			const auto variable = &game::scr_VarGlob->variableList_mp[this->id_ + 0x8000];
			variable_.type = variable->w.type & 0x1F;
			variable_.u = variable->u.u;
		}

		this->value_ = variable_;
	}

	void object_value::operator=(const script_value& value)
	{
		if (!this->id_)
		{
			return;
		}

		const auto& value_0 = value.get_raw();

		game::VariableValue previous{};

		if (game::environment::is_sp())
		{
			const auto variable = &game::scr_VarGlob->variableList_sp[this->id_ + 0x6000];
			previous.type = variable->w.type & 0x1F;
			previous.u = variable->u.u;

			variable->w.type |= value_0.type;
			variable->u.u = value_0.u;
		}
		else
		{
			const auto variable = &game::scr_VarGlob->variableList_mp[this->id_ + 0x8000];
			previous.type = variable->w.type & 0x1F;
			previous.u = variable->u.u;

			variable->w.type |= value_0.type;
			variable->u.u = value_0.u;
		}

		game::AddRefToValue(game::SCRIPTINSTANCE_SERVER, &value_0);
		game::RemoveRefToValue(game::SCRIPTINSTANCE_SERVER, previous.type, previous.u);

		this->value_ = value_0;
	}

	object::object(const unsigned int id)
		: id_(id)
	{
		this->add();
	}

	object::object(const object& other)
	{
		this->operator=(other);
	}

	object::object(object&& other) noexcept
	{
		this->id_ = other.id_;
		other.id_ = 0;
	}

	object::object()
	{
		this->id_ = game::AllocObject(game::SCRIPTINSTANCE_SERVER);
	}

	object::~object()
	{
		this->release();
	}

	object& object::operator=(const object& other)
	{
		if (&other != this)
		{
			this->release();
			this->id_ = other.id_;
			this->add();
		}

		return *this;
	}

	object& object::operator=(object&& other) noexcept
	{
		if (&other != this)
		{
			this->release();
			this->id_ = other.id_;
			other.id_ = 0;
		}

		return *this;
	}

	void object::add() const
	{
		if (this->id_)
		{
			game::VariableValue value{};
			value.u.uintValue = this->id_;
			value.type = game::SCRIPT_OBJECT;

			game::AddRefToValue(game::SCRIPTINSTANCE_SERVER, &value);
		}
	}

	void object::release() const
	{
		if (this->id_)
		{
			game::VariableValue value{};
			value.u.uintValue = this->id_;
			value.type = game::SCRIPT_OBJECT;

			game::RemoveRefToValue(game::SCRIPTINSTANCE_SERVER, value.type, value.u);
		}
	}

	std::vector<std::string> object::get_keys() const
	{
		return SELECT_VALUE(get_keys_sp, get_keys_mp)(this->id_);
	}

	unsigned int object::size() const
	{
		return game::Scr_GetSelf(game::SCRIPTINSTANCE_SERVER, this->id_);
	}

	void object::erase(const std::string& /*key*/) const
	{
		/*const auto string_value = game::SL_GetCanonicalString(key.data(), 0);
		const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);
		if (variable_id)
		{
			game::RemoveVariableValue(game::SCRIPTINSTANCE_SERVER, this->id_, variable_id);
		}*/
	}

	script_value object::get(const std::string& key) const
	{
		const auto string_value = get_canonical_string(key);
		const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);

		if (!variable_id)
		{
			return {};
		}

		game::VariableValue variable_{};

		if (game::environment::is_sp())
		{
			const auto variable = &game::scr_VarGlob->variableList_sp[variable_id + 0x6000];
			variable_.type = variable->w.type & 0x1F;
			variable_.u = variable->u.u;
		}
		else
		{
			const auto variable = &game::scr_VarGlob->variableList_mp[variable_id + 0x8000];
			variable_.type = variable->w.type & 0x1F;
			variable_.u = variable->u.u;
		}

		return variable_;
	}

	void object::set(const std::string& key, const script_value& value) const
	{
		const auto& value_ = value.get_raw();
		const auto variable_id = this->get_value_id(key);

		if (!variable_id)
		{
			return;
		}

		game::VariableValue previous{};

		if (game::environment::is_sp())
		{
			const auto variable = &game::scr_VarGlob->variableList_sp[variable_id + 0x6000];
			previous.type = variable->w.type & 0x1F;
			previous.u = variable->u.u;

			variable->w.type |= value_.type;
			variable->u.u = value_.u;
		}
		else
		{
			const auto variable = &game::scr_VarGlob->variableList_mp[variable_id + 0x8000];
			previous.type = variable->w.type & 0x1F;
			previous.u = variable->u.u;

			variable->w.type |= value_.type;
			variable->u.u = value_.u;
		}

		game::AddRefToValue(game::SCRIPTINSTANCE_SERVER, &value_);
		game::RemoveRefToValue(game::SCRIPTINSTANCE_SERVER, previous.type, previous.u);
	}

	unsigned int object::get_entity_id() const
	{
		return this->id_;
	}

	unsigned int object::get_value_id(const std::string& key) const
	{
		const auto string_value = get_canonical_string(key);
		const auto variable_id = game::FindVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);

		if (!variable_id)
		{
			return game::GetNewVariable(game::SCRIPTINSTANCE_SERVER, this->id_, string_value);
		}

		return variable_id;
	}

	entity object::get_raw() const
	{
		return entity(this->id_);
	}
}
