// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/common/uuid.h"

#include <rpc.h>

namespace ygg
{
    UUID create_uuid()
    {
        static_assert(sizeof(::UUID) == sizeof(UUID));
        ::UUID native_uuid = {};
        UuidCreateSequential(&native_uuid);
        UUID result = {};
        memcpy(&result, &native_uuid, sizeof(::UUID));
        return result;
    }

    std::string uuid_to_string(const UUID& uuid)
    {
        ::UUID native_uuid = {};
        memcpy(&native_uuid, &uuid, sizeof(::UUID));
        RPC_CSTR uuid_cstr = nullptr;
        UuidToString(&native_uuid, &uuid_cstr);
        std::string result(reinterpret_cast<const char*>(uuid_cstr), strlen(reinterpret_cast<const char*>(uuid_cstr)));
        RpcStringFree(&uuid_cstr);
        return result;
    }
}
