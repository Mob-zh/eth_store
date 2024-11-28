# cmake_minimum_required(VERSION 3.16)
# project(MyProjectName)

# #设置交叉编译工具链
# set(CMAKE_SYSTEM_NAME Linux) #设置目标系统名字
# set(CMAKE_SYSTEM_PROCESSOR arm) #设置目标处理器架构
# # 指定编译器的 sysroot 路径
# set(TOOLCHAIN_DIR /opt/atk-dlmp135-toolchain)
# set(CMAKE_SYSROOT ${TOOLCHAIN_DIR}/arm-buildroot-linux-gnueabihf/sysroot)
# # 指定交叉编译器 arm-linux-gcc 和 arm-linux-g++
# set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/arm-buildroot-linux-gnueabihf-gcc)
# set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-buildroot-linux-gnueabihf-g++)
# # 为编译器添加编译选项
# set(CMAKE_C_FLAGS "-mthumb -mfpu=neon-vfpv4 -mfloat-abi=hard -mcpu=cortex-a7")
# set(CMAKE_CXX_FLAGS "-mthumb -mfpu=neon-vfpv4 -mfloat-abi=hard -mcpu=cortex-a7")
# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


set(OS "linux")
#设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror") 
#指定要链接的库
set(OS_LIBS pthread rt)

# add_subdirectory(osal)
# add_subdirectory(oshw)
# add_subdirectory(soem)

#通过file 收集soem，osal，oshw下的源文件和头文件
file(GLOB SOEM_SOURCES soem/*.c)
file(GLOB OSAL_SOURCES osal/*.c)
file(GLOB OSHW_SOURCES oshw/*.c)

file(GLOB SOEM_HEADERS soem/*.h)
file(GLOB OSAL_HEADERS osal/osal.h osal/*.h)
file(GLOB OSHW_HEADERS oshw/*.h)

#将上面的文件封装成库
add_library(soem SHARED
  ${OSAL_SOURCES}
  ${OSHW_SOURCES}
  ${SOEM_SOURCES}
)
target_include_directories(soem PUBLIC soem  oshw  osal)
  
  #上面的库依赖pthread和rt
target_link_libraries(soem ${OS_LIBS})


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/simple_test)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/led_ctrl)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/simple_ng)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/slaveinfo)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/switch_ctrl)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/eepromtool)

