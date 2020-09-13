# extracted from https://github.com/SaschaWillems/Vulkan/blob/master/examples/CMakeLists.txt
function(buildExample EXAMPLE_NAME)
    set(EXAMPLE_BASE_SOURCE example_base.h example_base.cpp)
    set(EXAMPLE_BASE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})
    set(EXAMPLE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME})
    message(STATUS "Generating project file for example in ${EXAMPLE_FOLDER}")
    # Main
    file(GLOB SOURCE *.cpp ${BASE_HEADERS} ${EXAMPLE_FOLDER}/*.cpp)
    SET(MAIN_CPP ${EXAMPLE_FOLDER}/${EXAMPLE_NAME}.cpp)
    if (EXISTS ${EXAMPLE_FOLDER}/main.cpp)
        SET(MAIN_CPP ${EXAMPLE_FOLDER}/main.cpp)
    ENDIF ()
    # Add shaders
    set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME}/shaders")
    file(GLOB SHADERS
            "${SHADER_DIR}/*.vert"
            "${SHADER_DIR}/*.frag"
            "${SHADER_DIR}/*.comp"
            "${SHADER_DIR}/*.geom"
            "${SHADER_DIR}/*.tesc"
            "${SHADER_DIR}/*.tese"
            "${SHADER_DIR}/*.mesh"
            "${SHADER_DIR}/*.task"
            "${SHADER_DIR}/*.rgen"
            "${SHADER_DIR}/*.rchit"
            "${SHADER_DIR}/*.rmiss"
            )
    add_executable(${EXAMPLE_NAME} ${MAIN_CPP} ${SOURCE} ${SHADERS} ${EXAMPLE_BASE_SOURCE})
    add_dependencies(${EXAMPLE_NAME} glfw ponos)
    target_link_libraries(${EXAMPLE_NAME}
            vk
            ${VULKAN_LIBRARIES}
#            ${GLFW_LIBRARIES}
            )
    target_include_directories(${EXAMPLE_NAME} PUBLIC
            ${STB_INCLUDES}
            ${GLFW_INCLUDES}
            ${VULKAN_INCLUDES}
            ${PONOS_INCLUDE_DIR}
            ${VK_INCLUDES}
            ${EXAMPLE_BASE_INCLUDES}
            )
    target_compile_definitions(${EXAMPLE_NAME} PUBLIC
            -DSHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders"
            -DMODELS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/models"
            -DTEXTURES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/textures"
            )
endfunction(buildExample)

set(EXAMPLES
        hello_vulkan
        )

foreach (EXAMPLE ${EXAMPLES})
    buildExample(${EXAMPLE})
endforeach (EXAMPLE)


#add_executable(hello_vulkan hello_vulkan.cpp)
#target_compile_definitions(hello_vulkan PUBLIC
#        -DSHADERS_PATH="${VULKAN_EXPERIMENTS_SOURCE_DIR}/examples"
#        -DMODELS_PATH="${VULKAN_EXPERIMENTS_SOURCE_DIR}/examples/models"
#        -DTEXTURES_PATH="${VULKAN_EXPERIMENTS_SOURCE_DIR}/examples/textures")
#target_include_directories(hello_vulkan PUBLIC  
#        ${STB_INCLUDES} 
#        ${GLFW_INCLUDES} 
#        ${VULKAN_INCLUDES} 
#        ${PONOS_INCLUDE_DIR}
#        ${VK_INCLUDES}
#)
#add_dependencies(hello_vulkan
#        glfw
#        ponos
#        )
#target_link_libraries(hello_vulkan vk
#        # Vulkan::Vulkan
#        # ${GLFW_LIBRARIES}
#        ${VULKAN_LIBRARIES}
#        )
#
#set(IMGUI_SOURCE
#        imgui/imgui.h
#        imgui/imgui.cpp
#        imgui/imgui_draw.cpp
#        imgui/imgui_widgets.cpp
#        imgui/imgui_demo.cpp
#        vk_imgui.h
#        vk_imgui.cpp
#)
#set(IMGUI_INCLUDE_DIR imgui)
#
#add_executable(mesh_viewer mesh_viewer_app.cpp ${IMGUI_SOURCE})
#target_compile_definitions(mesh_viewer PUBLIC
#        -DSHADERS_PATH="${VULKAN_EXPERIMENTS_SOURCE_DIR}/examples"
#        -DMODELS_PATH="${VULKAN_EXPERIMENTS_SOURCE_DIR}/examples/models"
#        -DTEXTURES_PATH="${VULKAN_EXPERIMENTS_SOURCE_DIR}/examples/textures")
#target_include_directories(mesh_viewer PUBLIC  
#        ${IMGUI_INCLUDE_DIR}
#        ${STB_INCLUDES} 
#        ${GLFW_INCLUDES} 
#        ${VULKAN_INCLUDES} 
#        ${PONOS_INCLUDE_DIR}
#        ${VK_INCLUDES}
#)
#add_dependencies(mesh_viewer
#        glfw
#        ponos
#        )
#target_link_libraries(mesh_viewer vk
#        # Vulkan::Vulkan
#        # ${GLFW_LIBRARIES}
#        ${VULKAN_LIBRARIES}
#        )

