// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/glsl_compiler.h"

#include <fstream>
#include <algorithm>
#include <set>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <volk.h>

namespace ygg::vk::glsl_compiler
{
    void init()
    {
        glslang::InitializeProcess();
    }

    void deinit()
    {
        glslang::FinalizeProcess();
    }

    EShLanguage Shader_flags_to_lang(VkShaderStageFlags type)
    {
        switch (type)
        {
        case VK_SHADER_STAGE_VERTEX_BIT:                  return EShLanguage::EShLangVertex;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:    return EShLanguage::EShLangTessControl;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return EShLanguage::EShLangTessEvaluation;
        case VK_SHADER_STAGE_GEOMETRY_BIT:                return EShLanguage::EShLangGeometry;
        case VK_SHADER_STAGE_FRAGMENT_BIT:                return EShLanguage::EShLangFragment;
        case VK_SHADER_STAGE_COMPUTE_BIT:                 return EShLanguage::EShLangCompute;
        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:              return EShLanguage::EShLangRayGen;
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:             return EShLanguage::EShLangAnyHit;
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:         return EShLanguage::EShLangClosestHit;
        case VK_SHADER_STAGE_MISS_BIT_KHR:                return EShLanguage::EShLangMiss;
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:        return EShLanguage::EShLangIntersect;
        case VK_SHADER_STAGE_CALLABLE_BIT_KHR:            return EShLanguage::EShLangCallable;
        case VK_SHADER_STAGE_TASK_BIT_NV:                 return EShLanguage::EShLangTaskNV;
        case VK_SHADER_STAGE_MESH_BIT_NV:                 return EShLanguage::EShLangMeshNV;
        default:                                          return EShLanguage::EShLangCount;
        }
    };

    constexpr int32_t CLIENT_INPUT_SEMANTICS_VERSION = 130;
    constexpr glslang::EshTargetClientVersion VULKAN_CLIENT_VERSION = glslang::EShTargetVulkan_1_3;
    constexpr glslang::EShTargetLanguageVersion TARGET_SPIRV_VERSION = glslang::EShTargetSpv_1_6;
    constexpr int32_t DEFAULT_VERSION = 100;
    constexpr TBuiltInResource DEFAULT_RESOURCE = {};

    bool preprocess(glslang::TShader& shader, EShMessages messages,
        std::string& preprocess_target, glslang::TShader::Includer& includer)
    {
        return shader.preprocess(
            &DEFAULT_RESOURCE, DEFAULT_VERSION,
            ENoProfile, false, false, messages,
            &preprocess_target, includer);
    }

    bool parse(glslang::TShader& shader, EShMessages messages, std::string& preprocessed_glsl)
    {
        const char* cstr = preprocessed_glsl.c_str();
        shader.setStrings(&cstr, 1);
        return shader.parse(&DEFAULT_RESOURCE, DEFAULT_VERSION, false, messages);
    }

    bool link(glslang::TProgram& program, glslang::TShader& shader, EShMessages messages)
    {
        program.addShader(&shader);
        return program.link(messages);
    }

    std::vector<uint32_t> compile_spirv_1_6(EShLanguage type, glslang::TProgram& program)
    {
        spv::SpvBuildLogger logger = {};
        glslang::SpvOptions options = {};
        options.disableOptimizer = false;
        std::vector<uint32_t> bytecode = {};
        glslang::GlslangToSpv(*program.getIntermediate(type), bytecode, &logger, &options);
        return bytecode;
    }

    std::vector<uint32_t> compile_spirv_1_6(const std::string& code, VkShaderStageFlags shader_stage)
    {
        return compile_spirv_1_6(code, shader_stage, {});
    }

    std::vector<uint32_t> compile_spirv_1_6(const std::string& code, VkShaderStageFlags shader_stage,
        const std::span<std::string>& include_dirs)
    {
        auto type = Shader_flags_to_lang(shader_stage);
        auto shader = glslang::TShader(type);
        auto messages = EShMessages(EShMsgSpvRules | EShMsgVulkanRules);
        auto includer = DirStackFileIncluder();

        const char* cstr_code = code.c_str();
        shader.setStrings(&cstr_code, 1);
        shader.setEnvInput(glslang::EShSourceGlsl, type, glslang::EShClientVulkan, CLIENT_INPUT_SEMANTICS_VERSION);
        shader.setEnvClient(glslang::EShClientVulkan, VULKAN_CLIENT_VERSION);
        shader.setEnvTarget(glslang::EShTargetSpv, TARGET_SPIRV_VERSION);

        for (const auto& inc_dir : include_dirs) {
            includer.pushExternalLocalDirectory(inc_dir);
        }

        std::string preprocessed_glsl = {};
        glslang::TProgram program = {};

        if (!preprocess(shader, messages, preprocessed_glsl, includer)) {
            throw Glsl_compiler_error();
        }
        if (!parse(shader, messages, preprocessed_glsl)) {
            throw Glsl_compiler_error();
        }
        if (!link(program, shader, messages)) {
            throw Glsl_compiler_error();
        }

        return compile_spirv_1_6(type, program);
    }

    std::vector<uint32_t> compile_spirv_1_6_unchecked(const std::string& code, VkShaderStageFlags shader_stage)
    {
        return compile_spirv_1_6_unchecked(code, shader_stage, {});
    }

    std::vector<uint32_t> compile_spirv_1_6_unchecked(const std::string& code, VkShaderStageFlags shader_stage,
        const std::span<std::string>& include_dirs)
    {
        auto type = Shader_flags_to_lang(shader_stage);
        auto shader = glslang::TShader(type);
        auto messages = EShMessages(EShMsgSpvRules | EShMsgVulkanRules);
        auto includer = DirStackFileIncluder();

        const char* cstr_code = code.c_str();
        shader.setStrings(&cstr_code, 1);
        shader.setEnvInput(glslang::EShSourceGlsl, type, glslang::EShClientVulkan, CLIENT_INPUT_SEMANTICS_VERSION);
        shader.setEnvClient(glslang::EShClientVulkan, VULKAN_CLIENT_VERSION);
        shader.setEnvTarget(glslang::EShTargetSpv, TARGET_SPIRV_VERSION);

        for (const auto& inc_dir : include_dirs) {
            includer.pushExternalLocalDirectory(inc_dir);
        }

        std::string preprocessed_glsl = {};
        glslang::TProgram program = {};

        preprocess(shader, messages, preprocessed_glsl, includer);
        parse(shader, messages, preprocessed_glsl);
        link(program, shader, messages);
        return compile_spirv_1_6(type, program);
    }
}
