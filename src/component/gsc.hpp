#pragma once
#include "game/scripting/array.hpp"
#include "game/scripting/execution.hpp"

namespace scripting
{
	using script_function = void(*)(game::scr_entref_t);

	template <class... Args, std::size_t... I>
	auto wrap_function(const std::function<void(Args...)>& f, std::index_sequence<I...>)
	{
		return [f]([[maybe_unused]] const function_arguments& args)
		{
			f(args[I]...);
			return script_value{};
		};
	}

	template <class... Args, std::size_t... I>
	auto wrap_function(const std::function<script_value(Args...)>& f, std::index_sequence<I...>)
	{
		return [f]([[maybe_unused]] const function_arguments& args)
		{
			return f(args[I]...);
		};
	}

	template <typename R, class... Args, std::size_t... I>
	auto wrap_function(const std::function<R(Args...)>& f, std::index_sequence<I...>)
	{
		return [f]([[maybe_unused]] const function_arguments& args)
		{
			return script_value{f(args[I]...)};
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

	template <typename F>
	void add_function(const std::string& name, F f);

	template <typename F>
	void add_method(const std::string& name, F f);
}
