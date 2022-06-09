#pragma once
#include "game/game.hpp"
#include "variable_value.hpp"
#include "vector.hpp"

#include <utils/string.hpp>

namespace scripting
{
	class entity;
	class array;
	class object;
	class function;
	class script_value;

	namespace
	{
		std::unordered_map<int, std::string> typenames = 
		{
			{0, "undefined"},
			{1, "object"},
			{2, "string"},
			{3, "localized string"},
			{4, "vector"},
			{5, "float"},
			{6, "int"},
			{7, "codepos"},
			{8, "precodepos"},
			{9, "function"},
			{10, "stack"},
			{11, "animation"},
			{12, "developer codepos"},
			{13, "thread"},
			{14, "thread"},
			{15, "thread"},
			{16, "thread"},
			{17, "struct"},
			{18, "removed entity"},
			{19, "entity"},
			{20, "array"},
			{21, "removed thread"},
		};

		std::string get_typename(const game::VariableValue& value)
		{
			auto type_ = 0;
			if (value.type == game::SCRIPT_OBJECT)
			{
				type_ = game::scr_VarGlob->variableList[value.u.uintValue].w.type & 0x1F;
			}
			else
			{
				type_ = value.type;
			}

			if (typenames.find(type_) != typenames.end())
			{
				return typenames[type_];
			}

			printf("UNKNOWN TYPE %i\n", type_);
			return "unknown";
		}

		template <typename T>
		std::string get_c_typename()
		{
			auto& info = typeid(T);

			if (info == typeid(std::string))
			{
				return "string";
			}

			if (info == typeid(const char*))
			{
				return "string";
			}

			if (info == typeid(entity))
			{
				return "entity";
			}

			if (info == typeid(array))
			{
				return "array";
			}

			if (info == typeid(object))
			{
				return "struct";
			}

			if (info == typeid(function))
			{
				return "function";
			}

			if (info == typeid(vector))
			{
				return "vector";
			}

			return info.name();
		}
	}

	using arguments = std::vector<script_value>;

	class script_value
	{
	public:
		script_value() = default;
		script_value(const game::VariableValue& value);

		script_value(void* value);

		script_value(int value);
		script_value(unsigned int value);
		script_value(bool value);

		script_value(float value);
		script_value(double value);

		script_value(const char* value);
		script_value(const std::string& value);

		script_value(const entity& value);
		script_value(const array& value);
		script_value(const object& value);

		script_value(const function& value);

		script_value(const vector& value);

		template <typename T>
		bool is() const;

		template <typename T>
		T as() const
		{
			if (!this->is<T>())
			{
				const auto type = get_typename(this->get_raw());
				const auto c_type = get_c_typename<T>();
				throw std::runtime_error(std::string("has type '" + type + "' but should be '" + c_type + "'"));
			}

			return get<T>();
		}

		std::string type_name() const
		{
			return get_typename(this->get_raw());
		}

		template <template<class, class> class C, class T, typename ArrayType = array>
		script_value(const C<T, std::allocator<T>>& container)
		{
			ArrayType array_{};

			for (const auto& value : container)
			{
				array_.push(value);
			}

			game::VariableValue value{};
			value.type = game::SCRIPT_OBJECT;
			value.u.pointerValue = array_.get_entity_id();

			this->value_ = value;
		}

		template<class ...T>
		arguments operator()(T... arguments) const
		{
			return this->as<function>().call({arguments...});
		}

		std::string to_string() const;

		const game::VariableValue& get_raw() const;

		variable_value value_{};

	private:
		template <typename T>
		T get() const;

	};

	class variadic_args : public arguments
	{
	};

	class function_argument
	{
	public:
		function_argument(const arguments& args, const script_value& value, const int index);

		template <typename T>
		T as() const
		{
			try
			{
				return this->value_.as<T>();
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(utils::string::va("parameter %d %s",
					this->index_ + 1, e.what()));
			}
		}

		template <>
		variadic_args as() const
		{
			variadic_args args{};
			for (auto i = this->index_; i < static_cast<int>(this->values_.size()); i++)
			{
				args.push_back(this->values_[i]);
			}
			return args;
		}

		template <>
		script_value as() const
		{
			return this->value_;
		}

		template <typename T>
		operator T() const
		{
			return this->as<T>();
		}

	private:
		arguments values_{};
		script_value value_{};
		int index_{};
	};

	class function_arguments
	{
	public:
		function_arguments(const arguments& values);

		function_argument operator[](const int index) const
		{
			if (index >= static_cast<int>(values_.size()))
			{
				throw std::runtime_error(utils::string::va("parameter %d does not exist", index));
			}

			return {values_, values_[index], index};
		}
	private:
		arguments values_{};
	};
}
