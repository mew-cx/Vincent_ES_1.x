#! /bin/bash

echo Some versions of flex do not handle the glslang.l file properly, in this case the lines:
echo '  extern int yylineno;'
echo '  int yylineno = 1;'
echo have to be removed from the file Gen_glslang.cpp and the last section of the glslang.l file must be appended at the end.

# build the StandAlone app and all it's dependencies
make -C StandAlone

# so we can find the shared library
LD_LIBRARY_PATH=`pwd`/glslang/MachineIndependent/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH

# run using test data
cd StandAlone
./StandAlone -i sample.vert sample.frag

