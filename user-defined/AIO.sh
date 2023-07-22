echo "---make---"
make clean
make

echo "---setup environment varaibles---"
echo "export DYNINST_ROOT=/home/lzy/Desktop/dyninst-10.2.1"
export DYNINST_ROOT=/home/lzy/Desktop/dyninst-10.2.1
echo "export DYNINST_LIB=${DYNINST_ROOT}/build/lib"
export DYNINST_LIB=$DYNINST_ROOT/build/lib
echo "export DYNINSTAPI_RT_LIB=${DYNINST_LIB}/libdyninstAPI_RT.so"
export DYNINSTAPI_RT_LIB=$DYNINST_LIB/libdyninstAPI_RT.so
echo "export LD_LIBRARY_PATH=${DYNINST_LIB}:${LD_LIBRARY_PATH}"
export LD_LIBRARY_PATH=$DYNINST_LIB:$LD_LIBRARY_PATH

echo "---perform binary rewritting---"
echo -n "instrumentation time : "
{ time ./mutator ; } 2>&1 | grep real | awk '{print $2}'

echo "---run the rewritten binary---"
./mutatee-rewritten
