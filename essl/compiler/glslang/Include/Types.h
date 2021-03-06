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

#ifndef _TYPES_INCLUDED
#define _TYPES_INCLUDED

#include "../Include/Common.h"
#include "../Include/BaseTypes.h"

//
// Need to have association of line numbers to types in a list for building structs.
//
class TType;
struct TTypeLine {
    TType* type;
    int line;
};
typedef TVector<TTypeLine> TTypeList;

inline TTypeList* NewPoolTTypeList()
{
	void* memory = GlobalPoolAllocator.allocate(sizeof(TTypeList));
	return new(memory) TTypeList;
}

//
// This is a workaround for a problem with the yacc stack,  It can't have
// types that it thinks have non-trivial constructors.  It should
// just be used while recognizing the grammar, not anything else.  Pointers
// could be used, but also trying to avoid lots of memory management overhead.
//
// Not as bad as it looks, there is no actual assumption that the fields
// match up or are name the same or anything like that.
//
class TPublicType {
public:
    TBasicType type;
    TQualifier qualifier;
	TQualifier precision;			// Added for ESSL support
	bool invariant;					// Added for ESSL support
	int size;					    // size of vector or matrix, not size of array
    bool matrix;
    bool array;
    int arraySize;
    TType* userDef;
    int line;

	bool validType;				// Added for ESSL support. Used for setting a type invalid (used when declaring variables invariant)

    void setBasic(TBasicType bt, TQualifier q, int ln = 0)
    {
        type = bt;
        qualifier = q;
        size = 1;
        matrix = false;
        array = false;
        arraySize = 0;
        userDef = 0;
        line = ln;
		invariant = false;					// Added for ESSL support
		validType = true;					// Added for ESSL support
	}

    void setAggregate(int s, bool m = false)
    {
        size = s;
        matrix = m;
		invariant = false;					// Added for ESSL support
    }

    void setArray(bool a, int s = 0)
    {
        array = a;
        arraySize = s;
		invariant = false;					// Added for ESSL support
    }

	// Added for ESSL support
	void makeInvariant()
	{
		invariant = true;
	}

	// Added for ESSL support	(just a workaround for being able to declare a comma separated list of variables as invariant)
	void setInvalid(){
		validType = false;
	}
};

typedef std::map<TTypeList*, TTypeList*> TStructureMap;
typedef std::map<TTypeList*, TTypeList*>::iterator TStructureMapIterator;
//
// Base class for things that have a type.
//
class TType {
public:
    POOL_ALLOCATOR_NEW_DELETE(GlobalPoolAllocator)

	// Altered for ESSL support (precision, invariant)
	explicit TType(TBasicType t, TQualifier q = EvqTemporary, TQualifier p = EvqNoPrecSpecified, int s = 1, bool m = false, bool a = false) :
                            type(t), qualifier(q), precision(p), invariant(false), size(s), matrix(m), array(a), arraySize(0),
                            structure(0), structureSize(0), maxArraySize(0), arrayInformationType(0), fieldName(0), mangled(0), typeName(0)
                            { }
	// Altered for ESSL support (precision, invariant)
    explicit TType(const TPublicType &p) :
							type(p.type), qualifier(p.qualifier), precision(p.precision), invariant(p.invariant), size(p.size), matrix(p.matrix), array(p.array), arraySize(p.arraySize),
                            structure(0), structureSize(0), maxArraySize(0), arrayInformationType(0), fieldName(0), mangled(0), typeName(0)
                            {
                              if (p.userDef) {
                                  structure = p.userDef->getStruct();
                                  typeName = NewPoolTString(p.userDef->getTypeName().c_str());
                              }
                            }
	// Altered for ESSL support (precision, invariant)
    explicit TType(TTypeList* userDef, const TString& n) :
                            type(EbtStruct), qualifier(EvqTemporary), precision(EvqNoPrecSpecified), invariant(false), size(1), matrix(false), array(false), arraySize(0),
                            structure(userDef), maxArraySize(0), arrayInformationType(0), fieldName(0), mangled(0) {
								typeName = NewPoolTString(n.c_str());
                            }
	explicit TType() {}
    virtual ~TType() {}

	TType(const TType& type) { *this = type; }

	void copyType(const TType& copyOf, TStructureMap& remapper)
	{
		type = copyOf.type;
		qualifier = copyOf.qualifier;
		precision = copyOf.precision;						// Added for ESSL support
		invariant = copyOf.invariant;						// Added for ESSL support
		size = copyOf.size;
		matrix = copyOf.matrix;
		array = copyOf.array;
		arraySize = copyOf.arraySize;

		TStructureMapIterator iter;
		if (copyOf.structure) {
	        if ((iter = remapper.find(structure)) == remapper.end()) {
				// create the new structure here
				structure = NewPoolTTypeList();
				for (unsigned int i = 0; i < copyOf.structure->size(); ++i) {
					TTypeLine typeLine;
					typeLine.line = (*copyOf.structure)[i].line;
					typeLine.type = (*copyOf.structure)[i].type->clone(remapper);
					structure->push_back(typeLine);
				}
			} else {
				structure = iter->second;
			}
		} else
			structure = 0;

		fieldName = 0;
		if (copyOf.fieldName)
			fieldName = NewPoolTString(copyOf.fieldName->c_str());
		typeName = 0;
		if (copyOf.typeName)
			typeName = NewPoolTString(copyOf.typeName->c_str());

		mangled = 0;
		if (copyOf.mangled)
			mangled = NewPoolTString(copyOf.mangled->c_str());

		structureSize = copyOf.structureSize;
		maxArraySize = copyOf.maxArraySize;
		assert(copyOf.arrayInformationType == 0);
		arrayInformationType = 0; // arrayInformationType should not be set for builtIn symbol table level
	}

