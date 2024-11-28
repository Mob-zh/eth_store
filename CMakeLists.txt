# 设置最低版本要求和项目名
cmake_minimum_required(VERSION 3.10)
project(EthStore)

##################################
# 配置 ARM 交叉编译
#################################
set(CMAKE_SYSTEM_NAME Linux) #设置目标系统名字
set(CMAKE_SYSTEM_PROCESSOR arm) #设置目标处理器架构
# 指定编译器的 sysroot 路径
set(TOOLCHAIN_DIR /opt/atk-dlmp135-toolchain)
set(CMAKE_SYSROOT ${TOOLCHAIN_DIR}/arm-buildroot-linux-gnueabihf/sysroot)
# 指定交叉编译器 arm-linux-gcc 和 arm-linux-g++
set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/arm-buildroot-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-buildroot-linux-gnueabihf-g++)
# 为编译器添加编译选项
set(CMAKE_C_FLAGS "-mthumb -mfpu=neon-vfpv4 -mfloat-abi=hard -mcpu=cortex-a7")
set(CMAKE_CXX_FLAGS "-mthumb -mfpu=neon-vfpv4 -mfloat-abi=hard -mcpu=cortex-a7")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#################################
# end
##################################

# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

# 添加子目录
add_subdirectory(sqlite)
add_subdirectory(rfid)
add_subdirectory(ethercat)

