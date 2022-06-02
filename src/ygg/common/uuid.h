// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include <cstdint>
#include <string>

namespace ygg
{
    struct UUID
    {
        uint32_t data1;
        uint16_t data2;
        uint16_t data3;
        uint64_t data4;
    };

    UUID create_uuid();

    std::string uuid_to_string(const UUID& uuid);
}
