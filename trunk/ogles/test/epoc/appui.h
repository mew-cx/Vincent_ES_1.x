// ====================================================================================
//
// Symbian GUI application template
//
// ------------------------------------------------------------------------------------
//
// 11-05-2004   Iwan Junianto       initial version
// ====================================================================================

#ifndef __APPUI_H__
#define __APPUI_H__

#include <eikappui.h>

class CAppView;
class CEikMenuPane;


class CAppUi: public CEikAppUi
{
public:
	void ConstructL();
	~CAppUi();
	
	void HandleCommandL(TInt aCmd);	

private:
	void DynInitMenuPaneL(TInt aMenuId, CEikMenuPane* aMenuPane);
	void ShowAboutDlgL();
	
	CAppView* iAppView;	// has

};

#endif