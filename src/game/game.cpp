#include <stdinc.hpp>
#include "game.hpp"

#include <utils/hook.hpp>

namespace game
{
	gamemode current = *reinterpret_cast<int*>(0x401337) == 0x281488C0
		? gamemode::multiplayer
		: gamemode::singleplayer;

	namespace environment
	{
		bool is_mp()
		{
			return current == gamemode::multiplayer;
		}

		bool is_sp()
		{
			return current == gamemode::singleplayer;
		}
	}

	void AddRefToValue(scriptInstance_t inst, const VariableValue* value)
	{
		AddRefToValue_(inst, value->type, value->u);
	}

	unsigned int AllocVariable(scriptInstance_t inst)
	{
		static const auto func = utils::hook::assemble([](utils::hook::assembler& a)
		{
			a.mov(eax, 0);
			a.call(SELECT_VALUE(0, 0x8E49A0));
			a.ret();
		});

		return utils::hook::invoke<unsigned int>(func, inst);
	}

	VariableValue Scr_GetArrayIndexValue(scriptInstance_t inst, unsigned int name)
	{
		VariableValue value{};

		if (name >= 0x10000)
		{
			if (name >= 0x17FFE)
			{
				value.type = 6;
				value.u.intValue = name - 0x800000;
			}
			else
			{
				value.type = 1;
				value.u.intValue = name - 0x10000;
			}
		}
		else
		{
			value.type = 2;
			value.u.intValue = name;
		}

		return value;
	}

	unsigned int Scr_GetSelf(scriptInstance_t inst, unsigned int threadId)
	{
		return game::scr_VarGlob->variableList[threadId + 1].u.o.u.self;
	}

	namespace plutonium
	{
		bool is_up_to_date()
		{
			//const auto value = *reinterpret_cast<DWORD*>(0);
			//return value == 0;
			return false;
		}
	}
}
