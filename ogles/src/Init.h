#ifndef EGL_INIT_H
#define EGL_INIT_H 1


#pragma once


// ==========================================================================
//
// Init.h	Static constructor for C++
//
// --------------------------------------------------------------------------
//
// 08-10-2003	Hans-Martin Will	initial version
//
// ==========================================================================


#include "OGLES.h"


namespace EGL {

	class OGLES_API Init {
	public:
		typedef void (*InitFunction)();

		Init(InitFunction init) {
			init();
		}
	};
}


#endif //ndef EGL_INIT_H