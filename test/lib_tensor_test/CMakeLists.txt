cmake_minimum_required (VERSION 2.6)


file(GLOB SRC_FILES *.c)
add_executable(tensor_test ${SRC_FILES})
if (APPLE)
    target_link_libraries(tensor_test 
        -g 
        tensor 
        linalg 
        array 
        stringmanip
        cutest 
        ${macblas}/Accelerate)
else ()
    target_link_libraries(tensor_test 
        -g 
        tensor 
        linalg 
        array 
        stringmanip
        cutest 
        blas 
        lapack 
        -lm )
endif()


