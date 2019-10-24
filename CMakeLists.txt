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
    src/hello_vulkan.cpp 
    src/vk_app.cpp
    src/vk_buffer.cpp
    src/vk_graphics_display.cpp
    src/vk_image.cpp
    # src/vulkan_api.cpp 
	src/vulkan_instance.cpp
    src/vulkan_library.cpp
	src/vulkan_logical_device.cpp
	src/vulkan_physical_device.cpp
    )
    set(HEADERS 
	src/logging.h
    src/vk_app.h
    src/vk_buffer.h
    src/vk_graphics_display.h
    src/vk_image.h
    # src/vulkan_api.h
    # src/vulkan_api.inl
	src/vulkan_instance.h
    src/vulkan_library.h
	src/vulkan_logical_device.h
	src/vulkan_physical_device.h
)
add_executable(hello_vulkan ${SOURCES} ${HEADERS})
target_include_directories(hello_vulkan PUBLIC ${GLOG_INCLUDE_DIR} ${GLFW_INCLUDES} ${VULKAN_INCLUDES})
add_dependencies(hello_vulkan 
glfw 
)
target_link_libraries(hello_vulkan 
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
