// ====================================================================================
//
// Symbian GUI application template
//
// ------------------------------------------------------------------------------------
//
// 11-05-2004   Iwan Junianto       initial version
// ====================================================================================

#include <w32std.h>
#include <quartzkeys.h>
#include "appview.h"

extern int InitOpenGL(CWindowGc* gc, RWindow* win);
extern void ShutdownOpenGL();
extern void PaintProc(CWindowGc* gc, const TRect& aRect);
extern float angle;

_LIT(KBitmapFile, "z:\\system\\apps\\test\\dodge.mbm");

CAppView* CAppView::NewL(const TRect& aRect)
{
	CAppView* self = new (ELeave) CAppView;
	CleanupStack::PushL(self);
	self->ConstructL(aRect);
	CleanupStack::Pop();
	
	return self;
}

void CAppView::ConstructL(const TRect& aRect)
{
	CreateWindowL();
	SetRect(aRect);
	ActivateL();
    InitOpenGL(Gc(), Win());
    iCallBack = new (ELeave) TCallBack(&CAppView::OnTimer, this);
}

void CAppView::Exit()
{
    delete iCallBack;
    delete iTimer;
    iTimer = NULL;
    ShutdownOpenGL();
}

void CAppView::Draw(const TRect& aRect) const
{
    CWindowGc& gc = SystemGc();

    if (iShowBitmap)
    {
        CFbsBitmap* bitmap = NULL;
        TRAPD(err, bitmap = new (ELeave) CFbsBitmap);
        if (err!=KErrNone)
            return;

        err = bitmap->Load(KBitmapFile, 0);
        gc.BitBlt(TPoint(0,0), bitmap, aRect);
        delete bitmap;

        return;
    }

    if (iShowTest)
    {
        PaintProc(&gc, aRect);
        return;
    }

    gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
    gc.SetBrushColor(KRgbWhite);
    gc.DrawRect(aRect);
}

void CAppView::ShowBitmap()
{
    delete iTimer;
    iTimer = NULL;
    iShowTest = EFalse;
    iShowBitmap = ETrue;
    DrawNow();
}

void CAppView::ShowTest()
{
    iShowBitmap = EFalse;
    iShowTest = ETrue;
    iTimer = CPeriodic::NewL(0);
    // 40ms timer
    iTimer->Start(TTimeIntervalMicroSeconds32(0),TTimeIntervalMicroSeconds32(40000),*iCallBack);
}

CWindowGc* CAppView::Gc() const
{
    return &SystemGc();
}

RWindow* CAppView::Win() const
{
    return &Window();
}

TCoeInputCapabilities CAppView::InputCapabilities() const
{
	return TCoeInputCapabilities(TCoeInputCapabilities::ENavigation);
}

TKeyResponse CAppView::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
{
	if (aType!=EEventKey)
		return EKeyWasNotConsumed;
		
	switch (aKeyEvent.iCode) {
	case EQuartzKeyFourWayLeft:
		break;
		
	case EQuartzKeyFourWayRight:
        break;

	case EQuartzKeyFourWayUp:
		break;
		
	case EQuartzKeyFourWayDown:
		break;
		
	case EKeyYes:
		break;
		
	case EKeyNo:
		break;
	
	default:
		return EKeyWasNotConsumed;
	}
	
	return EKeyWasConsumed;
}

void CAppView::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
	if (aPointerEvent.iType==TPointerEvent::EButton1Down) 
	{
	}
}

int CAppView::OnTimer(void* aArg)
{
    CAppView* self = (CAppView*)aArg;
    angle += 0.5f;
    self->DrawNow();
    return 0;
}