#!/bin/bash

TESTING=0
CLEAN=0
INSTALL=0
while [[ $# > 1 ]]
do
    key="$1"
    case $key in 
        -t|--test)
        TESTING="$2"
        shift
        ;;
        -c|--clean)
        CLEAN="$2"
        shift
        ;;
        -i|--install)
        INSTALL="$2"
        shift
        ;;
        --default)
        TESTING=0
        shift
        ;;
        *)

esac
shift
done

echo TO TEST = "${TESTING}"
echo TO CLEAN = "${CLEAN}"


BUILD_TYPE="None"
CMD_LINE_ARGS=""

cd build

if [ $CLEAN -eq 1 ]
then
    make clean
fi

rm -f benchmarks/dmrgprod/bin/dmrgprodbench
rm -f benchmarks/fastmkron/bin/fmkronbench
rm -f benchmarks/qrdecomp/bin/qrbench

cmake ..

make
cd .. 

#install on linux
if [ $INSTALL -eq 1 ]
then
    sudo rm /usr/lib/libc3*
    sudo ln -t /usr/lib lib/libc3.*
fi
#install on mac
if [ $INSTALL -eq 2 ]
then
    sudo rm /usr/lib/libc3.*dylib
    sudo ln -F lib/libc3.dylib /usr/lib/libc3.dylib # for mac
fi



# add command line options to do this in the future

#Run tests
if [ $TESTING -eq 1 ]
then
    ./build/test/lib_stringmanip_test/stringmanip_test
    ./build/test/lib_array_test/array_test
    ./build/test/lib_linalg_test/linalg_test
    ./build/test/lib_quadrature_test/quad_test
    ./build/test/lib_funcs_test/funcs_test
    ./build/test/lib_clinalg_test/clinalg_test
    ./build/test/lib_tensor_test/tensor_test
    ./build/test/lib_tensdecomp_test/tensdecomp_test
    ./build/test/lib_tensdecomp_test/ttint_test
    ./build/test/lib_optimization_test/optimization_test
    ./build/test/lib_probability_test/probability_test
fi

