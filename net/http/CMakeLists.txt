
add_compile_options(-std=c++11 -g)

aux_source_directory(. HTTP_SRC)


add_library(http_lib ${HTTP_SRC})
target_link_libraries(http_lib)


INCLUDE_DIRECTORIES(../../../ ../../ ./)
