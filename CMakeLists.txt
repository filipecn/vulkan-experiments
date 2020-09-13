project(VULKAN_EXPERIMENTS)
cmake_minimum_required(VERSION 3.7.2)

if (MSVC)
    list(APPEND args --target INSTALL)
else (MSVC)
    list(APPEND args --target install)
endif (MSVC)


set(CMAKE_CXX_STANDARD 17)

set(ENV{VK_INSTANCE_LAYERS}
        "VK_LAYER_LUNARG_standard_validation;VK_LAYER_LUNARG_swapchain;VK_LAYER_LUNARG_api_dump")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# external
set(STB_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/ext")
include(ExternalProject)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext")
include(glfw)
include(ponos)


if (APPLE)
    set(VULKAN_SDK "/Users/filipecn/Documents/vulkansdk-macos-1.1.114.0")
    set(ENV{VK_ICD_FILENAMES} "${VULKAN_SDK}/macOS/etc/vulkan/icd.d/MoltenVK_icd.json")
    set(ENV{VK_LAYER_PATH} "${VULKAN_SDK}/macOS/etc/vulkan/explicit_layer.d")
    set(ENV{DYLD_LIBRARY_PATH} "${VULKAN_SDK}/macOS/lib/libvulkan.1.dylib")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
    LINK_DIRECTORIES(/usr/local/lib)
    set(VULKAN_LIBRARIES
            "/Users/filipecn/Documents/vulkansdk-macos-1.1.114.0/macOS/lib/libvulkan.1.dylib"
            "/Users/filipecn/Documents/vulkansdk-macos-1.1.114.0/macOS/lib/libvulkan.1.1.114.dylib"
            )
    set(VULKAN_INCLUDES "/Users/filipecn/Documents/vulkansdk-macos-1.1.114.0/macOS/include")
    set(GLFW_INCLUDES "/usr/local/include")
    set(GLFW_LIBRARIES glfw)
elseif (MSVC)
    set(DEBUG_FLAGS "/FS /Zi /EHsc /MD /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING ")
    set(RELEASE_FLAGS "/FS /EHsc")
    set(VULKAN_LIBRARIES "C:/VulkanSDK/1.2.131.2/Lib/vulkan-1.lib")
    set(VULKAN_INCLUDES "C:/VulkanSDK/1.2.131.2/Include")
    set(GLFW_INCLUDES ${GLFW_INCLUDE_DIR})
else ()
    set(GLFW_LIBRARIES glfw)
    set(VULKAN_LIBRARIES vulkan dl)
    set(VULKAN_INCLUDES /usr/local/include/vulkan)
endif ()
set(CMAKE_CXX_FLAGS ${RELEASE_FLAGS})
set(CMAKE_CXX_FLAGS_DEBUG ${DEBUG_FLAGS})
set(CMAKE_CONFIGURATION_TYPES Debug Release)
# find_package(vulkan REQUIRED)

set(SOURCES
        src/core/vk_app.cpp
        src/core/vk_buffer.cpp
        src/core/vk_mesh_buffer_data.cpp
        src/core/vk_command_buffer.cpp
        src/core/vk_device_memory.cpp
        src/core/vk_sync.cpp
        src/core/vk_graphics_display.cpp
        src/core/vk_image.cpp
        src/core/vk_pipeline.cpp
        src/core/vk_render_engine.cpp
        src/core/vk_renderpass.cpp
        src/core/vk_sampler.cpp
        src/core/vk_shader_module.cpp
        src/core/vk_swap_chain.cpp
        src/core/vk_texture_image.cpp
        src/core/vulkan_instance.cpp
        src/core/vulkan_library.cpp
        src/core/vulkan_logical_device.cpp
        src/core/vulkan_physical_device.cpp
        src/scene/camera.cpp
        src/scene/model.cpp
        )
set(HEADERS
        src/core/logging.h
        src/core/vk.h
        src/core/vk_app.h
        src/core/vk_buffer.h
        src/core/vk_mesh_buffer_data.h
        src/core/vk_command_buffer.h
        src/core/vk_device_memory.h
        src/core/vk_graphics_display.h
        src/core/vk_image.h
        src/core/vk_pipeline.h
        src/core/vk_render_engine.h
        src/core/vk_renderpass.h
        src/core/vk_sampler.h
        src/core/vk_shader_module.h
        src/core/vk_swap_chain.h
        src/core/vk_sync.h
        src/core/vk_texture_image.h
        src/core/vulkan_instance.h
        src/core/vulkan_library.h
        src/core/vulkan_logical_device.h
        src/core/vulkan_physical_device.h
        src/scene/camera.h
        src/scene/model.h
        )

set(VK_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/src")
add_library(vk STATIC ${SOURCES} ${HEADERS})
set_target_properties(vk PROPERTIES
        OUTPUT_NAME "vk"
        FOLDER "VK")
target_include_directories(vk PUBLIC
        ${STB_INCLUDES}
        ${GLFW_INCLUDES}
        ${VULKAN_INCLUDES}
        ${PONOS_INCLUDES}
        ${VK_INCLUDES}
        )
target_link_libraries(vk
        # Vulkan::Vulkan
        # ${GLFW_LIBRARIES}
        ${PONOS_LIBRARIES}
        ${VULKAN_LIBRARIES}
        )


if (APPLE)
    add_custom_command(TARGET hello_vulkan POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "/Users/filipecn/Documents/vulkansdk-macos-1.1.114.0/macOS/lib/libvulkan.1.1.114.dylib"
            $<TARGET_FILE_DIR:hello_vulkan>)
endif ()


add_subdirectory(examples)
