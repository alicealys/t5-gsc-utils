#pragma once
#include "game/scripting/array.hpp"
#include "game/scripting/execution.hpp"

namespace gsc
{
	using function_t = std::function<scripting::script_value(const scripting::function_arguments& args)>;
	using script_function = void(*)(game::scr_entref_t);

	template <class... Args, std::size_t... I>
	auto wrap_function(const std::function<void(Args...)>& f, std::index_sequence<I...>)
	{
		return [f]([[maybe_unused]] const scripting::function_arguments& args)
		{
			f(args[I]...);
			return scripting::script_value{};
		};
	}

	template <class... Args, std::size_t... I>
	auto wrap_function(const std::function<scripting::script_value(Args...)>& f, std::index_sequence<I...>)
	{
		return [f]([[maybe_unused]] const scripting::function_arguments& args)
		{
			return f(args[I]...);
		};
	}

	template <typename R, class... Args, std::size_t... I>
	auto wrap_function(const std::function<R(Args...)>& f, std::index_sequence<I...>)
	{
		return [f]([[maybe_unused]] const scripting::function_arguments& args)
		{
			return scripting::script_value{f(args[I]...)};
		};
	}

	template <typename R, class... Args>
	auto wrap_function(const std::function<R(Args...)>& f)
	{
		return wrap_function(f, std::index_sequence_for<Args...>{});
	}

	template <class F>
	auto wrap_function(F f)
	{
		return wrap_function(std::function(f));
	}

	namespace function
	{
		void add_internal(const std::string& name, const function_t& function);

		template <typename F>
		void add(const std::string& name, F f)
		{
			add_internal(name, wrap_function(f));
		}
	}

	namespace method
	{
		void add_internal(const std::string& name, const function_t& function);

		template <typename F>
		void add(const std::string& name, F f)
		{
			add_internal(name, wrap_function(f));
		}
	}
}
