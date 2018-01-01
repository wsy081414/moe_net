
add_compile_options(-std=c++11 -lpthread)

aux_source_directory(. BASE_SRC)

add_library(base_lib ${BASE_SRC})

INCLUDE_DIRECTORIES(../../../ ../../ ./)
