INCLUDE = -I. -I.. -I../.. -I../Include -I../Public -I../OSDependent/Linux -I../../OGLCompilersDLL
CC=g++
INCPREPROCESSOR=./preprocessor
INCCODEGEN=./../ARBCodeGen
INCOSDEPENDENT=./../OSDependent/Linux/
INCINITIALISATION=./../../OGLCompilersDLL/
LIBPREPROCESSOR=./preprocessor/libPreprocessor.a
LIBOSDEPENDENT=./../OSDependent/Linux/libOssource.a
LIBINITIALISATION=./../../OGLCompilersDLL/libInitializeDll.a
LIBCODEGEN=./../ARBCodeGen/libARBCodeGen.a
OBJECTS= Initialize.o IntermTraverse.o \
	Intermediate.o ParseHelper.o PoolAlloc.o QualifierAlive.o \
	RemoveTree.o ShaderLang.o intermOut.o parseConst.o SymbolTable.o \
	InfoSink.o

SRCS= Gen_glslang.cpp Gen_glslang_tab.cpp Initialize.cpp IntermTraverse.cpp \
	Intermediate.cpp ParseHelper.cpp PoolAlloc.cp QualifierAlive.cpp \
	RemoveTree.cpp ShaderLang.cpp SymbolTable.cpp intermOut.cpp \
	parseConst.cpp InfoSink.cpp
CPPFLAGS=$(DEFINE) $(INCLUDE)
SHAREDOBJECT=./lib/libglslang.so

default: all

all: $(SHAREDOBJECT)

$(SHAREDOBJECT): Gen_glslang.o Gen_glslang_tab.o $(OBJECTS) \
	$(LIBPREPROCESSOR) $(LIBCODEGEN) $(LIBOSDEPENDENT) $(LIBINITIALISATION)
	$(CC) -fPIC -shared -lc -o $@ $(OBJECTS) $(LIBPREPROCESSOR) $(LIBCODEGEN) $(LIBOSDEPENDENT) $(LIBINITIALISATION) Gen_glslang.o Gen_glslang_tab.o

Gen_glslang.o : Gen_glslang.cpp glslang_tab.h
	$(CC) -c $(INCLUDE) Gen_glslang.cpp -o $@

Gen_glslang_tab.o : Gen_glslang_tab.cpp 
	$(CC) -c $(INCLUDE) Gen_glslang_tab.cpp -o $@

Gen_glslang.cpp: glslang.l
	@echo Generating Gen_glslang.cpp
	@dos2unix glslang.l
	flex glslang.l

Gen_glslang_tab.cpp glslang_tab.h: glslang.y
	@echo Generating Gen_glslang_tab.cpp
	@dos2unix glslang.y
	bison -t -v -d glslang.y
	mv glslang.tab.c Gen_glslang_tab.cpp
	mv glslang.tab.h glslang_tab.h

%.o : %.cpp
	$(CC) $(CPPFLAGS) -c $<

$(LIBPREPROCESSOR):
	cd $(INCPREPROCESSOR); make 

$(LIBCODEGEN):
	cd $(INCCODEGEN); make 

$(LIBINITIALISATION):
	cd $(INCINITIALISATION); make 

$(LIBOSDEPENDENT):
	cd $(INCOSDEPENDENT); make 

depend:
	cd $(INCPREPROCESSOR); make depend
	cd $(INCCODEGEN); make depend
	cd $(INCINITIALISATION); make depend
	cd $(INCOSDEPENDENT); make depend
	makedepend -Y -- $(CPPFLAGS) -- $(SRCS) 
