# 定义源文件
set(SQLITE_SRC_FILES
    src/rfid_db.c
)

# 包含目录
include_directories(
    src
    /opt/sqlite-arm/include
    ../rfid/src
)

# 编译 SQLite 源文件
add_library(rfid_db STATIC ${SQLITE_SRC_FILES})

# 设置链接库的路径
target_link_libraries(rfid_db
    /opt/sqlite-arm/lib/libsqlite3.a
)
