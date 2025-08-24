cmake_minimum_required(VERSION 3.29)
project(DSTerminal)

include_directories(include)

find_package(freetype REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FREETYPE REQUIRED freetype2)

include(FetchContent)
FetchContent_Declare(
        cglm
        GIT_REPOSITORY https://github.com/recp/cglm.git
        GIT_TAG        v0.9.2 # or latest stable tag
)
FetchContent_MakeAvailable(cglm)

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
        utils/Shader.h
        terminal/Terminal.cpp
        terminal/Terminal.h
        utils/KeyState.h
        include/json.h
        utils/FileUtils.h
        locks/CylinderLock.cpp
        locks/CylinderLock.h
        utils/GameLayer.h
        include/STB_IMAGE/stb_image.cpp
        include/STB_IMAGE/stb_image.h
        include/STB_IMAGE/stb_image_write.cpp
        imp/Imp.cpp
        imp/Imp.h
        utils/TextureLoader.cpp
        utils/TextUtil.c
        utils/TextUtil.h
        utils/SoundManager.cpp
        utils/SoundManager.h
        include/audio/miniaudio.h
        ui/UI.cpp
        ui/UI.h
        ui/MenuScreen.cpp
        ui/MenuScreen.h
        ui/TitleScreen.cpp
        ui/TitleScreen.h
        include/cglm/cglm.h
)

target_link_libraries(DSTerminal PRIVATE
        GLFW
        GLAD
        ${FREETYPE_LIBRARIES}
)

target_include_directories(DSTerminal PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(DSTerminal PRIVATE cglm)

if(APPLE)
    target_link_libraries(DSTerminal PRIVATE
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
            "-framework CoreFoundation"
    )
endif()