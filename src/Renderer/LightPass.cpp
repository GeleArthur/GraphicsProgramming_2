﻿#include "LightPass.h"

#include "RenderInfoBuilder.h"
#include "Swapchain.h"

#include <DescriptorSets/DescriptorLayoutBuilder.h>
#include <GraphicsPipeline/PipelineLayoutBuilder.h>
#include <Image/ImageBuilder.h>
#include <Image/SamplerBuilder.h>
#include <Scene/PVPScene.h>

namespace pvp
{
    LightPass::LightPass(const Context& context, GBuffer& gbuffer)
        : m_context{ context }
        , m_gemotry_pass{ gbuffer }
    {
        build_pipelines();
        create_images();
    }

    void LightPass::build_pipelines()
    {
        m_context.descriptor_creator->create_layout()
            .add_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .add_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build(11);

        SamplerBuilder()
            .set_filter(VK_FILTER_LINEAR)
            .set_address_mode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
            .build(m_context, m_sampler);
        m_destructor_queue.add_to_queue([&] { vkDestroySampler(m_context.device->get_device(), m_sampler.handle, nullptr); });

        m_descriptor_binding = DescriptorSetBuilder()
                                   .bind_image(0, m_gemotry_pass.get_albedo_image(), m_sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                                   .bind_image(1, m_gemotry_pass.get_normal_image(), m_sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                                   .set_layout(m_context.descriptor_creator->get_layout(11))
                                   .build(m_context);

        PipelineLayoutBuilder()
            .add_descriptor_layout(m_context.descriptor_creator->get_layout(11))
            .build(m_context.device->get_device(), m_light_pipeline_layout);
        m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_context.device->get_device(), m_light_pipeline_layout, nullptr); });

        GraphicsPipelineBuilder()
            .add_shader("shaders/lightpass.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
            .add_shader("shaders/lightpass.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
            .set_color_format(std::array{ m_context.swapchain->get_swapchain_surface_format().format })
            .set_pipeline_layout(m_light_pipeline_layout)
            .build(*m_context.device, m_light_pipeline);
        m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_light_pipeline, nullptr); });

        // m_screensize_buffer = new UniformBuffer<glm::vec2>(m_context.allocator->get_allocator());
        // m_screensize_buffer->update(0, glm::vec2(m_image_info.image_size.width, m_image_info.image_size.height));
        // m_destructor_queue.add_to_queue([&] { delete m_screensize_buffer; });
    }

    void LightPass::create_images()
    {
        ImageBuilder()
            .set_format(m_context.swapchain->get_swapchain_surface_format().format)
            .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
            .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .set_size(m_context.swapchain->get_swapchain_extent())
            .set_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .build(m_context, m_light_image);
        m_destructor_queue.add_to_queue([&] { m_light_image.destroy(m_context); });
    }

    void LightPass::draw(VkCommandBuffer cmd)
    {
        m_light_image.transition_layout(cmd,
                                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                        VK_PIPELINE_STAGE_2_NONE,
                                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                        VK_ACCESS_2_NONE,
                                        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_light_pipeline_layout, 0, 1, &m_descriptor_binding.sets[0], 0, nullptr);

        const auto render_color_info = RenderInfoBuilder()
                                           .add_color(&m_light_image, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
                                           .set_size(m_light_image.get_size())
                                           .build();

        vkCmdBeginRendering(cmd, &render_color_info.rendering_info);
        {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_light_pipeline);
            vkCmdDraw(cmd, 3, 1, 0, 0);
        }
        vkCmdEndRendering(cmd);

        m_light_image.transition_layout(cmd,
                                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                        VK_ACCESS_2_TRANSFER_READ_BIT);
    }
} // namespace pvp