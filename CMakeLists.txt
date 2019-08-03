project(VULKAN_EXPERIMENTS)

cmake_minimum_required(VERSION 3.7.2)

set (CMAKE_CXX_STANDARD 11)

set(ENV{VK_INSTANCE_LAYERS} "VK_LAYER_LUNARG_standard_validation;VK_LAYER_LUNARG_swapchain;VK_LAYER_LUNARG_api_dump")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(hello_vulkan src/hello_vulkan.cpp src/vulkan_functions.cpp src/vulkan_library.cpp)
target_include_directories(hello_vulkan PUBLIC "C:/VulkanSDK/1.1.82.1/Include")
target_link_libraries(hello_vulkan "C:/VulkanSDK/1.1.82.1/Lib/vulkan-1.lib")