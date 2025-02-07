#include <stdinc.hpp>
#include "loader/component_loader.hpp"

#include "game/structs.hpp"
#include "game/game.hpp"

#include "gsc.hpp"

#include <utils/io.hpp>

namespace io
{
	class component final : public component_interface
	{
	public:
		void on_startup([[maybe_unused]] plugin::plugin* plugin) override
		{
			// TODO: fix this, or is it because pluto does it already?
			//const auto fs_basegame = game::Dvar_FindVar("fs_basegame");
			//std::filesystem::current_path(fs_basegame->current.string);

			gsc::function::add_multiple([](const std::string& file, const std::string& data,
				const scripting::variadic_args& va)
			{
				auto append = false;

				if (va.size() > 0)
				{
					append = va[0];
				}

				return utils::io::write_file(file, data, append);
			}, "writefile", "io::write_file");

			gsc::function::add_multiple([](const std::string& file, const std::string& data)
			{
				return utils::io::write_file(file, data, true);
			}, "appendfile", "io::append_file");

			gsc::function::add_multiple(utils::io::file_exists, "fileexists", "io::file_exists");
			gsc::function::add_multiple(utils::io::move_file, "movefile", "io::move_file");
			gsc::function::add_multiple(utils::io::file_size, "filesize", "io::file_size");
			gsc::function::add_multiple(utils::io::create_directory, "createdirectory", "io::create_directory");
			gsc::function::add_multiple(utils::io::directory_exists, "directoryexists", "io::directory_exists");
			gsc::function::add_multiple(utils::io::directory_is_empty, "directoryisempty", "io::directory_is_empty");
			gsc::function::add_multiple(utils::io::list_files, "listfiles", "io::list_files");
			gsc::function::add_multiple(utils::io::remove_file, "removefile", "io::remove_file");

			gsc::function::add_multiple([](const std::filesystem::path& src, const scripting::variadic_args& va)
			{
				bool recursive = false;
				if (va.size() > 0)
				{
					recursive = va[0];
				}

				utils::io::remove_directory(src, recursive);
			}, "removedirectory", "io::remove_directory");

			gsc::function::add_multiple(utils::io::copy_folder, "copyfolder", "io::copy_folder");
			gsc::function::add_multiple(utils::io::copy_folder, "copydirectory", "io::copy_directory");
			gsc::function::add_multiple(static_cast<std::string(*)(const std::string&)>(utils::io::read_file), "readfile", "io::read_file");
		}
	};
}

REGISTER_COMPONENT(io::component)
