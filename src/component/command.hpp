#pragma once

namespace command
{
	class params
	{
	public:
		params();

		int size() const;
		const char* get(int index) const;
		std::string join(int index) const;

		const char* operator[](const int index) const
		{
			return this->get(index);
		}

	private:
		int nesting_;
	};

	class params_sv
	{
	public:
		params_sv();

		int size() const;
		const char* get(int index) const;
		std::string join(int index) const;

		const char* operator[](const int index) const
		{
			return this->get(index);
		}

	private:
		int nesting_;
	};

	void add_raw(const char* name, void (*callback)());
	void add(const char* name, std::function<void(params&)> callback);
	void add_sv(const std::string& name, std::function<void(int, const params_sv&)> callback);

	void add_script_command(const std::string& name, const std::function<void(const params&)>& callback);
	void add_script_sv_command(const std::string& name, const std::function<void(int, const params_sv&)>& callback);
	void clear_script_commands();

	void execute(std::string command, const bool sync = false);
}