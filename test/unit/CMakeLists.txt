add_executable(unit_test_mt test_result.cc test_selector.cc test_task.cc counted.h)

target_link_libraries(unit_test_mt Catch2WithMain mt)
