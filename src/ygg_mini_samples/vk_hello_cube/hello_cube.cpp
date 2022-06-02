// Copyright 2022 Robert Ryan. See Licence.md.

#include <ygg/vulkan/image_utils.h>
#include <ygg_mini_sample_base/base_application.h>
#include <volk.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace ygg;
using namespace ygg::mini_sample;

constexpr static uint32_t WINDOW_WIDTH = 720;
constexpr static uint32_t WINDOW_HEIGHT = 480;

class App final : public Base_app
{
public:
    using Base_app::Base_app;

    virtual void init() override
    {
        vk::Image_info color_attachment_info = {
            .width = WINDOW_WIDTH,
            .height = WINDOW_HEIGHT,
            .depth = 1,
            .mip_levels = 1,
            .array_layers = 1,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .type = VK_IMAGE_TYPE_2D
        };
        m_color_attachment = create_managed_image(color_attachment_info);

        vk::Image_info depth_attachment_info = {
            .width = WINDOW_WIDTH,
            .height = WINDOW_HEIGHT,
            .depth = 1,
            .mip_levels = 1,
            .array_layers = 1,
            .format = VK_FORMAT_D32_SFLOAT,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .type = VK_IMAGE_TYPE_2D
        };
        m_depth_attachment = create_managed_image(depth_attachment_info);

        vk::Descriptor_set_layout_info set_layout_info = {
            .flags = 0,
            .bindings = {
                /**
                 * Instead of utilizing the fixed function vertex pipeline,
                 * we will use vertex pulling in this example.
                */
                {
                    .binding = 0,
                    .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .count = 1,
                    .stages = VK_SHADER_STAGE_VERTEX_BIT,
                    .immutable_samplers = {},
                    .flags = 0
                },
                /**
                 * Uniform binding for rotation and projection.
                */
                {
                    .binding = 1,
                    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .count = 1,
                    .stages = VK_SHADER_STAGE_VERTEX_BIT,
                    .immutable_samplers = {},
                    .flags = 0
                }
            }
        };
        m_descriptor_layout = create_managed_descriptor_set_layout(set_layout_info);

        auto set_layout_arr = std::to_array<VkDescriptorSetLayout>({ m_descriptor_layout });
        vk::Pipeline_layout_info pipe_layout_info = {
            .layouts = set_layout_arr,
            .push_constant_ranges = {}
        };
        m_pipeline_layout = create_managed_pipeline_layout(pipe_layout_info);

        Graphics_pipeline_create_info pipe_info = {
            .paths = {
                .vert = "res/shader/mini_sample/hello_cube/cube.vert",
                .frag = "res/shader/mini_sample/hello_cube/cube.frag"
            },
            .info = {
                .flags = 0,
                .program = {},
                .input_assembly_state = {
                    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                    .primitive_restart_enable = false
                },
                .raster_state = {
                    .depth_clamp_enable = false,
                    .rasterizer_discard_enable = false,
                    .polygon_mode = VK_POLYGON_MODE_FILL,
                    .cull_mode = VK_CULL_MODE_BACK_BIT,
                    .front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                    .depth_bias_enable = false
                },
                .depth_stencil_state = {{
                    .depth_test_enable = true,
                    .depth_write_enable = true,
                    .compare_op = VK_COMPARE_OP_LESS,
                    .depth_format = VK_FORMAT_D32_SFLOAT
                }},
                .color_blend_state = {{
                    .logic_op_enable = false
                }},
                .render_target_infos = {
                    {
                        .blend_enable = false,
                        .format = VK_FORMAT_R8G8B8A8_SRGB
                    }
                },
                .layout = m_pipeline_layout
            }
        };
        m_graphics_pipeline = create_managed_graphics_pipeline(pipe_info);

        struct Vertex
        {
            glm::vec3 pos;
            glm::u8vec4 color;
        };

        std::vector<Vertex> cube_verts = {
            {.pos = { -0.5f, -0.5f, -0.5f }, .color = { 100, 150, 200, 255 } },
            {.pos = { -0.5f, -0.5f,  0.5f }, .color = { 100, 150,   0, 255 } },
            {.pos = {  0.5f, -0.5f, -0.5f }, .color = { 100, 150, 200, 255 } },
            {.pos = {  0.5f, -0.5f,  0.5f }, .color = { 100, 150,   0, 255 } },
            {.pos = { -0.5f,  0.5f, -0.5f }, .color = { 100,  50, 200, 255 } },
            {.pos = { -0.5f,  0.5f,  0.5f }, .color = { 100,  50,   0, 255 } },
            {.pos = {  0.5f,  0.5f, -0.5f }, .color = { 100,  50, 200, 255 } },
            {.pos = {  0.5f,  0.5f,  0.5f }, .color = { 100,  50,   0, 255 } }
        };
        vk::Buffer_info cube_buffer_info = {
            .domain = vk::Buffer_domain::Device,
            .size = cube_verts.size() * sizeof(Vertex),
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
        };
        m_cube_buffer = create_managed_buffer(cube_buffer_info);
        Buffer_upload vertices_upload = {
            .dst = m_cube_buffer,
            .data = cube_verts.data(),
            .size = sizeof(Vertex) * cube_verts.size(),
            .offset = 0
        };
        add_initial_upload(vertices_upload);

        std::vector<uint16_t> cube_indices = {
            2,1,0,3,1,2,
            4,5,6,6,5,7,
            5,1,7,7,1,3,
            7,3,6,6,3,2,
            6,2,0,0,4,6,
            4,1,5,0,1,4
        };
        vk::Buffer_info cube_indices_info = {
            .domain = vk::Buffer_domain::Device,
            .size = cube_verts.size() * sizeof(Vertex),
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
        };
        m_cube_index_buffer = create_managed_buffer(cube_indices_info);
        Buffer_upload indices_upload = {
            .dst = m_cube_index_buffer,
            .data = cube_indices.data(),
            .size = sizeof(uint16_t) * cube_indices.size(),
            .offset = 0,
        };
        add_initial_upload(indices_upload);

        vk::Buffer_info uniform_buffer_info = {
            .domain = vk::Buffer_domain::Device_host_visible,
            .size = sizeof(glm::mat4),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
        };
        m_uniform_buffer = create_managed_buffer(uniform_buffer_info);
    }

