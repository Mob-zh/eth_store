# 设置最低版本要求和项目名
cmake_minimum_required(VERSION 3.10)
project(EthStore)

# 设置交叉编译工具链
set(CMAKE_C_COMPILER arm-buildroot-linux-gnueabihf-gcc)

# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

# 添加子目录
add_subdirectory(sqlite)
add_subdirectory(rfid)

