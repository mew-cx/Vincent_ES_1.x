Vincent 3D Rendering Library for Pocket PC
==========================================


http://sourceforge.net/projects/ogl-es/



Overview
--------

Vincent develops a compliant implementation of the OpenGL (R) ES 1.0 API specification for 
Pocket PCs and Smartphones. The current target platforms are Windows Mobile and Symbian devices 
using TI OMAP, Intel (R) StrongARM and XScale PA2xx processors.

As officially registered adopter of the OpenGL ES API, this project has full access to the
official OpenGL ES 1.0 conformance test. Starting with build 0.8, Vincent fully passes the 
conformance test for the Common Light profile, i.e. the fixed point API defined by the standard.

Further builds of the project will focus on addressing performance issues present in the
current build, as well as adressing issues as reported by users that are not covered by
the tests we have run so far.



How the project is organized
----------------------------

This project has build configurations for the following configurations:

- Visual Studio.NET 2003 for IA32/Windows
- Embedded Visual C++ 4.0 SP3 (both Emulator, ARM and XScale) for PocketPC 2003 or Smartphone 2003

You will need at least one of these environments installed on your computer. Please refer
to the project home page for links on how to obtain these tools.

The project tree as checked into sourceforge provides everything you need to build the project. The
folder structure is as follows:

/src				- Source files

/projects			- Visual Studio project files
	/evc4			- for embedded Visual C++ 4.0
	/vcnet			- for Visual Studio .NET 2003

/obj				- Intermediate build files
	/arm			- for ARM
		/Debug
		/Release
	/emu			- for PocketPC emulator
		/Debug
		/Release
	/x86			- for Windows
		/Debug
		/Release
	/xscale			- for Intel XScale
		/Debug
		/Release

/bin				- Final binaries
	/arm
		/Debug
		/Release
	/emu
		/Debug
		/Release
	/x86
		/Debug
		/Release
	/xscale
		/Debug
		/Release

/dist				- Distribution folders
	/arm
		/Debug
		/Release
	/emu
		/Debug
		/Release
	/x86
		/Debug
		/Release
	/xscale
		/Debug
		/Release

/tools				- Build scripts and helper tools



How do I build this project from the source?
--------------------------------------------

To build the libraries, open the corresponding project/workspace from the projects
subtree. If the download is correct, the project should build right away without 
any further adjustment.


How do I get support?
---------------------

Please refer to the SourceForge project homepage on how to obtain support.


How can I contribute?
---------------------

Please refer to the SourceForge project homepage on how to contribute to this project.

