﻿#pragma once

#include <vulkan/vulkan.h>

namespace pvp
{
    class WindowSurface;
    class Swapchain;
    class Device;
    class PvpVmaAllocator;
    class PhysicalDevice;
    class Instance;
    class QueueFamilies;
    class DescriptorLayoutCreator;
    struct Context
    {
        Instance*                instance;
        PhysicalDevice*          physical_device;
        Device*                  device;
        PvpVmaAllocator*         allocator;
        QueueFamilies*           queue_families;
        DescriptorLayoutCreator* descriptor_creator;
        Swapchain*               swapchain;
        WindowSurface*           window_surface;
    };

    struct ImageInfo
    {
        VkFormat   depth_format;
        VkFormat   color_format;
        VkExtent2D image_size;
    };

} // namespace pvp
