project(VULKAN_EXPERIMENTS)
cmake_minimum_required(VERSION 3.7.2)

set (CMAKE_CXX_STANDARD 17)

set(ENV{VK_INSTANCE_LAYERS} 
    "VK_LAYER_LUNARG_standard_validation;VK_LAYER_LUNARG_swapchain;VK_LAYER_LUNARG_api_dump")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
elseif(WIN32)
    set(VULKAN_LIBRARIES "C:/VulkanSDK/1.1.82.1/Lib/vulkan-1.lib")
    set(VULKAN_INCLUDES "C:/VulkanSDK/1.1.82.1/Include")
endif()

# find_package(vulkan REQUIRED)

set(SOURCES
    src/hello_vulkan.cpp 
    src/vk_app.cpp
    src/vk_graphics_display.cpp
    # src/vulkan_api.cpp 
    src/vulkan_library.cpp
    )
    set(HEADERS 
    src/vk_app.h
    src/vk_graphics_display.h
    # src/vulkan_api.h
    # src/vulkan_api.inl
    src/vulkan_library.h
)

add_executable(hello_vulkan ${SOURCES} ${HEADERS})
target_include_directories(hello_vulkan PUBLIC ${GLFW_INCLUDES} ${VULKAN_INCLUDES})
target_link_libraries(hello_vulkan 
# Vulkan::Vulkan
 ${GLFW_LIBRARIES} 
 ${VULKAN_LIBRARIES}
 )

add_custom_command(TARGET hello_vulkan POST_BUILD        
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        "/Users/filipecn/Documents/vulkansdk-macos-1.1.114.0/macOS/lib/libvulkan.1.1.114.dylib"
        $<TARGET_FILE_DIR:hello_vulkan>)                 
