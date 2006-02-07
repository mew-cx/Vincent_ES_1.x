#ifndef ARBCOMPILER_H
#define ARBCOMPILER_H

#include "../Include/Common.h"
#include "../Include/ShHandle.h"

//
// Here is where real machine specific high-level data would be defined.
//
class ARBCompiler : public TCompiler {
public:
	ARBCompiler(EShLanguage l, int dOptions) : TCompiler(l, infoSink), debugOptions(dOptions) {}
	virtual bool compile(TIntermNode* root);
	TInfoSink infoSink;
	int debugOptions;
};

#endif
