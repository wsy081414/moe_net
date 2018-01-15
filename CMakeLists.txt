add_compile_options(-std=c++11 -lpthread -g)
INCLUDE_DIRECTORIES(../ ./base ./net)
add_subdirectory(base)
add_subdirectory(net)
add_executable(a.out test.cc)

add_executable(cli.out cli.cc)
add_executable(serv.out serv.cc)


target_link_libraries(a.out pthread base_lib net_lib )
target_link_libraries(cli.out pthread base_lib net_lib )
target_link_libraries(serv.out pthread base_lib net_lib )


