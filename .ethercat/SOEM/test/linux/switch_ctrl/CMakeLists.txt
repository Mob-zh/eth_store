
set(SOURCES switch_ctrl.c)
add_executable(switch_ctrl ${SOURCES})
target_link_libraries(switch_ctrl soem)
install(TARGETS switch_ctrl DESTINATION bin)
