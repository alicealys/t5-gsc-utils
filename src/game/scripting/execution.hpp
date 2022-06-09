#pragma once
#include "game/game.hpp"
#include "entity.hpp"
#include "array.hpp"
#include "object.hpp"
#include "function.hpp"
#include "thread.hpp"
#include "script_value.hpp"

namespace scripting
{
	void push_value(const script_value& value);

	script_value exec_ent_thread(const entity& entity, const char* pos, const std::vector<script_value>& arguments);

	void set_entity_field(const entity& entity, const std::string& field, const script_value& value);
	script_value get_entity_field(const entity& entity, const std::string& field);

	void notify(const entity& entity, const std::string& event, const std::vector<script_value>& arguments);

	unsigned int make_object();
}
