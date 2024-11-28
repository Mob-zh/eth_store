# 定义源文件
set(SQLITE_SRC_FILES
    src/rfid_db.c
)
set(CREATE_SHELF_SRC_FILES
    app/create_shelf.c
)
set(SELECT_DB_SRC_FILES
    app/select_db.c
)
set(DB_TEST_SRC_FILES
    test/db_test.c
)


# 包含目录
include_directories(
    src
    /opt/sqlite-arm/include
    ../rfid/src
)

# 编译 SQLite 源文件 (自建的库) 
add_library(rfid_db STATIC ${SQLITE_SRC_FILES})

# 设置链接库的路径
target_link_libraries(rfid_db
    /opt/sqlite-arm/lib/libsqlite3.a
)

##################################################
# 设置目标 create_shelf
add_executable(create_shelf ${CREATE_SHELF_SRC_FILES} ${SQLITE_SRC_FILES})

# 链接外部库 (sqlite3)
target_link_libraries(create_shelf
    /opt/sqlite-arm/lib/libsqlite3.a
    m #链接数学库
)
set_target_properties(create_shelf PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sqlite/app
)

##################################################
# 设置目标 select_db
add_executable(select_db ${SELECT_DB_SRC_FILES} ${SQLITE_SRC_FILES})

# 链接外部库 (sqlite3)
target_link_libraries(select_db
    /opt/sqlite-arm/lib/libsqlite3.a
    m #链接数学库
)
set_target_properties(select_db PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sqlite/app
)

##################################################
# 设置目标 db_test
add_executable(db_test ${DB_TEST_SRC_FILES} ${SQLITE_SRC_FILES})

# 链接外部库 (sqlite3)
target_link_libraries(db_test
    /opt/sqlite-arm/lib/libsqlite3.a
    m #链接数学库
)
set_target_properties(db_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sqlite/test
)