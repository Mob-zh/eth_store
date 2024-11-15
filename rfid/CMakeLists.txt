# 定义源文件
set(RFID_GETELEM_SRC_FILES
    src/rfid_M4255.c
    src/uart.c
    app/rfid_GetElem.c
    ../sqlite/src/rfid_db.c  
)

set(UART_TEST_SRC_FILES
    src/rfid_M4255.c
    src/uart.c
    test/uart_test.c
)

set(RFID_TEST_SRC_FILES
    src/rfid_M4255.c
    src/uart.c
    test/rfid_test.c
)

# 包含目录
include_directories(
    src
    /opt/sqlite-arm/include
    ../sqlite/src
    
)

# 设置目标 rfid_GetElem
add_executable(rfid_GetElem ${RFID_GETELEM_SRC_FILES})

# 链接外部库 (sqlite3)
target_link_libraries(rfid_GetElem
    /opt/sqlite-arm/lib/libsqlite3.a
    m #链接数学库
)

# 设置输出目录
set_target_properties(rfid_GetElem PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/rfid/app
)
################################################################
# 创建 rfid_test 可执行文件
add_executable(rfid_test ${RFID_TEST_SRC_FILES})
target_link_libraries(rfid_test
    m  
)
# 设置 rfid_test 输出目录为 test
set_target_properties(rfid_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/rfid/test
)
#################################################################
# 创建 uart_test 可执行文件
add_executable(uart_test  ${UART_TEST_SRC_FILES})
target_link_libraries(uart_test
    m  # 链接数学库
)
# 设置 uart_test 输出目录为 test
set_target_properties(uart_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/rfid/test
)
