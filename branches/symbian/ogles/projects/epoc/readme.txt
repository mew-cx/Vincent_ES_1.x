This file contains instructions to build ogles library and test application using Symbian UIQ 2.1 SDK and MSVC (for UIQ emulator only, build for target device (ARM4/THUMB) is not possible at the moment).

1. From command line.
Make sure MSVC include and library path is set (you can run vcvars32.bat to do this). Set EPOCROOT to correct path. Go to ogles\projects\epoc folder and run the following commands:
bldmake bldfiles
abld export
abld build wins udeb
abld test build wins udeb

Before running test application, make sure the texture file (dodge.mbm) can be found in the same folder as test executable (test.app) in folder  z\system\apps\test. It should be copied automatically during the build process. 

2. Using MSVC6.0
To create projects for MSVC6.0:
bldmake bldfiles
abld export
abld makefile vc6
abld test makefile vc6

Set project dependency as follow:
test.dsp --> gles_cl.dsp --> codegen.dsp
                         --> int64x.dsp