#
# Cleanup
#
.PHONY : clean
clean :
	$(RM) *.o *.a Gen_glslang_tab.cpp Gen_glslang.cpp glslang_tab.h glslang.output
	$(RM) ./lib/*.so
	cd $(INCPREPROCESSOR); make clean
	cd $(INCCODEGEN); make clean
	cd $(INCINITIALISATION); make clean
	cd $(INCOSDEPENDENT); make clean
	
# DO NOT DELETE

Gen_glslang.o: ParseHelper.h ../Include/ShHandle.h ../Public/ShaderLang.h
Gen_glslang.o: ../Include/ResourceLimits.h ../Include/InfoSink.h
Gen_glslang.o: ../Include/Common.h ../Include/PoolAlloc.h SymbolTable.h
Gen_glslang.o: ../Include/Common.h ../Include/intermediate.h
Gen_glslang.o: ../Include/Types.h ../Include/BaseTypes.h
Gen_glslang.o: ../Include/ConstantUnion.h ../Include/InfoSink.h
Gen_glslang.o: localintermediate.h ../Include/intermediate.h glslang_tab.h
Gen_glslang.o: ./unistd.h ./preprocessor/preprocess.h
Gen_glslang.o: ./preprocessor/slglobals.h ./preprocessor/memory.h
Gen_glslang.o: ./preprocessor/atom.h ./preprocessor/scanner.h
Gen_glslang.o: ./preprocessor/parser.h ./preprocessor/cpp.h
Gen_glslang.o: ./preprocessor/tokens.h ./preprocessor/symbols.h
Gen_glslang.o: ./preprocessor/compile.h
Gen_glslang_tab.o: SymbolTable.h ../Include/Common.h
Gen_glslang_tab.o: ../Include/intermediate.h ../Include/Common.h
Gen_glslang_tab.o: ../Include/PoolAlloc.h ../Include/Types.h
Gen_glslang_tab.o: ../Include/BaseTypes.h ../Include/ConstantUnion.h
Gen_glslang_tab.o: ../Include/InfoSink.h ParseHelper.h ../Include/ShHandle.h
Gen_glslang_tab.o: ../Public/ShaderLang.h ../Include/ResourceLimits.h
Gen_glslang_tab.o: ../Include/InfoSink.h localintermediate.h
Gen_glslang_tab.o: ../Include/intermediate.h
Initialize.o: ../Include/intermediate.h Initialize.h
Initialize.o: ../Include/ResourceLimits.h ../Include/Common.h
Initialize.o: ../Include/PoolAlloc.h ../Include/ShHandle.h
Initialize.o: ../Public/ShaderLang.h ../Include/ResourceLimits.h
Initialize.o: ../Include/InfoSink.h SymbolTable.h ../Include/Common.h
Initialize.o: ../Include/intermediate.h ../Include/Types.h
Initialize.o: ../Include/BaseTypes.h ../Include/ConstantUnion.h
Initialize.o: ../Include/InfoSink.h
IntermTraverse.o: ../Include/intermediate.h
Intermediate.o: localintermediate.h ../Include/intermediate.h
Intermediate.o: ../Public/ShaderLang.h ../Include/ResourceLimits.h
Intermediate.o: SymbolTable.h ../Include/Common.h ../Include/intermediate.h
Intermediate.o: ../Include/Common.h ../Include/PoolAlloc.h ../Include/Types.h
Intermediate.o: ../Include/BaseTypes.h ../Include/ConstantUnion.h
Intermediate.o: ../Include/InfoSink.h QualifierAlive.h RemoveTree.h
ParseHelper.o: ParseHelper.h ../Include/ShHandle.h ../Public/ShaderLang.h
ParseHelper.o: ../Include/ResourceLimits.h ../Include/InfoSink.h
ParseHelper.o: ../Include/Common.h ../Include/PoolAlloc.h SymbolTable.h
ParseHelper.o: ../Include/Common.h ../Include/intermediate.h
ParseHelper.o: ../Include/Types.h ../Include/BaseTypes.h
ParseHelper.o: ../Include/ConstantUnion.h ../Include/InfoSink.h
ParseHelper.o: localintermediate.h ../Include/intermediate.h
ParseHelper.o: ../Include/InitializeParseContext.h
ParseHelper.o: ../OSDependent/Linux/osinclude.h
ParseHelper.o: ../Include/InitializeGlobals.h ../Include/PoolAlloc.h
QualifierAlive.o: ../Include/intermediate.h
RemoveTree.o: ../Include/intermediate.h RemoveTree.h
ShaderLang.o: SymbolTable.h ../Include/Common.h ../Include/intermediate.h
ShaderLang.o: ../Include/Common.h ../Include/PoolAlloc.h ../Include/Types.h
ShaderLang.o: ../Include/BaseTypes.h ../Include/ConstantUnion.h
ShaderLang.o: ../Include/InfoSink.h ParseHelper.h ../Include/ShHandle.h
ShaderLang.o: ../Public/ShaderLang.h ../Include/ResourceLimits.h
ShaderLang.o: ../Include/InfoSink.h localintermediate.h
ShaderLang.o: ../Include/intermediate.h ../../OGLCompilersDLL/InitializeDll.h
ShaderLang.o: ../OSDependent/Linux/osinclude.h ../Include/InitializeGlobals.h
ShaderLang.o: ../Include/PoolAlloc.h Initialize.h ../Include/ResourceLimits.h
SymbolTable.o: SymbolTable.h ../Include/Common.h ../Include/intermediate.h
SymbolTable.o: ../Include/Common.h ../Include/PoolAlloc.h ../Include/Types.h
SymbolTable.o: ../Include/BaseTypes.h ../Include/ConstantUnion.h
SymbolTable.o: ../Include/InfoSink.h
intermOut.o: localintermediate.h ../Include/intermediate.h
intermOut.o: ../Public/ShaderLang.h ../Include/ResourceLimits.h SymbolTable.h
intermOut.o: ../Include/Common.h ../Include/intermediate.h
intermOut.o: ../Include/Common.h ../Include/PoolAlloc.h ../Include/Types.h
intermOut.o: ../Include/BaseTypes.h ../Include/ConstantUnion.h
intermOut.o: ../Include/InfoSink.h
parseConst.o: ParseHelper.h ../Include/ShHandle.h ../Public/ShaderLang.h
parseConst.o: ../Include/ResourceLimits.h ../Include/InfoSink.h
parseConst.o: ../Include/Common.h ../Include/PoolAlloc.h SymbolTable.h
parseConst.o: ../Include/Common.h ../Include/intermediate.h
parseConst.o: ../Include/Types.h ../Include/BaseTypes.h
parseConst.o: ../Include/ConstantUnion.h ../Include/InfoSink.h
parseConst.o: localintermediate.h ../Include/intermediate.h
InfoSink.o: ../Include/InfoSink.h