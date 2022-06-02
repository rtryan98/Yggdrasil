// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg_mini_sample_base/base_application.h"

#include <volk.h>
#include <ygg/common/file_util.h>
#include <ygg/vulkan/glsl_compiler.h>

namespace ygg::mini_sample
{
    Base_app::Base_app(uint32_t window_width, uint32_t window_height, const std::string& title)
        : m_clock(), m_window(window_width, window_height, title.c_str()),
        m_wsi(m_window), m_context(m_wsi), m_swapchain(m_context, m_wsi)
    {
        vk::glsl_compiler::init();
    }

    Base_app::~Base_app()
    {
        m_context.device_wait_idle();
        for (auto& b : m_buffers) {
            m_context.destroy_buffer(b);
        }
        for (auto& i : m_images) {
            m_context.destroy_image(i);
        }
        for (auto s : m_set_layouts) {
            m_context.destroy_descriptor_set_layout(s);
        }
        for (auto p : m_pipeline_layouts) {
            m_context.destroy_pipeline_layout(p);
        }
        for (auto& p : m_graphics_pipelines) {
            m_context.destroy_pipeline(p.pipeline);
        }
        cleanup();
        vk::glsl_compiler::deinit();
    }

    void Base_app::run()
    {
        init();
        frame_loop();
    }

    Buffer_handle Base_app::create_managed_buffer(const vk::Buffer_info& info)
    {
        m_buffers.push_back(m_context.create_buffer(info, m_context.graphics_queue().queue_family_index));
        return Buffer_handle(m_buffers.size() - 1ull);
    }

    Image_handle Base_app::create_managed_image(const vk::Image_info& info)
    {
        m_images.push_back(m_context.create_image(info, m_context.graphics_queue().queue_family_index));
        return Image_handle(m_images.size() - 1ull);
    }

    VkDescriptorSetLayout Base_app::create_managed_descriptor_set_layout(const vk::Descriptor_set_layout_info& info)
    {
        m_set_layouts.push_back(m_context.create_descriptor_set_layout(info));
        return m_set_layouts.at(m_set_layouts.size() - 1ull);
    }

    VkPipelineLayout Base_app::create_managed_pipeline_layout(const vk::Pipeline_layout_info& info)
    {
        m_pipeline_layouts.push_back(m_context.create_pipeline_layout(info));
        return m_pipeline_layouts.at(m_pipeline_layouts.size() - 1ull);
    }

    std::vector<uint32_t> compile_vert_shader_fail()
    {
        std::string vert_fail_code = "#version 460 core\n"
            "void main(){\n"
            "gl_Position = vec4(1.0);\n"
            "}";
        return vk::glsl_compiler::compile_spirv_1_6_unchecked(vert_fail_code, VK_SHADER_STAGE_VERTEX_BIT);
    }