    virtual void render(vk::Graphics_command_buffer& cmdbuf,
        vk::Frame_context& frame_ctx, const util::Clock& clock) override
    {
        auto& col_attachment = img_from_handle(m_color_attachment);
        auto& dep_attachment = img_from_handle(m_depth_attachment);

        cmdbuf.pipeline_barrier_builder()
            .push_image_memory_barrier(
                VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_CLEAR_BIT, VK_ACCESS_2_MEMORY_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                col_attachment.allocated_image.handle, vk::img_utils::get_image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT))
            .push_image_memory_barrier(
                VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_CLEAR_BIT, VK_ACCESS_2_MEMORY_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                dep_attachment.allocated_image.handle, vk::img_utils::get_image_subresource_range(VK_IMAGE_ASPECT_DEPTH_BIT))
            .flush(0);

        auto set = frame_ctx.allocate_transient_descriptor_set(m_descriptor_layout);
        vk::Descriptor_buffer_info vert_desc_buf_info = descriptor_buffer_info(m_cube_buffer, 0, VK_WHOLE_SIZE);
        auto vert_desc_buf_info_arr = std::to_array<vk::Descriptor_buffer_info>({ vert_desc_buf_info });
        vk::Descriptor_buffer_info uniform_desc_buf_info = descriptor_buffer_info(m_uniform_buffer, 0, VK_WHOLE_SIZE);
        auto uniform_desc_buf_info_arr = std::to_array<vk::Descriptor_buffer_info>({ uniform_desc_buf_info });
        auto desc_updates = std::to_array<vk::Descriptor_set_write_info>({
            {
                .set = set,
                .binding = 0,
                .array_index = 0,
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .buffer_infos = { vert_desc_buf_info_arr }
            },
            {
                .set = set,
                .binding = 1,
                .array_index = 0,
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .buffer_infos = { uniform_desc_buf_info_arr }
            }
            });
        update_descriptor_sets(desc_updates);

        auto color_attachments = std::to_array<vk::Rendering_info::Attachment_info>({
            {
                .view = col_attachment.allocated_image.default_view,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .store_op = VK_ATTACHMENT_STORE_OP_STORE,
                .clear_value = {
                    .color = {
                        .f32 = {
                            (0.125f * std::sin(clock.time_since_start() * 1.00f) + 0.25f),
                            (0.125f * std::sin(clock.time_since_start() * 1.75f) + 0.25f),
                            (0.125f * std::sin(clock.time_since_start() * 2.50f) + 0.25f),
                            1.0f
                        }
                    }
                }
            }
            });
        vk::Rendering_info ri = {
            .offset_x = 0,
            .offset_y = 0,
            .width = WINDOW_WIDTH,
            .height = WINDOW_HEIGHT,
            .color_attachments = {{ color_attachments.begin(), color_attachments.end() }},
            .depth_attachment = {{
                .view = dep_attachment.allocated_image.default_view,
                .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .store_op = VK_ATTACHMENT_STORE_OP_STORE,
                .clear_value = {
                    .depth_stencil = {
                        .depth = 1.0f
                    }
                }
            }}
        };

