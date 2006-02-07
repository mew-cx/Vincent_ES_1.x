/*
//
//Copyright (C) 2005-2006  Falanx Microsystems AS
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
//    Neither the name of Falanx Microsystems AS nor the names of its
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
*/
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

//
// This file contains the Linux specific functions
//
#include "osinclude.h"
#include "InitializeDll.h"

#if !(defined(linux))
#error Trying to build a Linux specific file in a non-Linux build.
#endif


//
// Thread cleanup
//

//
// Wrapper for Linux call to DetachThread.  This is required as pthread_cleanup_push() expects
// the cleanup routine to return void.
//
void DetachThreadLinux(void *)
{
	DetachThread();
}


//
// Registers cleanup handler, sets cancel type and state, and excecutes the thread specific
// cleanup handler.  This function will be called in the Standalone.cpp for regression
// testing.  When OpenGL applications are run with the driver code, Linux OS does the
// thread cleanup.
//
void OS_CleanupThreadData(void)
{
	int old_cancel_state, old_cancel_type;
	void *cleanupArg = NULL;

	//
	// Set thread cancel state and push cleanup handler.
	//
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_cancel_state);
	pthread_cleanup_push(DetachThreadLinux, (void *) cleanupArg);

	//
	// Put the thread in deferred cancellation mode.
	//
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &old_cancel_type);

	//
	// Pop cleanup handler and execute it prior to unregistering the cleanup handler.
	//
	pthread_cleanup_pop(1);

	//
	// Restore the thread's previous cancellation mode.
	//
	pthread_setcanceltype(old_cancel_state, NULL);
}


//
// Thread Local Storage Operations
//
OS_TLSIndex OS_AllocTLSIndex()
{
	pthread_key_t pPoolIndex;

	//
	// Create global pool key.
	//
	if ((pthread_key_create(&pPoolIndex, NULL)) != 0) {
		assert(0 && "OS_AllocTLSIndex(): Unable to allocate Thread Local Storage");
		return false;
	}
	else
		return pPoolIndex;
}


bool OS_SetTLSValue(OS_TLSIndex nIndex, void *lpvValue)
{
	if (nIndex == OS_INVALID_TLS_INDEX) {
		assert(0 && "OS_SetTLSValue(): Invalid TLS Index");
		return false;
	}

	if (pthread_setspecific(nIndex, lpvValue) == 0)
		return true;
	else
		return false;
}


bool OS_FreeTLSIndex(OS_TLSIndex nIndex)
{
	if (nIndex == OS_INVALID_TLS_INDEX) {
		assert(0 && "OS_SetTLSValue(): Invalid TLS Index");
		return false;
	}

	//
	// Delete the global pool key.
	//
	if (pthread_key_delete(nIndex) == 0)
		return true;
	else
		return false;
}
