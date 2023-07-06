add_executable(hello_world hello_world.cc)
add_executable(echo_client echo_client.cc)
add_executable(echo_server echo_server.cc)
add_executable(functional_test functional_test.cc)

target_link_libraries(hello_world mt)
target_link_libraries(echo_client mt)
target_link_libraries(echo_server mt)
target_link_libraries(functional_test mt)
