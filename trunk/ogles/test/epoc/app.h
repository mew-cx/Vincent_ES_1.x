// ====================================================================================
//
// Symbian GUI application template
//
// ------------------------------------------------------------------------------------
//
// 11-05-2004   Iwan Junianto       initial version
// ====================================================================================

#ifndef __APP_H__
#define __APP_H__

#include <eikapp.h>

class CApaDocument;

class CApp: public CEikApplication
{
public:
	TUid AppDllUid() const;
	
private:	
	CApaDocument* CreateDocumentL();
};

#endif