CC=g++

OBJECTS=ARBBackEnd.o ARBInstruction.o ARBFunctions.o ARBNodeHandling.o ARBOptimization.o FrontEndInterface.o Link.o Util.o
AR=ar

SRCS=ARBBackEnd.cpp ARBInstruction.cpp ARBFunctions.cpp ARBNodeHandling.cpp ARBOptimization.cpp FrontEndInterface.cpp Link.cpp Util.cpp

default : all
all : libARBCodeGen.a

libARBCodeGen.a : $(OBJECTS)
	$(AR) rvu $@ $(OBJECTS)
	ranlib $@

%.o : %.cpp
	$(CC) -c $<

#
# Cleanup
#
.PHONY : clean
clean :
	$(RM) *.o *.a

depend:
	makedepend -Y -- $(CFLAGS) -- $(SRCS)
# DO NOT DELETE

ARBBackEnd.o: ../Include/intermediate.h ../Include/Common.h
ARBBackEnd.o: ../Include/PoolAlloc.h ../Include/Types.h
ARBBackEnd.o: ../Include/BaseTypes.h ../Include/ConstantUnion.h ARBCompiler.h
ARBBackEnd.o: ../Include/ShHandle.h ../Public/ShaderLang.h
ARBBackEnd.o: ../Include/ResourceLimits.h ../Include/InfoSink.h
ARBBackEnd.o: ARBInstruction.h ARBInstructionType.h ARBNodeHandling.h
ARBBackEnd.o: ARBBackEnd.h ARBFunctions.h ARBOptimization.h Util.h
ARBBackEnd.o: ../Include/InfoSink.h
ARBInstruction.o: ../Include/intermediate.h ../Include/Common.h
ARBInstruction.o: ../Include/PoolAlloc.h ../Include/Types.h
ARBInstruction.o: ../Include/BaseTypes.h ../Include/ConstantUnion.h
ARBInstruction.o: ARBInstructionType.h Util.h ../Include/InfoSink.h
ARBInstruction.o: ARBInstruction.h ../Include/ShHandle.h
ARBInstruction.o: ../Public/ShaderLang.h ../Include/ResourceLimits.h
ARBInstruction.o: ../Include/InfoSink.h
ARBFunctions.o: ARBFunctions.h ARBInstruction.h ARBInstructionType.h
ARBFunctions.o: ../Include/intermediate.h ../Include/Common.h
ARBFunctions.o: ../Include/PoolAlloc.h ../Include/Types.h
ARBFunctions.o: ../Include/BaseTypes.h ../Include/ConstantUnion.h
ARBFunctions.o: ../Include/ShHandle.h ../Public/ShaderLang.h
ARBFunctions.o: ../Include/ResourceLimits.h ../Include/InfoSink.h
ARBFunctions.o: ARBBackEnd.h Util.h ../Include/InfoSink.h
ARBNodeHandling.o: ARBFunctions.h ARBInstruction.h ARBInstructionType.h
ARBNodeHandling.o: ../Include/intermediate.h ../Include/Common.h
ARBNodeHandling.o: ../Include/PoolAlloc.h ../Include/Types.h
ARBNodeHandling.o: ../Include/BaseTypes.h ../Include/ConstantUnion.h
ARBNodeHandling.o: ../Include/ShHandle.h ../Public/ShaderLang.h
ARBNodeHandling.o: ../Include/ResourceLimits.h ../Include/InfoSink.h
ARBNodeHandling.o: ARBBackEnd.h Util.h ../Include/InfoSink.h
ARBNodeHandling.o: ARBNodeHandling.h
ARBOptimization.o: ARBOptimization.h ARBBackEnd.h ARBInstruction.h
ARBOptimization.o: ARBInstructionType.h ../Include/intermediate.h
ARBOptimization.o: ../Include/Common.h ../Include/PoolAlloc.h
ARBOptimization.o: ../Include/Types.h ../Include/BaseTypes.h
ARBOptimization.o: ../Include/ConstantUnion.h ../Include/ShHandle.h
ARBOptimization.o: ../Public/ShaderLang.h ../Include/ResourceLimits.h
ARBOptimization.o: ../Include/InfoSink.h Util.h ../Include/InfoSink.h
FrontEndInterface.o: ../Include/Common.h ../Include/PoolAlloc.h
FrontEndInterface.o: ../Include/ShHandle.h ../Public/ShaderLang.h
FrontEndInterface.o: ../Include/ResourceLimits.h ../Include/InfoSink.h
FrontEndInterface.o: ARBCompiler.h
Link.o: ../Include/Common.h ../Include/PoolAlloc.h ../Include/ShHandle.h
Link.o: ../Public/ShaderLang.h ../Include/ResourceLimits.h
Link.o: ../Include/InfoSink.h
Util.o: Util.h ../Include/intermediate.h ../Include/Common.h
Util.o: ../Include/PoolAlloc.h ../Include/Types.h ../Include/BaseTypes.h
Util.o: ../Include/ConstantUnion.h ../Include/InfoSink.h ARBInstruction.h
Util.o: ARBInstructionType.h ../Include/ShHandle.h ../Public/ShaderLang.h
Util.o: ../Include/ResourceLimits.h ../Include/InfoSink.h
