// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/memory/sparse_pool.h"

#include <cstdint>
#include <iostream>
#include <thread>

int32_t main(uint32_t*, const char*[])
{
    ygg::memory::sparse_pool<uint64_t> pool;
    pool.emplace(0);
    pool.emplace(1);
    pool.emplace(2);
    pool.remove(0);
    pool.remove(1);
    pool.remove(2);
    std::cout << pool.emplace(42) << std::endl;
    std::cout << pool.emplace(420) << std::endl;
    std::cout << pool.emplace(4200) << std::endl;

    return 0;
}
