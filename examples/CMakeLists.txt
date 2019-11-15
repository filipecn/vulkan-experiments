add_executable(hello_vulkan hello_vulkan.cpp)
target_compile_definitions(hello_vulkan PUBLIC -DSHADERS_PATH="${VULKAN_EXPERIMENTS_SOURCE_DIR}/examples")
target_include_directories(hello_vulkan PUBLIC ${GLOG_INCLUDE_DIR} ${GLFW_INCLUDES} ${VULKAN_INCLUDES})
add_dependencies(hello_vulkan 
glfw 
)
target_link_libraries(hello_vulkan vk
# Vulkan::Vulkan
 ${GLFW_LIBRARIES} 
 ${VULKAN_LIBRARIES}
 )


include_directories("${VULKAN_EXPERIMENTS_SOURCE_DIR}/src")

# set(WINDOWS_BINARIES main)

#if (MSVC)
    # Tell MSVC to use main instead of WinMain for Windows subsystem executables
#    set_target_properties(${WINDOWS_BINARIES} PROPERTIES
#                          LINK_FLAGS "/ENTRY:mainCRTStartup")
#endif()
