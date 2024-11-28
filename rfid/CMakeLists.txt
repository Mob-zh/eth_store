# 公共源文件
set(COMMON_SRC_FILES
    src/rfid_M4255.c
    src/uart.c
)

# RFID GetElem 源文件
set(RFID_GETELEM_SRC_FILES
    ${COMMON_SRC_FILES}
    app/rfid_GetElem.c
    ../sqlite/src/rfid_db.c  
)

# RFID 测试文件
set(RFID_TEST_SRC_FILES
    ${COMMON_SRC_FILES}
    test/rfid_test.c
)

# UART 测试文件
set(UART_TEST_SRC_FILES
    ${COMMON_SRC_FILES}
    test/uart_test.c
)

# 外部库路径
set(SQLITE_LIB_PATH /opt/sqlite-arm/lib/libsqlite3.a)

# 包含目录
include_directories(src /opt/sqlite-arm/include ../sqlite/src)

# 创建 rfid_GetElem 目标
add_executable(rfid_GetElem ${RFID_GETELEM_SRC_FILES})
target_link_libraries(rfid_GetElem ${SQLITE_LIB_PATH} m)
set_target_properties(rfid_GetElem PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/rfid/app
)

# 创建 rfid_test 目标
add_executable(rfid_test ${RFID_TEST_SRC_FILES})
target_link_libraries(rfid_test m)
set_target_properties(rfid_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/rfid/test
)

# 创建 uart_test 目标
add_executable(uart_test ${UART_TEST_SRC_FILES})
target_link_libraries(uart_test m)
set_target_properties(uart_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/rfid/test
)
