#ifndef GLES_GL_HASH_H
#define GLES_GL_HASH_H

/*
** ==========================================================================
**
** hash.h			Hash Table for Name Lookup
**
** --------------------------------------------------------------------------
**
** 02-05-2006		Hans-Martin Will	Adapted from Mesa-3D
**
** --------------------------------------------------------------------------
**
** Copyright (C) 1999-2005  Brian Paul   All Rights Reserved.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and associated documentation files (the "Software"),
** to deal in the Software without restriction, including without limitation
** the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the
** Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
** OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
** BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
** AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


struct HashTable *GlesNewHashTable(void);

void GlesDeleteHashTable(struct HashTable *table);

void * GlesHashLookup(const struct HashTable *table, GLuint key);

void GlesHashInsert(struct HashTable *table, GLuint key, void *data);

void GlesHashRemove(struct HashTable *table, GLuint key);

GLuint GlesHashFirstEntry(struct HashTable *table);

GLuint GlesHashNextEntry(const struct HashTable *table, GLuint key);

GLuint GlesHashFindFreeKeyBlock(struct HashTable *table, GLuint numKeys);


#endif /* ndef GLES_GL_HASH_H */