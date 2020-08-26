#define VOLT_PACK_SRCS

#include "volt_pack.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>

void PrintList(std::vector<std::string> const &list)
{
    for (std::size_t i = 0; i < list.size(); i++)
        std::cout << list[i] << std::endl;
}

int main(int argc, char *argv[])
{
    std::cout << std::filesystem::current_path() << std::endl;

    auto filePath = std::filesystem::current_path().append("./res/net_con.cpp");

    if (!std::filesystem::exists(filePath))
    {
        std::cerr << "File '" << filePath.generic_string() << "' does not exist"
                  << std::endl;
        return 1;
    }
    auto includes = volt::pack::GetIncludes(filePath);

    std::vector<std::string> trueIncludes = {
        "volt/net/net_con.hpp",
        "volt/event/global_event.hpp",
        "volt/net/event_types/e_closed_con.hpp",
        "volt/net/event_types/e_new_con.hpp",
        "volt/net/network.hpp",
        "volt/net/volt_net_defs.hpp",
        "assert.h",
        "iostream",
        "string"};

    if (includes.size() != trueIncludes.size())
    {
        std::cerr << "Lists are not equal sizes" << std::endl;
        std::cerr << "Includes: " << includes.size() << std::endl;
        std::cerr << "Truth:    " << trueIncludes.size() << std::endl;
        PrintList(includes);
        return 1;
    }

    for (std::size_t i = 0; i < includes.size(); i++)
    {
        if (includes[i] != trueIncludes[i])
        {
            std::cerr << "List elements were not equal: " << includes[i]
                      << std::endl;
            PrintList(includes);
            return 1;
        }
    }
    std::cout << "Test passed" << std::endl;
    return 0;
}