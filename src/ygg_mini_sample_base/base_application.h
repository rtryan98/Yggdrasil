// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include <ygg/common/handle.h>
#include <ygg/util/clock.h>
#include <ygg/vulkan/context.h>
#include <ygg/vulkan/graphics_command_buffer.h>
#include <ygg/vulkan/swapchain.h>
#include <ygg/vulkan/window_system_integration_win32.h>
#include <ygg/window/window_win32.h>

#include <glm/glm.hpp>
#include <vector>

namespace ygg::mini_sample
{
    struct Graphics_pipeline_path_info
    {
        std::string vert;
        std::string tesc;
        std::string tese;
        std::string geom;
        std::string frag;
    };

    struct Graphics_pipeline_create_info
    {
        Graphics_pipeline_path_info paths;

        /**
         * Skip vk::Graphics_pipeline_info::program. Use paths instead.
        */
        vk::Graphics_pipeline_info info;
    };

    struct Compute_pipeline_create_info
    {
        std::string path;

        /**
         * Skip vk::Compute_pipeline_info::shader. Use path instead.
        */
        vk::Compute_pipeline_info info;
    };

    namespace detail
    {
        struct Buffer_tag {};
        struct Image_tag {};
        struct Shader_module_tag {};
        struct Graphics_pipeline_tag {};
        struct Compute_pipeline_tag {};

        struct Graphics_pipeline
        {
            Graphics_pipeline_create_info create_info;
            vk::Pipeline pipeline;
        };

        struct Compute_pipeline
        {
            Compute_pipeline_create_info create_info;
            vk::Pipeline pipeline;
        };
    }

    using Buffer_handle = Handle<detail::Buffer_tag, std::size_t>;
    using Image_handle = Handle<detail::Image_tag, std::size_t>;
    using Shader_handle = Handle<detail::Shader_module_tag, std::size_t>;
    using Graphics_pipeline_handle = Handle<detail::Graphics_pipeline_tag, std::size_t>;
    using Compute_pipeline_handle = Handle<detail::Compute_pipeline_tag, std::size_t>;

    struct Buffer_upload
    {
        Buffer_handle dst;
        void* data;
        std::size_t size;
        std::size_t offset = 0;
    };

    struct Image_upload
    {
        Image_handle dst;
        void* data;
        uint32_t width;
        uint32_t height;
        uint32_t depth = 1;
        uint32_t width_offset = 0;
        uint32_t height_offset = 0;
        uint32_t depth_offset = 0;
        uint32_t layers = 1;
        uint32_t base_layer = 0;
        uint32_t mip_level = 0;
    };

    /**
     * @brief Base application class used for very simple rendering applications.
    */
    class Base_app
    {
    public:
        Base_app(uint32_t window_width, uint32_t window_height, const std::string& title);
        virtual ~Base_app();

        Base_app(const Base_app& other) = delete;
        Base_app& operator=(const Base_app& other) = delete;
        Base_app(Base_app&& other) = delete;
        Base_app& operator=(Base_app&& other) = delete;

        void run();

        virtual void init() = 0;
        virtual void render(vk::Graphics_command_buffer& cmdbuf, vk::Frame_context& frame_ctx, const util::Clock& clock) = 0;
        virtual void swapchain_pass(vk::Graphics_command_buffer& cmdbuf, vk::Image& swapchain_img) = 0;
        virtual void cleanup() {};

        Buffer_handle create_managed_buffer(const vk::Buffer_info& info);
        Image_handle create_managed_image(const vk::Image_info& info);
        VkDescriptorSetLayout create_managed_descriptor_set_layout(const vk::Descriptor_set_layout_info& info);
        VkPipelineLayout create_managed_pipeline_layout(const vk::Pipeline_layout_info& info);
        Graphics_pipeline_handle create_managed_graphics_pipeline(const Graphics_pipeline_create_info& info);
        void update_descriptor_set(const vk::Descriptor_set_write_info& info);
        void update_descriptor_sets(std::span<vk::Descriptor_set_write_info> infos);

        vk::Buffer& buf_from_handle(Buffer_handle buf) { return m_buffers.at(std::size_t(buf)); };
        vk::Image& img_from_handle(Image_handle img) { return m_images.at(std::size_t(img)); };
        vk::Pipeline& pipeline_from_handle(Graphics_pipeline_handle p) { return m_graphics_pipelines.at(std::size_t(p)).pipeline; };
        vk::Pipeline& pipeline_from_handle(Compute_pipeline_handle p) { return m_compute_pipelines.at(std::size_t(p)).pipeline; };
        const Window_win32_data& window_data() const { return m_window.window_data(); }

        vk::Descriptor_buffer_info descriptor_buffer_info(Buffer_handle buffer, VkDeviceSize offset, VkDeviceSize size);

        void add_initial_upload(const Buffer_upload& buffer_upload);
        void add_initial_upload(const Image_upload& image_upload);

    private:
        void upload_data(vk::Graphics_command_buffer& cmdbuf);
        void frame_loop();
        vk::Allocated_buffer select_allocated_buffer(Buffer_handle buf);

    private:
        util::Clock m_clock;
        Window_win32 m_window;
        vk::Window_system_integration_win32 m_wsi;
        vk::Context m_context;
        vk::Swapchain m_swapchain;

        std::vector<Buffer_upload> m_buffer_uploads = {};
        std::vector<Image_upload> m_image_uploads = {};

        std::vector<vk::Buffer> m_buffers = {};
        std::vector<vk::Image> m_images = {};
        std::vector<VkDescriptorSetLayout> m_set_layouts = {};
        std::vector<VkPipelineLayout> m_pipeline_layouts = {};
        std::vector<detail::Graphics_pipeline> m_graphics_pipelines = {};
        std::vector<detail::Compute_pipeline> m_compute_pipelines = {};
    };
}
