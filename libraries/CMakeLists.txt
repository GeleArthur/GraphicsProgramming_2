﻿cmake_minimum_required(VERSION 3.25)
project(pretty-vulkan-printer-libraries)

include(FetchContent)

find_package(Vulkan REQUIRED)
if(NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan SDK not installed.")
endif()

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG release-1.0.2
        GIT_SHALLOW TRUE
)
set(GLM_ENABLE_CXX_20 ON)
FetchContent_MakeAvailable(glm)
target_compile_definitions(glm PUBLIC
        GLM_FORCE_XYZW_ONLY
        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_SILENT_WARNINGS
        GLM_ENABLE_EXPERIMENTAL
        GLM_EXT_INCLUDED
)

set(GLFW_INSTALL OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

#set(VMA_ENABLE_INSTALL ON)
#FetchContent_Declare(
#        vma
#        URL https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v3.2.1.zip
#)
#FetchContent_MakeAvailable(vma)


FetchContent_Declare(
        imgui
        URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.zip
)
FetchContent_MakeAvailable(imgui)

add_library(imgui)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
target_sources(imgui PRIVATE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp

        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

target_link_libraries(imgui PRIVATE
        glfw
        Vulkan::Vulkan
)

FetchContent_Declare(
        spdlog
        URL https://github.com/gabime/spdlog/archive/refs/tags/v1.15.1.zip
)
FetchContent_MakeAvailable(spdlog)

set(ASSIMP_BUILD_TESTS OFF)
FetchContent_Declare(
        assimp
        URL https://github.com/assimp/assimp/archive/refs/tags/v5.4.3.zip
)
FetchContent_MakeAvailable(assimp)

add_library(${PROJECT_NAME} INTERFACE)

# hihihi
add_library(spirv-reflect STATIC
        ${Vulkan_INCLUDE_DIRS}/../Source/SPIRV-Reflect/spirv_reflect.h
        ${Vulkan_INCLUDE_DIRS}/../Source/SPIRV-Reflect/spirv_reflect.c
)
target_include_directories(spirv-reflect PUBLIC ${Vulkan_INCLUDE_DIRS}/../Source/SPIRV-Reflect)
set_target_properties(spirv-reflect PROPERTIES PROPERTY EXCLUDE_FROM_ALL)

add_library(shaderc STATIC IMPORTED)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(shaderc PROPERTIES
            IMPORTED_LOCATION "${Vulkan_INCLUDE_DIRS}/../Lib/shaderc_combinedd.lib"
    )
else()
    set_target_properties(shaderc PROPERTIES
            IMPORTED_LOCATION "${Vulkan_INCLUDE_DIRS}/../Lib/shaderc_combined.lib"
    )
endif()

target_link_libraries(${PROJECT_NAME} INTERFACE
        glm::glm
        glfw
        imgui
        spdlog
        assimp
        Vulkan::Vulkan
        spirv-reflect
        shaderc
)