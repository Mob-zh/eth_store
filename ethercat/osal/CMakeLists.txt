file(GLOB OSAL_SOURCES *.c)
add_library(osal SHARED ${OSAL_SOURCES})
target_include_directories(osal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})