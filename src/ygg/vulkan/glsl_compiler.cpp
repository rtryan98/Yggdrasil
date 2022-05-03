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
    constexpr TBuiltInResource DEFAULT_RESOURCE = {
        .maxLights = 32,
        .maxClipPlanes = 6,
        .maxTextureUnits = 32,
        .maxTextureCoords = 32,
        .maxVertexAttribs = 64,
        .maxVertexUniformComponents = 4096,
        .maxVaryingFloats = 64,
        .maxVertexTextureImageUnits = 32,
        .maxCombinedTextureImageUnits = 80,
        .maxTextureImageUnits = 32,
        .maxFragmentUniformComponents = 4096,
        .maxDrawBuffers = 32,
        .maxVertexUniformVectors = 128,
        .maxVaryingVectors = 8,
        .maxFragmentUniformVectors = 16,
        .maxVertexOutputVectors = 16,
        .maxFragmentInputVectors = 15,
        .minProgramTexelOffset = -8,
        .maxProgramTexelOffset = 7,
        .maxClipDistances = 8,
        .maxComputeWorkGroupCountX = 65535,
        .maxComputeWorkGroupCountY = 65535,
        .maxComputeWorkGroupCountZ = 65535,
        .maxComputeWorkGroupSizeX = 1024,
        .maxComputeWorkGroupSizeY = 1024,
        .maxComputeWorkGroupSizeZ = 64,
        .maxComputeUniformComponents = 1024,
        .maxComputeTextureImageUnits = 16,
        .maxComputeImageUniforms = 8,
        .maxComputeAtomicCounters = 8,
        .maxComputeAtomicCounterBuffers = 1,
        .maxVaryingComponents = 60,
        .maxVertexOutputComponents = 64,
        .maxGeometryInputComponents = 64,
        .maxGeometryOutputComponents = 128,
        .maxFragmentInputComponents = 128,
        .maxImageUnits = 8,
        .maxCombinedImageUnitsAndFragmentOutputs = 8,
        .maxCombinedShaderOutputResources = 8,
        .maxImageSamples = 0,
        .maxVertexImageUniforms = 0,
        .maxTessControlImageUniforms = 0,
        .maxTessEvaluationImageUniforms = 0,
        .maxGeometryImageUniforms = 0,
        .maxFragmentImageUniforms = 8,
        .maxCombinedImageUniforms = 8,
        .maxGeometryTextureImageUnits = 16,
        .maxGeometryOutputVertices = 256,
        .maxGeometryTotalOutputComponents = 1024,
        .maxGeometryUniformComponents = 1024,
        .maxGeometryVaryingComponents = 64,
        .maxTessControlInputComponents = 128,
        .maxTessControlOutputComponents = 128,
        .maxTessControlTextureImageUnits = 16,
        .maxTessControlUniformComponents = 1024,
        .maxTessControlTotalOutputComponents = 4096,
        .maxTessEvaluationInputComponents = 128,
        .maxTessEvaluationOutputComponents = 128,
        .maxTessEvaluationTextureImageUnits = 16,
        .maxTessEvaluationUniformComponents = 1024,
        .maxTessPatchComponents = 120,
        .maxPatchVertices = 32,
        .maxTessGenLevel = 64,
        .maxViewports = 16,
        .maxVertexAtomicCounters = 0,
        .maxTessControlAtomicCounters = 0,
        .maxTessEvaluationAtomicCounters = 0,
        .maxGeometryAtomicCounters = 0,
        .maxFragmentAtomicCounters = 8,
        .maxCombinedAtomicCounters = 8,
        .maxAtomicCounterBindings = 1,
        .maxVertexAtomicCounterBuffers = 0,
        .maxTessControlAtomicCounterBuffers = 0,
        .maxTessEvaluationAtomicCounterBuffers = 0,
        .maxGeometryAtomicCounterBuffers = 0,
        .maxFragmentAtomicCounterBuffers = 1,
        .maxCombinedAtomicCounterBuffers = 1,
        .maxAtomicCounterBufferSize = 16384,
        .maxTransformFeedbackBuffers = 4,
        .maxTransformFeedbackInterleavedComponents = 64,
        .maxCullDistances = 8,
        .maxCombinedClipAndCullDistances = 8,
        .maxSamples = 4,
        .maxMeshOutputVerticesNV = 256,
        .maxMeshOutputPrimitivesNV = 512,
        .maxMeshWorkGroupSizeX_NV = 32,
        .maxMeshWorkGroupSizeY_NV = 1,
        .maxMeshWorkGroupSizeZ_NV = 1,
        .maxTaskWorkGroupSizeX_NV = 32,
        .maxTaskWorkGroupSizeY_NV = 1,
        .maxTaskWorkGroupSizeZ_NV = 1,
        .maxMeshViewCountNV = 4,
        .maxDualSourceDrawBuffersEXT = 1,
        .limits = {
            .nonInductiveForLoops = 1,
            .whileLoops = 1,
            .doWhileLoops = 1,
            .generalUniformIndexing = 1,
            .generalAttributeMatrixVectorIndexing = 1,
            .generalVaryingIndexing = 1,
            .generalSamplerIndexing = 1,
            .generalVariableIndexing = 1,
            .generalConstantMatrixVectorIndexing = 1,
        }
    };

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
            throw Glsl_compiler_error(std::string("Shader preprocess error.\n") + shader.getInfoLog());
        }
        if (!parse(shader, messages, preprocessed_glsl)) {
            throw Glsl_compiler_error(std::string("Shader parse error.\n") + shader.getInfoLog());
        }
        if (!link(program, shader, messages)) {
            throw Glsl_compiler_error(std::string("Shader link error.\n") + shader.getInfoLog());
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
