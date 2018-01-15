
add_compile_options(-std=c++11 -lpthread -g)

aux_source_directory(. NET_SRC)


add_library(net_lib ${NET_SRC})
target_link_libraries(net_lib base_lib)


INCLUDE_DIRECTORIES(../../../ ../../ ./)
