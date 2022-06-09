#include <stdinc.hpp>
#include "entity.hpp"
#include "script_value.hpp"
#include "execution.hpp"

namespace scripting
{
	entity::entity()
		: entity(0)
	{
	}

	entity::entity(const entity& other) : entity(other.entity_id_)
	{
	}

	entity::entity(entity&& other) noexcept
	{
		this->entity_id_ = other.entity_id_;
		other.entity_id_ = 0;
	}

	entity::entity(const unsigned int entity_id)
		: entity_id_(entity_id)
	{
		this->add();
	}

	entity::~entity()
	{
		this->release();
	}

	entity& entity::operator=(const entity& other)
	{
		if (&other != this)
		{
			this->release();
			this->entity_id_ = other.entity_id_;
			this->add();
		}

		return *this;
	}

	entity& entity::operator=(entity&& other) noexcept
	{
		if (&other != this)
		{
			this->release();
			this->entity_id_ = other.entity_id_;
			other.entity_id_ = 0;
		}

		return *this;
	}

	unsigned int entity::get_entity_id() const
	{
		return this->entity_id_;
	}

	game::scr_entref_t entity::get_entity_reference() const
	{
		if (!this->entity_id_)
		{
			const auto not_null = static_cast<uint16_t>(~0ui16);
			return game::scr_entref_t{not_null, not_null};
		}

		game::scr_entref_t entref{};
		game::Scr_GetEntityIdRef(&entref, game::SCRIPTINSTANCE_SERVER, this->get_entity_id());
		return entref;
	}

	bool entity::operator==(const entity& other) const noexcept
	{
		return this->get_entity_id() == other.get_entity_id();
	}

	bool entity::operator!=(const entity& other) const noexcept
	{
		return !this->operator==(other);
	}

	void entity::add() const
	{
		if (this->entity_id_)
		{
			game::VariableValue value;
			value.type = game::SCRIPT_OBJECT;
			value.u.uintValue = this->entity_id_;

			game::AddRefToValue(game::SCRIPTINSTANCE_SERVER, &value);
		}
	}

	void entity::release() const
	{
		if (this->entity_id_)
		{
			game::VariableValue value;
			value.type = game::SCRIPT_OBJECT;
			value.u.uintValue = this->entity_id_;

			game::RemoveRefToValue(game::SCRIPTINSTANCE_SERVER, value.type, value.u);
		}
	}

	void entity::set(const std::string& field, const script_value& value) const
	{
		set_entity_field(*this, field, value);
	}

	template <>
	script_value entity::get<script_value>(const std::string& field) const
	{
		return get_entity_field(*this, field);
	}
}
