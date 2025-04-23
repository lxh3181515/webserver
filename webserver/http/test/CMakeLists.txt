cmake_minimum_required(VERSION 3.10)   # 指定最低 CMake 版本

project(WebServer VERSION 1.0)          # 定义项目名称和版本

# 设置 C++ 标准为 C++11
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("${PROJECT_SOURCE_DIR}/..", "${PROJECT_SOURCE_DIR}/../../threadpool")
file(GLOB SRCS "../*.cc")

# 添加源文件
add_executable(test test.cpp ${SRCS})