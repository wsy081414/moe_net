
add_compile_options(-std=c++11 -lpthread)

aux_source_directory(. NET_SRC)

add_library(net_lib ${NET_SRC})

INCLUDE_DIRECTORIES(../../../ ../../ ./)
