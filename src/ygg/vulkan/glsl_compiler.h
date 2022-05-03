// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/vk_forward_decl.h"

#include <stdexcept>
#include <span>
#include <string>
#include <vector>

namespace ygg::vk::glsl_compiler
{
    /**
     * @brief Initializes the glsl compiler. Must be called before using it.
    */
    void init();

    /**
     * @brief Finalizes the glsl compiler. Must be called before closing the process.
    */
    void deinit();

    /**
     * @brief Error thrown when compiling invalid GLSL code.
    */
    class Glsl_compiler_error : public std::runtime_error
    {
    public:
        Glsl_compiler_error(const std::string& s) : std::runtime_error(s) {}
        Glsl_compiler_error() = default;
    };

    /**
     * @brief Compiles the provided code into Spirv 1.6.
     * @details This function may throw if the provided code could not compile.
    */
    std::vector<uint32_t> compile_spirv_1_6(const std::string& code, VkShaderStageFlags shader_stage);

    /**
     * @brief Compiles the provided code into Spirv 1.6.
     * @details This function may throw if the provided code could not compile.
    */
    std::vector<uint32_t> compile_spirv_1_6(const std::string& code, VkShaderStageFlags shader_stage,
        const std::span<std::string>& include_dirs);

    /**
     * @brief Compiles the provided code into Spirv 1.6.
     * @details This function may *crash* if the provided code could not compile.
    */
    std::vector<uint32_t> compile_spirv_1_6_unchecked(const std::string& code, VkShaderStageFlags shader_stage);

    /**
     * @brief Compiles the provided code into Spirv 1.6.
     * @details This function may *crash* if the provided code could not compile.
    */
    std::vector<uint32_t> compile_spirv_1_6_unchecked(const std::string& code, VkShaderStageFlags shader_stage,
        const std::span<std::string>& include_dirs);
}