        const auto& w_data = window_data();
        glm::mat4 proj = glm::perspective(
            glm::radians(60.0f),
            float_t(w_data.width) / float_t(w_data.height),
            0.05f,
            10.0f);
        glm::mat4 view = glm::lookAt(
            glm::vec3(-2.0f, 0.0f, 0.0f),
            glm::vec3(0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 angle = glm::normalize(glm::vec3(
            0.5f + glm::sin(clock.time_since_start() * 0.25f),
            0.5f + glm::sin(clock.time_since_start() * 0.5f),
            1.25f + glm::sin(clock.time_since_start() * 1.25f)));
        glm::mat4 rotation = glm::rotate(
            glm::mat4(1.0f),
            clock.time_since_start(),
            angle);
        glm::mat4 transform = proj * view * rotation;
        cmdbuf.upload_buffer_data(buf_from_handle(m_uniform_buffer), &transform, sizeof(glm::mat4), 0);

        cmdbuf.begin_rendering(ri);
        cmdbuf.set_viewport(0.0f, float_t(WINDOW_HEIGHT), float_t(WINDOW_WIDTH), -float_t(WINDOW_HEIGHT));
        cmdbuf.set_scissor(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        auto& pipeline = pipeline_from_handle(m_graphics_pipeline);
        cmdbuf.bind_descriptor_set(pipeline.bind_point, m_pipeline_layout, 0, set);
        cmdbuf.bind_pipeline(pipeline);
        auto& cube_index_buffer = buf_from_handle(m_cube_index_buffer);
        cmdbuf.bind_index_buffer(cube_index_buffer, 0, VK_INDEX_TYPE_UINT16);
        cmdbuf.draw_indexed(36);
        cmdbuf.end_rendering();
    }

    virtual void swapchain_pass(vk::Graphics_command_buffer& cmdbuf, vk::Image& swapchain_img) override
    {
        auto& col_attachment = img_from_handle(m_color_attachment);

        cmdbuf.pipeline_barrier_builder()
            .push_image_memory_barrier(
                VK_PIPELINE_STAGE_2_CLEAR_BIT, VK_ACCESS_2_MEMORY_READ_BIT,
                VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                col_attachment.allocated_image.handle, vk::img_utils::get_image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT))
            .push_image_memory_barrier(
                VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_MEMORY_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                swapchain_img.allocated_image.handle, vk::img_utils::get_image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT))
            .flush(0);

        vk::Image_blit_info blit_info = {
            .src_mip_level = 0,
            .src_base_array_layer = 0,
            .src_layer_count = 1,
            .src_offsets = {
                { 0, 0, 0 },
                { WINDOW_WIDTH, WINDOW_HEIGHT, 1 }
            },
            .dst_mip_level = 0,
            .dst_base_array_layer = 0,
            .dst_layer_count = 1,
            .dst_offsets = {
                { 0, 0, 0 },
                { int32_t(swapchain_img.info.width), int32_t(swapchain_img.info.height), 1 }
            },
            .filter = VK_FILTER_LINEAR,
            .aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT
        };
        cmdbuf.blit(col_attachment, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            swapchain_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, blit_info);

        cmdbuf.pipeline_barrier_builder()
            .push_image_memory_barrier(
                VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                swapchain_img.allocated_image.handle, vk::img_utils::get_image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT))
            .flush(0);
    }

private:
    Image_handle m_color_attachment;
    Image_handle m_depth_attachment;
    VkDescriptorSetLayout m_descriptor_layout;
    VkPipelineLayout m_pipeline_layout;
    Graphics_pipeline_handle m_graphics_pipeline;
    Buffer_handle m_cube_buffer;
    Buffer_handle m_cube_index_buffer;
    Buffer_handle m_uniform_buffer;
};

int32_t main(uint32_t, char*[])
{
    App app(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Vulkan Cube!");
    app.run();
    return 0;
}
