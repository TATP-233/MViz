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