	TType* clone(TStructureMap& remapper)
	{
		TType *newType = new TType();
		newType->copyType(*this, remapper);

		return newType;
	}

    virtual void setType(TBasicType t, int s, bool m, bool a, int aS = 0)
                            { type = t; size = s; matrix = m; array = a; arraySize = aS; }
    virtual void setType(TBasicType t, int s, bool m, TType* userDef = 0)
                            { type = t;
                              size = s;
                              matrix = m;
                              if (userDef)
                                  structure = userDef->getStruct();
                              // leave array information intact.
                            }
    virtual void setTypeName(const TString& n) { typeName = NewPoolTString(n.c_str()); }
    virtual void setFieldName(const TString& n) { fieldName = NewPoolTString(n.c_str()); }
    virtual const TString& getTypeName() const
    {
		assert(typeName);
    	return *typeName;
    }

    virtual const TString& getFieldName() const
    {
    	assert(fieldName);
		return *fieldName;
    }

    virtual TBasicType getBasicType() const { return type; }
    virtual TQualifier getQualifier() const { return qualifier; }
    virtual TQualifier getPrecision() const { return precision; }		// Added for ESSL support
    virtual void setPrecision(TQualifier prec) { precision = prec; }	// Added for ESSL support
	virtual bool	   isInvariant() const { return invariant; }		// Added for ESSL support
	virtual void	   makeInvariant(){ invariant = true; }				// Added for ESSL support
	virtual void changeQualifier(TQualifier q) { qualifier = q; }

    // One-dimensional size of single instance type
    virtual int getNominalSize() const { return size; }

    // Full-dimensional size of single instance of type
    virtual int getInstanceSize() const
    {
        if (matrix)
            return size * size;
        else
            return size;
    }

	virtual bool isMatrix() const { return matrix ? true : false; }
    virtual bool isArray() const  { return array ? true : false; }
    int getArraySize() const { return arraySize; }
    void setArraySize(int s) { array = true; arraySize = s; }
    void setMaxArraySize (int s) { maxArraySize = s; }
    int getMaxArraySize () const { return maxArraySize; }
    void clearArrayness() { array = false; arraySize = 0; maxArraySize = 0; }
    void setArrayInformationType(TType* t) { arrayInformationType = t; }
    TType* getArrayInformationType() { return arrayInformationType; }
    virtual bool isVector() const { return size > 1 && !matrix; }
    static char* getBasicString(TBasicType t);
    const char* getBasicString() const { return TType::getBasicString(type); }
    const char* getQualifierString() const { return ::getQualifierString(qualifier); }
    TTypeList* getStruct() { return structure; }

    int getObjectSize() const
    {
        int totalSize;

        if (getBasicType() == EbtStruct)
            totalSize = getStructSize();
        else if (matrix)
            totalSize = size * size;
        else
            totalSize = size;

        if (isArray())
            totalSize *= Max(getArraySize(), getMaxArraySize());

        return totalSize;
    }

    TTypeList* getStruct() const { return structure; }
    TString& getMangledName() {
        if (!mangled) {
			mangled = NewPoolTString("");
            buildMangledName(*mangled);
            *mangled += ';' ;
        }

        return *mangled;
    }
    bool sameElementType(const TType& right) const {
        return      type == right.type   &&
                    size == right.size   &&
                  matrix == right.matrix &&
               structure == right.structure;
    }
    bool operator==(const TType& right) const {
        return      type == right.type   &&
                    size == right.size   &&
                  matrix == right.matrix &&
                   array == right.array  && (!array || arraySize == right.arraySize) &&
               structure == right.structure;
        // don't check the qualifier, it's not ever what's being sought after
    }
    bool operator!=(const TType& right) const {
        return !operator==(right);
    }
    TString getCompleteString() const;

protected:
    void buildMangledName(TString&);
    int getStructSize() const;

	TBasicType type      : 6;
	TQualifier qualifier : 7;
	int size             : 8; // size of vector or matrix, not size of array
	unsigned int matrix  : 1;
	unsigned int array   : 1;
    int arraySize;

	TQualifier precision;		// Added for ESSL support
	bool invariant;				// Added for ESSL support

    TTypeList* structure;      // 0 unless this is a struct
    mutable int structureSize;
    int maxArraySize;
    TType* arrayInformationType;
	TString *fieldName;         // for structure field names
    TString *mangled;
	TString *typeName;          // for structure field type name
};

#endif // _TYPES_INCLUDED_