    std::vector<uint32_t> compile_frag_shader_fail()
    {
        std::string frag_fail_code = "#version 460 core\n"
            "void main(){}";
        return vk::glsl_compiler::compile_spirv_1_6_unchecked(frag_fail_code, VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    Graphics_pipeline_handle Base_app::create_managed_graphics_pipeline(const Graphics_pipeline_create_info& info)
    {
        vk::Graphics_pipeline_info create_info = info.info;
        vk::Graphics_program program = {};

        try {
            auto vert_code = file_util::file_to_str(info.paths.vert);
            std::vector<uint32_t> spirv = {};
            try {
                spirv = vk::glsl_compiler::compile_spirv_1_6(vert_code, VK_SHADER_STAGE_VERTEX_BIT);
            }
            catch (const vk::glsl_compiler::Glsl_compiler_error& e) {
                printf("Glsl compiler error.\nfile: '%s'\nmsg: '%s'\n", info.paths.vert.c_str(), e.what());
                spirv = compile_vert_shader_fail();
            }
            program.vert = m_context.create_shader_module(spirv, VK_SHADER_STAGE_VERTEX_BIT);
        }
        catch (const file_util::IO_error& e) {
            printf("Glsl compiler file read error (vert).\n%s\n", e.what());
            std::vector<uint32_t> spirv = compile_vert_shader_fail();
            program.vert = m_context.create_shader_module(spirv, VK_SHADER_STAGE_VERTEX_BIT);
        }

        if (info.paths.tesc.size()) {
            assert(false && "Not yet supported.");
        }

        if (info.paths.tese.size()) {
            assert(false && "Not yet supported.");
        }

        if (info.paths.geom.size()) {
            assert(false && "Not yet supported.");
        }

        try {
            auto frag_code = file_util::file_to_str(info.paths.frag);
            std::vector<uint32_t> spirv = {};
            try {
                spirv = vk::glsl_compiler::compile_spirv_1_6(frag_code, VK_SHADER_STAGE_FRAGMENT_BIT);
            }
            catch (const vk::glsl_compiler::Glsl_compiler_error& e) {
                printf("Glsl compiler error.\nfile: '%s'\nmsg: '%s'\n", info.paths.frag.c_str(), e.what());
                spirv = compile_frag_shader_fail();
            }
            program.frag = m_context.create_shader_module(spirv, VK_SHADER_STAGE_FRAGMENT_BIT);
        }
        catch (const file_util::IO_error& e) {
            printf("Glsl compiler file read error (frag).\n%s\n", e.what());
            std::vector<uint32_t> spirv = compile_frag_shader_fail();
            program.frag = m_context.create_shader_module(spirv, VK_SHADER_STAGE_FRAGMENT_BIT);
        }

        create_info.program = program;
        detail::Graphics_pipeline pipeline = {
            .create_info = info,
            .pipeline = m_context.create_graphics_pipeline(create_info)
        };
        m_graphics_pipelines.push_back(pipeline);
        return Graphics_pipeline_handle(m_graphics_pipelines.size() - 1ull);
    }

    void Base_app::update_descriptor_set(const vk::Descriptor_set_write_info& info)
    {
        m_context.update_descriptor_set(info);
    }

    void Base_app::update_descriptor_sets(std::span<vk::Descriptor_set_write_info> infos)
    {
        m_context.update_descriptor_sets(infos);
    }

    vk::Descriptor_buffer_info Base_app::descriptor_buffer_info(Buffer_handle buffer, VkDeviceSize offset, VkDeviceSize size)
    {
        return {
            .buffer = select_allocated_buffer(buffer).handle,
            .offset = offset,
            .range = size
        };
    }

    void Base_app::add_initial_upload(const Buffer_upload& buffer_upload)
    {
        void* data = malloc(buffer_upload.size);
        memcpy(data, buffer_upload.data, buffer_upload.size);
        Buffer_upload u = {
            .dst = buffer_upload.dst,
            .data = data,
            .size = buffer_upload.size,
            .offset = buffer_upload.offset
        };
        m_buffer_uploads.push_back(u);
    }

    void Base_app::add_initial_upload(const Image_upload& image_upload)
    {
        m_image_uploads.push_back(image_upload);
    }

    void Base_app::upload_data(vk::Graphics_command_buffer& cmdbuf)
    {
        bool has_uploads = m_buffer_uploads.size() > 0 || m_image_uploads.size() > 0;
        if (!has_uploads)
            return;

        for (const auto& buf_upload : m_buffer_uploads) {
            cmdbuf.upload_buffer_data(buf_from_handle(buf_upload.dst),
                buf_upload.data, buf_upload.size, buf_upload.offset);
            free(buf_upload.data);
        }
        m_buffer_uploads.clear();
        for (const auto& img_upload : m_image_uploads) {
            img_upload;
            assert(false);
        }
        m_image_uploads.clear();
        cmdbuf.pipeline_barrier_builder()
            .push_memory_barrier(
                VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_WRITE_BIT,
                VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT
            )
            .flush(0);
    }

    void Base_app::frame_loop()
    {
        while (!m_window.window_data().is_closed) {
            m_window.update();
            m_context.begin_frame();
            std::vector<VkCommandBuffer> submit_cmdbufs = {};

            auto cmdbuf = m_context.frame_context().acquire_graphics_command_buffer();
            cmdbuf.begin();
            upload_data(cmdbuf);
            render(cmdbuf, m_context.frame_context(), m_clock);
            submit_cmdbufs.emplace_back(cmdbuf.handle());

            std::vector<vk::Semaphore_signal_info> await_sema_infos = {};
            bool has_blitted_to_swapchain = false;
            auto& window_data = m_window.window_data();
            bool can_use_swapchain = window_data.height > 0 && window_data.width > 0;
            VkSemaphore acquire_semaphore = VK_NULL_HANDLE;
            if (can_use_swapchain) {
                acquire_semaphore = m_context.create_binary_semaphore();
                m_context.frame_context().zombify_semaphore(acquire_semaphore);
                auto acquire_result = m_swapchain.try_acquire_index_recreate_on_resize(acquire_semaphore);
                if (acquire_result != VK_SUCCESS) {
                    printf("\nUnrecoverable swapchain acquire error. VkResult: %i\n\n", acquire_result);
                    std::abort();
                }
                auto swapchain_img = m_swapchain.image();
                swapchain_pass(cmdbuf, swapchain_img);
                has_blitted_to_swapchain = true;
                await_sema_infos.emplace_back(vk::Semaphore_signal_info{
                    .semaphore = acquire_semaphore,
                    .value = 0,
                    .stage_mask = VK_PIPELINE_STAGE_2_NONE
                    });
            }
            cmdbuf.end();

            std::vector<vk::Semaphore_signal_info> signal_sema_infos = {};
            VkSemaphore submit_semaphore = VK_NULL_HANDLE;
            if (can_use_swapchain && has_blitted_to_swapchain) {
                submit_semaphore = m_context.create_binary_semaphore();
                m_context.frame_context().zombify_semaphore(submit_semaphore);
                signal_sema_infos.emplace_back(vk::Semaphore_signal_info{
                    .semaphore = submit_semaphore,
                    .value = 0,
                    .stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
                    });
            }

            vk::Submit submit_info = {
                .await_semas = { await_sema_infos },
                .cmd_bufs = { submit_cmdbufs },
                .signal_semas = { signal_sema_infos }
            };
            m_context.submit(m_context.graphics_queue().queue, submit_info, m_context.frame_fence());

            if (can_use_swapchain && has_blitted_to_swapchain) {
                auto present_result = m_swapchain.try_present_recreate_on_resize(
                    m_context.graphics_queue().queue, submit_semaphore);
                if (present_result != VK_SUCCESS) {
                    printf("\nUnrecoverable swapchain present error. VkResult: %i\n\n", present_result);
                    std::abort();
                }
            }

            m_context.end_frame();
            m_clock.next_clock_frame();
        }
    }

    vk::Allocated_buffer Base_app::select_allocated_buffer(Buffer_handle buf)
    {
        return m_context.select_allocated_buffer(buf_from_handle(buf));
    }
}
