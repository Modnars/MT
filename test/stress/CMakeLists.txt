add_executable(stress_test stress_test.cc)

target_link_libraries(stress_test PRIVATE Catch2WithMain nanobench mt)
