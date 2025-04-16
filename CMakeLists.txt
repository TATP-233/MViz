cmake_minimum_required(VERSION 3.14)
project(mviz VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用FetchContent模块
include(FetchContent)

# 下载并配置GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# 下载并配置GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG master
)
set(GLAD_PROFILE "core" CACHE STRING "OpenGL profile" FORCE)
set(GLAD_API "gl=3.3" CACHE STRING "API type/version pairs" FORCE)
set(GLAD_GENERATOR "c" CACHE STRING "Language" FORCE)
FetchContent_MakeAvailable(glad)

# 下载并配置GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# 添加可执行文件
add_executable(mviz 
    src/main.cpp
    src/core/Application.cpp
    src/core/Camera.cpp
    src/core/TFManager.cpp
    src/rendering/Shader.cpp
    src/rendering/Renderer.cpp
)

# 链接库
target_link_libraries(mviz PRIVATE 
    glfw
    glad
    glm
)

# 包含头文件目录
target_include_directories(mviz PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 复制着色器文件到输出目录
add_custom_command(TARGET mviz POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:mviz>/shaders
    COMMENT "Copying shader files to output directory"
)
