Vincent 3D Rendering Library for Pocket PC
==========================================


http://sourceforge.net/projects/ogl-es/



Overview
--------

The goal of this project is create a compliant implementation of the OpenGL (R) ES specification 
for mobile dvices. The initial target platform are Windows Mobile 2003 devices using the Intel 
StrongARM or XScale PA2xx processor.



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

