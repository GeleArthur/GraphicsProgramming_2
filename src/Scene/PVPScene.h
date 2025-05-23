﻿#pragma once
#include "Camera.h"
#include "ModelData.h"

#include <filesystem>
#include <vector>
#include <Buffer/Buffer.h>
#include <Context/Context.h>
#include <DescriptorSets/DescriptorSets.h>
#include <Image/Sampler.h>
#include <UniformBuffers/UniformBuffer.h>
#include <glm/mat4x4.hpp>
#include <glm/detail/func_packing_simd.inl>

namespace pvp
{
    struct Sampler;
}
namespace pvp
{

    struct MaterialTransform
    {
        glm::mat4x4 transform;
        uint32_t    diffuse_texture_index;
        uint32_t    normal_texture_index;
        uint32_t    metalness_texture_index;
    };

    struct Model
    {
        Buffer            vertex_data;
        Buffer            index_data;
        uint32_t          index_count;
        MaterialTransform material;
    };

    struct SceneGlobals
    {
        glm::mat4x4 camera_view_projection;
        glm::vec3   lights[1];
    };

    class PvpScene
    {
    public:
        explicit PvpScene(Context& context);
        ~PvpScene();
        void update();
        void update_render() const;

        const std::vector<Model>& get_models() const
        {
            return m_gpu_models;
        };

        const std::vector<Image>& get_textures() const
        {
            return m_gpu_textures;
        };

        UniformBuffer<SceneGlobals>* get_scene_globals() const
        {
            return m_scene_globals_gpu;
        };

        const DescriptorSets& get_scene_descriptor() const
        {
            return m_scene_binding;
        }
        const DescriptorSets& get_textures_descriptor() const
        {
            return m_all_textures;
        }

    private:
        Context&           m_context;
        std::vector<Model> m_gpu_models;
        std::vector<Image> m_gpu_textures;
        Camera             m_camera;
        DescriptorSets     m_scene_binding;
        DescriptorSets     m_all_textures;
        Sampler            m_shadered_sampler;
        SceneGlobals       m_scene_globals;

        UniformBuffer<SceneGlobals>* m_scene_globals_gpu{};
    };

} // namespace pvp