link_libraries(aergiav)

link_libraries(${GLFW_LIB} ${aergiav_LIBRARIES} ${VULKAN_LIBRARY})

include_directories("${VULKAN_EXPERIMENTS_SOURCE_DIR}/src")

add_executable(main WIN32 main.cpp)

set(WINDOWS_BINARIES main)

if (MSVC)
    # Tell MSVC to use main instead of WinMain for Windows subsystem executables
    set_target_properties(${WINDOWS_BINARIES} PROPERTIES
                          LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()
