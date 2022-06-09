#pragma once
#include "entity.hpp"
#include "script_value.hpp"

namespace scripting
{
	class function
	{
	public:
		function(const char*);

		script_value get_raw() const;
		const char* get_pos() const;
		std::string get_name() const;

		script_value call(const entity& self, const arguments& arguments) const;

		script_value operator()(const entity& self, const arguments& arguments) const
		{
			return this->call(self, arguments);
		}

		script_value operator()(const arguments& arguments) const
		{
			return this->call(*game::levelEntityId, arguments);
		}

		script_value operator()() const
		{
			return this->call(*game::levelEntityId, {});
		}

		template<class ...T>
		arguments operator()(T... arguments) const
		{
			return this->call(*game::levelEntityId, {arguments...});
		}

		template<class ...T>
		arguments operator()(const entity& self, T... arguments) const
		{
			return this->call(self, {arguments...});
		}

	private:
		const char* pos_;
	};
}
