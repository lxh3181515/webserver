cmake_minimum_required(VERSION 3.10)   # 指定最低 CMake 版本

project(WebServer VERSION 1.0)          # 定义项目名称和版本

# 设置 C++ 标准为 C++11
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    "${PROJECT_SOURCE_DIR}/webserver"
    "${PROJECT_SOURCE_DIR}/webserver/epoll"
    "${PROJECT_SOURCE_DIR}/webserver/http"
    "${PROJECT_SOURCE_DIR}/webserver/socket"
    "${PROJECT_SOURCE_DIR}/webserver/threadpool")
file(GLOB SRCS 
    "${PROJECT_SOURCE_DIR}/webserver/*.cc" 
    "${PROJECT_SOURCE_DIR}/webserver/epoll/*.cc"
    "${PROJECT_SOURCE_DIR}/webserver/http/*.cc"
    "${PROJECT_SOURCE_DIR}/webserver/socket/*.cc"
    "${PROJECT_SOURCE_DIR}/webserver/threadpool/*.cc")

# 添加源文件
add_executable(WebServer main.cpp ${SRCS})