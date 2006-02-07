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


#include <GLES/gl.h>
#include "config.h"
#include "platform/platform.h"
#include "gl/hash.h"


#define TABLE_SIZE 1023  /**< Size of lookup table/array */

#define HASH_FUNC(K)  ((K) % TABLE_SIZE)


/**
* An entry in the hash table.  
*
* This struct is private to this file.
*/
struct HashEntry {
	GLuint Key;             /**< the entry's key */
	void *Data;             /**< the entry's data */
	struct HashEntry *Next; /**< pointer to next entry */
};

/**
* The hash table data structure.  
*
* This is an opaque types (it's not defined in hash.h file).
*/
struct HashTable {
	struct HashEntry *Table[TABLE_SIZE];  /**< the lookup table */
	GLuint MaxKey;                        /**< highest key inserted so far */
};


/**
* Create a new hash table.
* 
* \return pointer to a new, empty hash table.
*/
struct HashTable *
	GlesNewHashTable(void)
{
	struct HashTable *table = MallocClient(sizeof(struct HashTable));

	return table;
}



/**
* Delete a hash table.
* Frees each entry on the hash table and then the hash table structure itself.
* Note that the caller should have already traversed the table and deleted
* the objects in the table (i.e. We don't free the entries' data pointer).
*
* \param table the hash table to delete.
*/
void
GlesDeleteHashTable(struct HashTable *table)
{
	GLuint i;
	GLES_ASSERT(table);
	for (i = 0; i < TABLE_SIZE; i++) {
		struct HashEntry *entry = table->Table[i];
		while (entry) {
			struct HashEntry *next = entry->Next;
			FreeClient(entry);
			entry = next;
		}
	}
	FreeClient(table);
}



/**
* Lookup an entry in the hash table.
* 
* \param table the hash table.
* \param key the key.
* 
* \return pointer to user's data or NULL if key not in table
*/
void *
GlesHashLookup(const struct HashTable *table, GLuint key)
{
	GLuint pos;
	const struct HashEntry *entry;

	GLES_ASSERT(table);
	GLES_ASSERT(key);

	pos = HASH_FUNC(key);
	entry = table->Table[pos];
	while (entry) {
		if (entry->Key == key) {
			return entry->Data;
		}
		entry = entry->Next;
	}
	return NULL;
}



/**
* Insert a key/pointer pair into the hash table.  
* If an entry with this key already exists we'll replace the existing entry.
* 
* \param table the hash table.
* \param key the key (not zero).
* \param data pointer to user data.
*/
void
GlesHashInsert(struct HashTable *table, GLuint key, void *data)
{
	/* search for existing entry with this key */
	GLuint pos;
	struct HashEntry *entry;

	GLES_ASSERT(table);
	GLES_ASSERT(key);

	if (key > table->MaxKey)
		table->MaxKey = key;

	pos = HASH_FUNC(key);
	entry = table->Table[pos];
	while (entry) {
		if (entry->Key == key) {
			/* replace entry's data */
			entry->Data = data;
			return;
		}
		entry = entry->Next;
	}

	/* alloc and insert new table entry */
	entry = MallocClient(sizeof(struct HashEntry));
	entry->Key = key;
	entry->Data = data;
	entry->Next = table->Table[pos];
	table->Table[pos] = entry;
}



/**
* Remove an entry from the hash table.
* 
* \param table the hash table.
* \param key key of entry to remove.
*
* While holding the hash table's lock, searches the entry with the matching
* key and unlinks it.
*/
void
GlesHashRemove(struct HashTable *table, GLuint key)
{
	GLuint pos;
	struct HashEntry *entry, *prev;

	GLES_ASSERT(table);
	GLES_ASSERT(key);

	pos = HASH_FUNC(key);
	prev = NULL;
	entry = table->Table[pos];
	while (entry) {
		if (entry->Key == key) {
			/* found it! */
			if (prev) {
				prev->Next = entry->Next;
			}
			else {
				table->Table[pos] = entry->Next;
			}
			FreeClient(entry);
			return;
		}
		prev = entry;
		entry = entry->Next;
	}
}



/**
* Get the key of the "first" entry in the hash table.
* 
* This is used in the course of deleting all display lists when
* a context is destroyed.
* 
* \param table the hash table
* 
* \return key for the "first" entry in the hash table.
*
* While holding the lock, walks through all table positions until finding
* the first entry of the first non-empty one.
*/
GLuint
GlesHashFirstEntry(struct HashTable *table)
{
	GLuint pos;
	GLES_ASSERT(table);

	for (pos=0; pos < TABLE_SIZE; pos++) {
		if (table->Table[pos]) {
			return table->Table[pos]->Key;
		}
	}

	return 0;
}


/**
* Given a hash table key, return the next key.  This is used to walk
* over all entries in the table.  Note that the keys returned during
* walking won't be in any particular order.
* \return next hash key or 0 if end of table.
*/
GLuint
GlesHashNextEntry(const struct HashTable *table, GLuint key)
{
	const struct HashEntry *entry;
	GLuint pos;

	GLES_ASSERT(table);
	GLES_ASSERT(key);

	/* Find the entry with given key */
	pos = HASH_FUNC(key);
	entry = table->Table[pos];
	while (entry) {
		if (entry->Key == key) {
			break;
		}
		entry = entry->Next;
	}

	if (!entry) {
		/* the key was not found, we can't find next entry */
		return 0;
	}

	if (entry->Next) {
		/* return next in linked list */
		return entry->Next->Key;
	}
	else {
		/* look for next non-empty table slot */
		pos++;
		while (pos < TABLE_SIZE) {
			if (table->Table[pos]) {
				return table->Table[pos]->Key;
			}
			pos++;
		}
		return 0;
	}
}


/**
* Find a block of adjacent unused hash keys.
* 
* \param table the hash table.
* \param numKeys number of keys needed.
* 
* \return Starting key of free block or 0 if failure.
*
* If there are enough free keys between the maximum key existing in the table
* (HashTable::MaxKey) and the maximum key possible, then simply return
* the adjacent key. Otherwise do a full search for a free key block in the
* allowable key range.
*/
GLuint
GlesHashFindFreeKeyBlock(struct HashTable *table, GLuint numKeys)
{
	GLuint maxKey = ~((GLuint) 0);

	if (maxKey - numKeys > table->MaxKey) {
		/* the quick solution */
		return table->MaxKey + 1;
	}
	else {
		/* the slow solution */
		GLuint freeCount = 0;
		GLuint freeStart = 1;
		GLuint key;
		for (key=1; key!=maxKey; key++) {
			if (GlesHashLookup(table, key)) {
				/* darn, this key is already in use */
				freeCount = 0;
				freeStart = key+1;
			}
			else {
				/* this key not in use, check if we've found enough */
				freeCount++;
				if (freeCount == numKeys) {
					return freeStart;
				}
			}
		}
		/* cannot allocate a block of numKeys consecutive keys */
		return 0;
	}
}
