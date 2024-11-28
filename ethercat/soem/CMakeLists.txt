file(GLOB SOEM_SOURCES *.c)
add_library(soem SHARED ${SOEM_SOURCES})
target_include_directories(soem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}  ${CMAKE_CURRENT_SOURCE_DIR}/../oshw  ${CMAKE_CURRENT_SOURCE_DIR}/../osal)

target_link_libraries(soem PUBLIC oshw osal)
