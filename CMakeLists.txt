cmake_minimum_required(VERSION 3.10)   # 指定最低 CMake 版本

project(MyProject VERSION 1.0)          # 定义项目名称和版本

# 设置 C++ 标准为 C++11
set(CMAKE_BUILD_TYPE release)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

aux_source_directory(${PROJECT_SOURCE_DIR}/webserver SRC_LIST)

# 添加头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/webserver)

# 添加源文件
add_executable(simple_client simple_client.cpp)
add_executable(simple_server simple_server.cpp ${SRC_LIST})
