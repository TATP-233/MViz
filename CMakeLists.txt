cmake_minimum_required(VERSION 3.14)
project(mviz VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用FetchContent模块
include(FetchContent)

# 包含第三方库配置
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/third_party/glfw.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/third_party/glad.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/third_party/glm.cmake)

# 递归查找所有源文件
file(GLOB_RECURSE PROJECT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# 添加可执行文件
add_executable(mviz ${PROJECT_SOURCES})

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
