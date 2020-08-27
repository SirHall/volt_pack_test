#define VOLT_PACK_SRCS

#include "volt_pack.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>

void RecursePerms(std::filesystem::path dir, std::filesystem::perms newPerms,
                  std::filesystem::perm_options permOptions)
{
    for (auto &path : std::filesystem::recursive_directory_iterator(dir))
    {
        try
        {
            std::filesystem::permissions(path, newPerms, permOptions);
        }
        catch (std::exception &e)
        {
            std::cerr << "Permission change error for '" << dir.generic_string()
                      << "':\n\t" << e.what() << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    // wd = working directory
    std::filesystem::path wd = std::filesystem::current_path();

    std::error_code err;
    if (std::filesystem::exists(wd / "example/README.md"))
    {
        RecursePerms(wd / "example", std::filesystem::perms::owner_write,
                     std::filesystem::perm_options::add);
        std::filesystem::remove_all(wd / "example", err);
        std::cout << "Deleted old example cloned repo" << std::endl;
    }
    if (err)
    {
        std::cerr << "Could not delete old repository directory:\n\t"
                  << err.message() << std::endl;
        return 1;
    }

    volt::pack::GitClone("https://www.github.com/SirHall/example.git");

    // Check if the repo exists
    if (std::filesystem::exists(wd / "example/README.md"))
    {
        std::cout << "Successfully cloned github.com/SirHall/example"
                  << std::endl;
        return 0;
    }
    std::cerr << "Failed to clone github.com/SirHall/example" << std::endl;
    return 1;
}