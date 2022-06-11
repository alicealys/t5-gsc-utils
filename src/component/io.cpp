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
        void post_unpack() override
        {
            gsc::function::add("writefile", [](const std::string& file, const std::string& data,
                const scripting::variadic_args& va)
            {
                auto append = false;

                if (va.size() > 0)
                {
                    append = va[0];
                }

                return utils::io::write_file(file, data, append);
            });

            gsc::function::add("appendfile", [](const std::string& file, const std::string& data)
            {
                return utils::io::write_file(file, data, true);
            });

            gsc::function::add("fileexists", utils::io::file_exists);
            gsc::function::add("movefile", utils::io::move_file);
            gsc::function::add("filesize", utils::io::file_size);
            gsc::function::add("createdirectory", utils::io::create_directory);
            gsc::function::add("directoryexists", utils::io::directory_exists);
            gsc::function::add("directoryisempty", utils::io::directory_is_empty);
            gsc::function::add("listfiles", utils::io::list_files);
            gsc::function::add("removefile", utils::io::remove_file);

            gsc::function::add("removedirectory", [](const std::filesystem::path& src, const scripting::variadic_args& va)
            {
                bool recursive = false;
                if (va.size() > 0)
                {
                    recursive = va[0];
                }

                utils::io::remove_directory(src, recursive);
            });

            gsc::function::add("copyfolder", utils::io::copy_folder);
            gsc::function::add("copydirectory", utils::io::copy_folder);
            gsc::function::add("readfile", static_cast<std::string(*)(const std::string&)>(utils::io::read_file));
        }
    };
}

REGISTER_COMPONENT(io::component)
