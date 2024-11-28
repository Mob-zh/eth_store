file(GLOB OSHW_SOURCES *.c)
add_library(oshw SHARED ${OSHW_SOURCES})
target_include_directories(oshw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}   ${CMAKE_CURRENT_SOURCE_DIR}/../soem  )
target_link_libraries(oshw PUBLIC osal)