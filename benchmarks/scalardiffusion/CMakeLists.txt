cmake_minimum_required (VERSION 2.6)


add_executable(sdiffbench sdiff.c)
target_link_libraries(sdiffbench -g
    clinalg 
    funcs 
    quadrature 
    linalg 
    array 
    stringmanip
    ${blas_lapack}
    -lm)



