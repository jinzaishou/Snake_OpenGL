# 设置 CMake 最低版本
cmake_minimum_required(VERSION 3.10)

# 设置项目名和语言
project(OpenGLSnake)

# 设置使用的 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========== 添加 GLFW ==========
# 告诉 CMake 去 external/glfw 子目录里找 CMakeLists.txt 构建 GLFW 库
add_subdirectory(external/glfw)

# ========== 包含头文件目录 ==========
# 告诉编译器：以后在这些目录里找 .h 头文件
include_directories(
    external/glfw/include        # GLFW 的头文件
    external/glad/include        # GLAD 的头文件
)

# ========== 添加可执行文件 ==========
# 声明你项目的源文件有哪些，会被编译为 OpenGLSnake 可执行程序
add_executable(OpenGLSnake
    CMake贪吃蛇.cpp                 # 你的主程序
    external/glad/src/glad.c     # GLAD 的实现文件
)

# ========== 链接需要的库 ==========
# 告诉编译器：这个项目需要用哪些库（顺序有时很重要）
target_link_libraries(OpenGLSnake
    glfw                        # 链接 GLFW（刚刚 add_subdirectory 添加的）
    opengl32                    # Windows 下的 OpenGL 系统库
)