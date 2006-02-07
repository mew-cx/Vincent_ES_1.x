CC=g++
OBJECTPATH=./../glslang/MachineIndependent
LIBPATH=./../glslang/MachineIndependent/lib
SRCS=ARBCompiler.cpp

TARGETOBJECT=ARBCompiler.o

default: all

all: ARBCompiler

ARBCompiler: $(TARGETOBJECT) SHAREDOBJECT
	$(CC) -g -o $@ $(TARGETOBJECT) -L $(LIBPATH) -lglslang -lpthread \
	-lm -lstdc++

SHAREDOBJECT:
	cd $(OBJECTPATH); make all

%.o : %.cpp
	$(CC) -c $<

#
# Cleanup
#
.PHONY : clean
clean :
	$(RM) *.o ARBCompiler
	cd $(OBJECTPATH); make clean

depend:
	cd $(OBJECTPATH); make depend
	makedepend -Y -- $(SRCS)
# DO NOT DELETE

ARBCompiler.o: ./../glslang/Include/ShHandle.h
ARBCompiler.o: ./../glslang/Public/ShaderLang.h
ARBCompiler.o: ./../glslang/Include/ResourceLimits.h
ARBCompiler.o: ./../glslang/Include/InfoSink.h ./../glslang/Include/Common.h
ARBCompiler.o: ./../glslang/Include/PoolAlloc.h
ARBCompiler.o: ./../glslang/Public/ShaderLang.h
