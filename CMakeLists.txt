add_compile_options(-std=c++11 -lpthread)
INCLUDE_DIRECTORIES(../ ./base)
add_subdirectory(base)
add_executable(a.out test.cc)
target_link_libraries(a.out base_lib pthread)

