//
//Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//POSSIBILITY OF SUCH DAMAGE.
//
#include "./../glslang/Include/ShHandle.h"
#include "./../glslang/Public/ShaderLang.h"
#include <string.h>
#include <math.h>

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
#endif

#include <iostream>

using namespace std;

//
// Return codes from main.
//
enum TFailCode {
    ESuccess = 0,
    EFailUsage,
    EFailCompile,
    EFailLink,
    EFailCompilerCreate,
    EFailLinkerCreate
};

static EShLanguage FindLanguage(char *lang);
bool CompileFile(char *fileName, ShHandle, int, const TBuiltInResource*);
void usage();
void FreeFileData(char **data);
char** ReadFileData(char *fileName);
//Added to accomodate the multiple strings.
int OutputMultipleStrings = 1;

//
// Set up the per compile resources
//
void GenerateResources(TBuiltInResource& resources)
{    
//    resources.maxLights = 32;
//    resources.maxClipPlanes = 6;
//    resources.maxTextureUnits = 32;
//    resources.maxTextureCoords = 32;
    resources.maxVertexAttribs = 8;
    resources.maxVertexUniformComponents = 384;
    resources.maxVaryingFloats = 32;
    resources.maxVertexTextureImageUnits = 0;
    resources.maxCombinedTextureImageUnits = 2;
    resources.maxTextureImageUnits = 2;
    resources.maxFragmentUniformComponents = 64;
    resources.maxDrawBuffers = 1;
}

int main(int argc, char* argv[])
{
	ShInitialize();
	
	cout << "# Vertex program:" << endl;	
	ShHandle vertcompiler = ShConstructCompiler(EShLangVertex, EDebugOpNone);
	TBuiltInResource vertresources;
	GenerateResources(vertresources);
	CompileFile("basic.vert", vertcompiler, EDebugOpNone, &vertresources);
	ShDestruct(vertcompiler);
	cout << endl;

	cout << "# Fragment program:" << endl;	
	ShHandle fragcompiler = ShConstructCompiler(EShLangFragment, EDebugOpNone);
	TBuiltInResource fragresources;
	GenerateResources(fragresources);
	CompileFile("basic.frag", fragcompiler, EDebugOpNone, &fragresources);
	ShDestruct(fragcompiler);
	cout << endl;
	
	return 0;
}

//
//   Read a file's data into a string, and compile it using ShCompile
//
bool CompileFile(char *fileName, ShHandle compiler, int debugOptions, const TBuiltInResource *resources)
{
    int ret;
    char **data = ReadFileData(fileName);

    if (!data)
        return false;

    ret = ShCompile(compiler, data, OutputMultipleStrings, EShOptNone, resources, debugOptions);
//	cout << "# Infolog:" << endl;
	if (!ret) {
		const char* infolog = ShGetInfoLog(compiler);
		if (infolog) {
			cout << infolog << endl;
		} else {
			cerr << "Error compiling file" << endl;
		}
	}

    FreeFileData(data);

    return ret ? true : false;
}


//
//   Malloc a string of sufficient size and read a string into it.
//
# define MAX_SOURCE_STRINGS 5
char** ReadFileData(char *fileName) 
{
    FILE *in    = fopen(fileName, "r");
    char *fdata;
    int count = 0;
    char**return_data=(char**)malloc(MAX_SOURCE_STRINGS+1);

    //return_data[MAX_SOURCE_STRINGS]=NULL;
	if (!in) {
        printf("Error: unable to open input file: %s\n", fileName);
        return 0;
    }
    
	while (fgetc(in) != EOF)
		count++;

	fseek(in, 0, SEEK_SET);
	
	if (!(fdata = (char *)malloc(count+2))) {
            printf("Error allocating memory\n");
            return 0;
    }
	if (fread(fdata,1,count, in)!=count) {
            printf("Error reading input file: %s\n", fileName);
            return 0;
    }
    fdata[count] = '\0';
    fclose(in);
    if(count==0){
        return_data[0]=(char*)malloc(count+2);
        return_data[0][0]='\0';
        OutputMultipleStrings=0;
        return return_data;       
    }

	int len = (int)(ceil)((float)count/(float)OutputMultipleStrings);
    int ptr_len=0,i=0;
	while(count>0){
		return_data[i]=(char*)malloc(len+2);
		memcpy(return_data[i],fdata+ptr_len,len);
		return_data[i][len]='\0';
		count-=(len);
		ptr_len+=(len);
		if(count<len){
            if(count==0){
               OutputMultipleStrings=(i+1);
               break;
            }
           len = count;
		}  
		++i;
	}
    return return_data;
}

void FreeFileData(char **data)
{
    for(int i=0;i<OutputMultipleStrings;i++)
        free(data[i]);
}
