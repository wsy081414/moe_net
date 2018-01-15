
add_compile_options(-std=c++11 -lpthread -g)

aux_source_directory(. BASE_SRC)

add_library(base_lib ${BASE_SRC})

INCLUDE_DIRECTORIES(../../../ ../../ ./)
target_link_libraries(base_lib net_lib pthread)
