#pragma once

#include "structs.hpp"

#define SELECT_VALUE(sp, mp) (game::environment::is_sp() ? (sp) : (mp))

namespace game
{
	enum gamemode
	{
		none,
		multiplayer,
		singleplayer
	};

	extern gamemode current;

	namespace environment
	{
		bool is_mp();
		bool is_sp();
	}

	template <typename T>
	class symbol
	{
	public:
		symbol(const size_t sp_address, const size_t mp_address)
			: sp_object_(reinterpret_cast<T*>(sp_address))
			, mp_object_(reinterpret_cast<T*>(mp_address))
		{
		}

		T* get() const
		{
			if (environment::is_mp())
			{
#ifdef DEBUG
				if (mp_object_ == nullptr)
				{
					MessageBoxA(nullptr, "nullptr symbol", "", 0);
				}
#endif

				return mp_object_;
			}

#ifdef DEBUG
			if (sp_object_ == nullptr)
			{
				MessageBoxA(nullptr, "nullptr symbol", "", 0);
			}
#endif

			return sp_object_;
		}

		operator T* () const
		{
			return this->get();
		}

		T* operator->() const
		{
			return this->get();
		}

	private:
		T* sp_object_;
		T* mp_object_;
	};

	void AddRefToValue(scriptInstance_t inst, const VariableValue* value);
	unsigned int AllocVariable(scriptInstance_t inst);

	VariableValue Scr_GetArrayIndexValue(scriptInstance_t inst, unsigned int name);
	unsigned int Scr_GetSelf(scriptInstance_t inst, unsigned int threadId);

	namespace plutonium
	{
		bool is_up_to_date();
	}
}

#include "symbols.hpp"
