// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/common/file_util.h"

#include <fstream>
#include <filesystem>
#include <sstream>

namespace ygg::file_util
{
    IO_error::IO_error()
        : std::runtime_error("IOError")
    {}

    IO_error::IO_error(const std::string& message)
        : std::runtime_error(message)
    {}

    IO_error::IO_error(const char* message)
        : std::runtime_error(message)
    {}

    std::string file_to_str(const std::string& path)
    {
        std::stringstream result = {};
        std::ifstream input = {};
        input.open(path);
        if (input.fail())
        {
            input.close();
            throw IO_error(std::string("Failed to open '") + path + "'.");
        }
        std::string line{};
        while (getline(input, line))
        {
            result << line << '\n';
        }
        input.close();
        return result.str();
    }

    std::string file_ext_to_str(const std::string& path)
    {
        std::filesystem::path p = path;
        auto s = p.extension().string();
        if (s.length() < 1)
        {
            throw IO_error(std::string("Invalid filepath '") + path + "'.");
        }
        return s.substr(1, s.length());
    }
}
