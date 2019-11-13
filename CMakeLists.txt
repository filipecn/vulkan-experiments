project(VULKAN_EXPERIMENTS)
cmake_minimum_required(VERSION 3.7.2)

set (CMAKE_CXX_STANDARD 17)

set(ENV{VK_INSTANCE_LAYERS} 
    "VK_LAYER_LUNARG_standard_validation;VK_LAYER_LUNARG_swapchain;VK_LAYER_LUNARG_api_dump")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# external
include(ExternalProject)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext")
include(glfw)

if(APPLE)
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
elseif(MSVC)
    set(VULKAN_LIBRARIES "C:/VulkanSDK/1.1.82.1/Lib/vulkan-1.lib")
    set(VULKAN_INCLUDES "C:/VulkanSDK/1.1.82.1/Include")
	set(GLFW_INCLUDES ${GLFW_INCLUDE_DIR})
else()
    set(GLFW_LIBRARIES glfw)
    set(VULKAN_LIBRARIES vulkan dl)
    set(VULKAN_INCLUDES )
endif()

# find_package(vulkan REQUIRED)

set(SOURCES
    src/vk_app.cpp
    src/vk_buffer.cpp
	src/vk_command_buffer.cpp
    src/vk_device_memory.cpp
	src/vk_fence.cpp
    src/vk_graphics_display.cpp
    src/vk_image.cpp
    src/vk_pipeline.cpp
	src/vk_renderpass.cpp
	src/vk_shader_module.cpp
	src/vk_swap_chain.cpp
	src/vulkan_instance.cpp
    src/vulkan_library.cpp
	src/vulkan_logical_device.cpp
	src/vulkan_physical_device.cpp
    )
set(HEADERS 
	src/logging.h
    src/vk_app.h
    src/vk_buffer.h
	src/vk_command_buffer.h
    src/vk_device_memory.h
	src/vk_fence.h
    src/vk_graphics_display.h
    src/vk_image.h
    src/vk_pipeline.h
	src/vk_renderpass.h
	src/vk_shader_module.h
	src/vk_swap_chain.h
	src/vulkan_instance.h
    src/vulkan_library.h
	src/vulkan_logical_device.h
	src/vulkan_physical_device.h
)

add_library(vk STATIC ${SOURCES} ${HEADERS})
set_target_properties(vk PROPERTIES
        OUTPUT_NAME "vk"
        FOLDER "VK")
target_include_directories(vk PUBLIC ${GLOG_INCLUDE_DIR} ${GLFW_INCLUDES} ${VULKAN_INCLUDES})
target_link_libraries(vk
# Vulkan::Vulkan
 ${GLFW_LIBRARIES} 
 ${VULKAN_LIBRARIES}
 )


if(APPLE)
add_custom_command(TARGET hello_vulkan POST_BUILD        
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        "/Users/filipecn/Documents/vulkansdk-macos-1.1.114.0/macOS/lib/libvulkan.1.1.114.dylib"
        $<TARGET_FILE_DIR:hello_vulkan>)                 
endif()

add_subdirectory(examples)
