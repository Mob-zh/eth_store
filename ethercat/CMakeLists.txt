
cmake_minimum_required(VERSION 3.9)

#设置变量CMAKE_MODULE_PATH ，将当前目录下的cmake/Modules加入模块搜索路径
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")
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
#设置版本信息
project(SOEM
    DESCRIPTION "Simple Open EtherCAT Master"
    VERSION 1.4.0
    LANGUAGES C)

#设置默认路径
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  # Default to installing in SOEM source directory
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/install)
endif()

#设置安装头文件的目录和库文件安装目录
set(SOEM_INCLUDE_INSTALL_DIR include/soem)
set(SOEM_LIB_INSTALL_DIR lib)

#检查项目是否作为主项目构建
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(BUILD_TESTS TRUE)
else()
  message(STATUS "SOEM: not building tests when built as dependency")
  set(BUILD_TESTS FALSE)
endif()

if(WIN32)
  set(OS "win32")

elseif(UNIX AND NOT APPLE)
  set(OS "linux")
  #设置编译选项
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror") 
  #指定要链接的库
  set(OS_LIBS pthread rt)


elseif(APPLE)
  # This must come *before* linux or MacOSX will identify as Unix.
  set(OS "macosx")
  # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
  # set(OS_LIBS pthread pcap)

elseif(${CMAKE_SYSTEM_NAME} MATCHES "rt-kernel")
  set(OS "rtk")
  message(STATUS "ARCH is ${ARCH}")
  message(STATUS "BSP is ${BSP}")
  include_directories(oshw/${OS}/${ARCH})
  file(GLOB OSHW_EXTRA_SOURCES oshw/${OS}/${ARCH}/*.c)
  set(OSHW_SOURCES "${OS_HW_SOURCES} ${OSHW_ARCHSOURCES}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-but-set-variable")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format")
  set(OS_LIBS "-Wl,--start-group -l${BSP} -l${ARCH} -lkern -ldev -lsio -lblock -lfs -lusb -llwip -leth -li2c -lrtc -lcan -lnand -lspi -lnor -lpwm -ladc -ltrace -lc -lm -Wl,--end-group")

  elseif(${CMAKE_SYSTEM_NAME} MATCHES "rtems")
  message(STATUS "Building for RTEMS")
  set(OS "rtems")
  set(SOEM_LIB_INSTALL_DIR ${LIB_DIR})
  set(BUILD_TESTS FALSE)
endif()

message(STATUS "OS is ${OS}")

#通过file 收集soem，osal，oshw下的源文件和头文件
file(GLOB SOEM_SOURCES soem/*.c)
file(GLOB OSAL_SOURCES osal/${OS}/*.c)
file(GLOB OSHW_SOURCES oshw/${OS}/*.c)

file(GLOB SOEM_HEADERS soem/*.h)
file(GLOB OSAL_HEADERS osal/osal.h osal/${OS}/*.h)
file(GLOB OSHW_HEADERS oshw/${OS}/*.h)

#将上面的文件封装成库
add_library(soem STATIC
  ${SOEM_SOURCES}
  ${OSAL_SOURCES}
  ${OSHW_SOURCES}
  ${OSHW_EXTRA_SOURCES})
  
  #上面的库依赖pthread和rt
target_link_libraries(soem ${OS_LIBS})


#设置头文件路径
target_include_directories(soem PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/soem>
  $<INSTALL_INTERFACE:include/soem>)


target_include_directories(soem PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/osal>
  $<INSTALL_INTERFACE:include/soem>)

target_include_directories(soem PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/osal/${OS}>
  $<INSTALL_INTERFACE:include/soem>)

target_include_directories(soem
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/oshw/${OS}>
  $<INSTALL_INTERFACE:include/soem>
  )

message(STATUS "LIB_DIR: ${SOEM_LIB_INSTALL_DIR}")

#安装设置
install(TARGETS soem EXPORT soemConfig DESTINATION ${SOEM_LIB_INSTALL_DIR})

install(EXPORT soemConfig DESTINATION share/soem/cmake)


install(FILES
  ${SOEM_HEADERS}
  ${OSAL_HEADERS}
  ${OSHW_HEADERS}
  DESTINATION ${SOEM_INCLUDE_INSTALL_DIR})

if(BUILD_TESTS) 
  add_subdirectory(test/simple_ng)
  add_subdirectory(test/linux/slaveinfo)
  add_subdirectory(test/linux/eepromtool)
  add_subdirectory(test/linux/simple_test)
  add_subdirectory(test/linux/switch_ctrl)
endif()
