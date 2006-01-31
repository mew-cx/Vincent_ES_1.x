/*
** ==========================================================================
**
** pool.c			Object Pool Management
**
** --------------------------------------------------------------------------
**
** 06-17-2005		Hans-Martin Will	initial version
**
** --------------------------------------------------------------------------
**
** Copyright (c) 2003-2006, Hans-Martin Will. All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
**	 *  Redistributions of source code must retain the above copyright
** 		notice, this list of conditions and the following disclaimer.
**   *	Redistributions in binary form must reproduce the above copyright
** 		notice, this list of conditions and the following disclaimer in the
** 		documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
** OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
** THE POSSIBILITY OF SUCH DAMAGE.
**
** ==========================================================================
*/

#include <GLES/gl.h>
#include "config.h"
#include "platform/platform.h"
#include "gl/state.h"




#define NIL (~0u)

#define IS_NIL(p)		(((GLuint) (p)) == NIL)
#define IS_POINTER(p)	(((GLuint) (p)) & 1 == 0)
#define MAKE_INDEX(i)	(((GLuint) (p)) << 1 | 1)
#define GET_INDEX(p)	(((GLuint) (p)) >> 1)


/*
** --------------------------------------------------------------------------
** Macro set to encode both object pointers as well as free list into
** machine words.
** --------------------------------------------------------------------------
*/

#if 0
void Pool_Initialize(ObjectPool * pool, GLuint initialSize) {

	GLuint index;

	pool->numFreeObjects = pool->numAllocatedObjects = initialSize;
	pool->freeListHead = 0;
	pool->objects = malloc(sizeof(void *) * initialSize);

	if (!pool->objects) {
		RecordError(GLES_GET_STATE(), GL_OUT_OF_MEMORY);
		return;
	}

	for (index = 0; index < initialSize; ++index) {
		pool->objects[index] = MAKE_INDEX(index + 1);
	}

	pool->objects[initialSize - 1] = NIL;
}

void Pool_Destroy(ObjectPool * pool) {

	if (pool->objects != 0) {
		GLuint index;

		for (index = 0; index < pool->numAllocatedObjects; ++index) {
			void * obj =  pool->objects[index];

			if (IS_POINTER(obj) && obj) {
				if (pool->destructor) {
					pool->destructor(obj);
				}
			}
		}

		free(pool->objects);
		free(pool->freeList);
	}
}

GLuint Pool_Allocate(ObjectPool * pool) {

	GLuint result;

	if (pool->numFreeObjects == 0) {

		/* increase array sizes */

		GLuint newAllocatedObjects = pool->numAllocatedObjects * 2;
		void ** newObjects = malloc(sizeof(void *) * newAllocatedObjects);
		GLuint index;

		for (index = 0; index < pool->numAllocatedObjects; ++index) {
			newObjects[index] = pool->objects[index];
		}

		for (index = pool->numAllocatedObjects; index < newAllocatedObjects - 1; ++index) {
			newFreeList[index] = MAKE_INDEX(index + 1);
		}

		newObjects[newAllocatedObjects - 1] = NIL;

		free(pool->objects);

		pool->objects = newObjects;
		pool->numFreeObjects = newAllocatedObjects - pool->numAllocatedObjects;
		pool->freeListHead = pool->numAllocatedObjects;
		pool->numAllocatedObjects = newAllocatedObjects;
	}

	result = pool->freeListHead;
	pool->freeListHead = pool->freeList[result];
	pool->numFreeObjects--;
	pool->objects[result] = NULL;

	return result;
}

void Pool_Deallocate(ObjectPool * pool, GLuint index) {

	void * obj;

	if (index >= pool->numAllocatedObjects)
		return;

	obj = pool->objects[index];

	if (IS_POINTER(obj)) {
		assert(!obj);

		pool->objects[index] = MAKE_INDEX(pool->freeListHead);
		pool->freeListHead = index;
		pool->numFreeObjects++;
	}
}

GLboolean Pool_IsObject(ObjectPool * pool, GLuint index) {
	return index < pool->numAllocatedObjects && IS_POINTER(pool->objects[index]);
}

void * Pool_GetObject(ObjectPool * pool, GLuint index) {

	if (index >= pool->numAllocatedObjects || !IS_POINTER(pool->objects[index])) 
		return NULL;

	return pool->objects[index];
}

void * Pool_SetObject(ObjectPool * pool, GLuint index, void * obj) {

	if (index >= pool->numAllocatedObjects || !IS_POINTER(pool->objects[index])) 
		return NULL;

	assert(!pool->objects[index] || !obj);

	pool->objects[index] = obj;
}

#endif