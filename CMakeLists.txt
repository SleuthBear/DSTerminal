cmake_minimum_required(VERSION 3.29)
project(DSTerminal)

include_directories(include)

find_package(freetype REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FREETYPE REQUIRED freetype2)

# glfw
add_library(GLFW STATIC IMPORTED)
set_target_properties(GLFW PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/include/bin/libglfw3.a"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include/glfw")

# GLAD
add_library(GLAD STATIC "${CMAKE_CURRENT_SOURCE_DIR}/include/glad/glad.c")
target_include_directories(GLAD PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/glad")

set(CMAKE_CXX_STANDARD 20)

add_executable(DSTerminal main.cpp
        Shader.h
        Terminal.cpp
        Terminal.h
        KeyState.h
        include/json.h
        utils/FileUtils.h
        CylinderLock.cpp
        CylinderLock.h
        GameLayer.h
        include/STB_IMAGE/stb_image.cpp
        include/STB_IMAGE/stb_image.h
        include/STB_IMAGE/stb_image_write.cpp
        Imp.cpp
        Imp.h
        utils/TextureLoader.cpp
        utils/TextUtil.cpp
        utils/TextUtil.h
)

target_link_libraries(DSTerminal PRIVATE
        GLFW
        GLAD
        ${FREETYPE_LIBRARIES}
)

target_include_directories(DSTerminal PRIVATE ${FREETYPE_INCLUDE_DIRS})

if(APPLE)
    target_link_libraries(DSTerminal PRIVATE
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
            "-framework CoreFoundation"
    )
endif()