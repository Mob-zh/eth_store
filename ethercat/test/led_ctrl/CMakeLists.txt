
set(SOURCES led_ctrl.c)
add_executable(led_ctrl ${SOURCES})
target_link_libraries(led_ctrl soem)
install(TARGETS led_ctrl DESTINATION bin)
