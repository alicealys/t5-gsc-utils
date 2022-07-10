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

	void SV_GetServerinfo(char* buffer, int bufferSize)
	{
		if (bufferSize < 1)
		{
			Com_Error(ERR_DROP, "\x15SV_GetServerinfo: bufferSize == %i", bufferSize);
		}

		I_strncpyz(buffer, Dvar_InfoString(0, 4), bufferSize);
	}

	void AddRefToValue(scriptInstance_t inst, const VariableValue* value)
	{
		AddRefToValue_(inst, value->type, value->u);
	}

	void RemoveRefToValue(scriptInstance_t inst, const int type, VariableUnion value)
	{
		if (game::environment::is_mp())
		{
			mp::RemoveRefToValue(inst, type, value);
		}
		else
		{
			game::VariableValue var{};
			var.type = type;
			var.u = value;

			sp::RemoveRefToValue(inst, &var);
		}
	}

	unsigned int GetObjectType(scriptInstance_t, unsigned int id)
	{
		if (game::environment::is_sp())
		{
			return game::scr_VarGlob->variableList_sp[id + 1].w.type & 0x1F;
		}
		else
		{
			return game::scr_VarGlob->variableList_mp[id + 1].w.type & 0x1F;
		}
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

	void Scr_NotifyId(scriptInstance_t inst, int /*client_num*/, unsigned int id,
		unsigned int string_value, unsigned int paramcount)
	{
		game::Scr_NotifyNum_Internal(inst, -1, id, 0, string_value, paramcount);
	}

	VariableValue Scr_GetArrayIndexValue(scriptInstance_t, unsigned int name)
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

	unsigned int Scr_GetSelf(scriptInstance_t, unsigned int threadId)
	{
		if (game::environment::is_sp())
		{
			return game::scr_VarGlob->variableList_sp[threadId + 1].u.o.size;
		}
		else
		{
			return game::scr_VarGlob->variableList_mp[threadId + 1].u.o.u.self;
		}
	}
}
