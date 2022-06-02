// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include <stdexcept>
#include <string>

namespace ygg::file_util
{
    class IO_error : public std::runtime_error
    {
    public:
        explicit IO_error();
        explicit IO_error(const std::string& message);
        explicit IO_error(const char* message);
    };

    std::string file_to_str(const std::string& path);

    std::string file_ext_to_str(const std::string& path);
}
