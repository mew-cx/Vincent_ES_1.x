//==============================================================================
//
// Symbian version of UG helper library
//
//------------------------------------------------------------------------------
//
// 12-05-2004	Janusz Gregorczyk	initial version
//
//==============================================================================

#include <e32base.h>
#include <estlib.h>
#include <w32std.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLES/egl.h>

#include "ug.h"

typedef struct {
    void (*idle)(UGWindow w);

    RWsSession wsSession;
    CWsScreenDevice* wsScreenDevice;
    CWindowGc* wsWindowGc; // NativeDisplayType

    EGLDisplay egldisplay;

    //TODO proxi: dummy impl, need to come up with something more generic
    struct ugwindow* win;
} UGCtx_t;

typedef struct ugwindow {
    void (*draw)(UGWindow w);
    void (*reshape)(UGWindow w, int width,int height);
    void (*kbd)(UGWindow w, int key, int x, int y);
    void (*pointer)(UGWindow w, int button, int state, int x, int y);
    void (*motion)(UGWindow w, int x, int y);

    UGCtx_t* ug;

    RWindowGroup wsWindowGroup;
    RWindow wsWindow;

    EGLConfig eglconfig;
    EGLContext eglcontext;
    EGLSurface eglsurface;
} UGWindow_t;

UGCtx APIENTRY
ugInit(void) {
    UGCtx_t* ug = reinterpret_cast<UGCtx_t*>(malloc(sizeof *ug));
    if (ug) {
        memset(ug, 0, sizeof *ug);

        //TODO proxi: error handling
	TInt error = ug->wsSession.Connect();
        ug->wsScreenDevice = new(ELeave) CWsScreenDevice(ug->wsSession);
	ug->wsScreenDevice->Construct();
	ug->wsScreenDevice->CreateContext(ug->wsWindowGc);

        ug->egldisplay = eglGetDisplay(ug->wsWindowGc);
        if (!eglInitialize(ug->egldisplay, NULL, NULL)) {
            return 0;
        }
    }

    return (UGCtx)ug;
}

void APIENTRY
ugFini(UGCtx ug) {
    UGCtx_t* _ug = reinterpret_cast<UGCtx_t*>(ug);
    eglTerminate(_ug->egldisplay);

    delete _ug->wsWindowGc;
    delete _ug->wsScreenDevice;
    if (_ug->wsSession.WsHandle()) {
        _ug->wsSession.Close();
    }

    free(_ug);
}

UGCtx APIENTRY ugCtxFromWin(UGWindow uwin) {
    UGWindow_t* _uwin = reinterpret_cast<UGWindow_t*>(uwin);
    return (UGCtx)(_uwin->ug);
}

UGWindow APIENTRY
ugCreateWindow(UGCtx ug, const char* config,
               const char* title, int width, int height, int x, int y) {
    UGWindow_t *w = reinterpret_cast<UGWindow_t*>(malloc(sizeof *w));
    UGCtx_t *_ug = reinterpret_cast<UGCtx_t*>(ug);

    if (w) {
        w->ug = _ug;

        _ug->win = w;

        //TODO proxi: handle errors
        //TODO proxi: think of multiple UG windows
        w->wsWindowGroup = RWindowGroup(_ug->wsSession);
	w->wsWindowGroup.Construct(reinterpret_cast<TInt>(&w->wsWindowGroup));

	w->wsWindow = RWindow(_ug->wsSession);
	w->wsWindow.Construct(w->wsWindowGroup, reinterpret_cast<TInt>(&w->wsWindow));
	w->wsWindow.SetSize(_ug->wsScreenDevice->SizeInPixels()); 
	w->wsWindow.SetVisible(ETrue);
        w->wsWindow.Activate();

        //TODO proxi: find out what is expected behaviour and implement it
        EGLConfig configs[1];
	EGLint matchingConfigs;
	EGLint attribList[] = { 0 };
	eglChooseConfig(_ug->egldisplay, attribList, &configs[0], 1, &matchingConfigs);
	w->eglconfig = configs[0];

	w->eglcontext = eglCreateContext(_ug->egldisplay, w->eglconfig, NULL, NULL);
	w->eglsurface = eglCreateWindowSurface(_ug->egldisplay, w->eglconfig, reinterpret_cast<NativeWindowType>(&(w->wsWindow)), 0);
        eglMakeCurrent(_ug->egldisplay, w->eglsurface, w->eglsurface, w->eglcontext);
    }

    return reinterpret_cast<UGWindow>(w);
}

void APIENTRY
ugDestroyWindow(UGWindow uwin) {
    UGWindow_t* _uwin = reinterpret_cast<UGWindow_t*>(uwin);

    eglDestroySurface(_uwin->ug, _uwin->eglsurface);
    eglDestroyContext(_uwin->ug, _uwin->eglcontext);

    _uwin->wsWindow.Close();
    _uwin->wsWindowGroup.Close();

    free(_uwin);
}

void APIENTRY
ugReshapeFunc(UGWindow uwin, void (*f)(UGWindow uwin, int width, int height)) {
    UGWindow_t* _uwin = reinterpret_cast<UGWindow_t*>(uwin);
    _uwin->reshape = f;
}

void APIENTRY
ugDisplayFunc(UGWindow uwin, void (*f)(UGWindow uwin)) {
    UGWindow_t* _uwin = reinterpret_cast<UGWindow_t*>(uwin);
    _uwin->draw = f;
}

void APIENTRY
ugKeyboardFunc(UGWindow uwin, void (*f)(UGWindow uwin, int key, int x, int y)) {
    UGWindow_t* _uwin = reinterpret_cast<UGWindow_t*>(uwin);
    _uwin->kbd = f;
}

void APIENTRY
ugPointerFunc(UGWindow uwin, void (*f)(UGWindow uwin, int button, int state, int x, int y)) {
    UGWindow_t* _uwin = reinterpret_cast<UGWindow_t*>(uwin);
    _uwin->pointer = f;
}

void APIENTRY
ugMotionFunc(UGWindow uwin, void (*f)(UGWindow uwin, int x, int y)) {
    UGWindow_t* _uwin = reinterpret_cast<UGWindow_t*>(uwin);
    _uwin->motion = f;
}

void APIENTRY
ugIdleFunc(UGCtx ug, void (*f)(UGWindow w)) {
    UGCtx_t* _ug = reinterpret_cast<UGCtx_t*>(ug);
    _ug->idle = f;
}

//TODO proxi: rewrite from scratch, handle wserv and input events,
//add missing parts, think of threading issues, ...
void APIENTRY
ugMainLoop(UGCtx ug) {
    UGCtx_t* _ug = reinterpret_cast<UGCtx_t*>(ug);

    TRequestStatus aStat = KRequestPending;
    _ug->wsSession.RedrawReady(&aStat);

    for (;;) {
        if (aStat != KRequestPending) {
            UGWindow_t* w = reinterpret_cast<UGWindow_t*>(_ug->win);
            CWindowGc* gc = _ug->wsWindowGc;

            TRect rect = TRect(w->wsWindow.Size());

            TWsRedrawEvent redrawEvent;
            _ug->wsSession.GetRedraw(redrawEvent);

            gc->Activate(w->wsWindow);
            w->wsWindow.BeginRedraw(rect);

            if (w->draw) {
                if (w->reshape) {
                    (w->reshape)((UGWindow)w, rect.Width(), rect.Height());
                }

                (w->draw)((UGWindow)w);
            }

            w->wsWindow.EndRedraw();
            gc->Deactivate();

            aStat = KRequestPending;
            _ug->wsSession.RedrawReady(&aStat);
	}
    }
}

void APIENTRY
ugPostRedisplay(UGWindow uwin) {
    //TODO proxi: implement
}

void APIENTRY
ugSwapBuffers(UGWindow uwin) {
    UGWindow_t* _uwin = reinterpret_cast<UGWindow_t*>(uwin);
    eglSwapBuffers(_uwin->ug->egldisplay, _uwin->eglsurface);
}

extern "C" int main(int argc, const char* argv[]);

//TODO proxi: add debugging macros, handle panics
TInt E32Main()
{
    CTrapCleanup* cleanup = CTrapCleanup::New();

#if defined(__WINS__) && !defined(EXEDLL)
    // Note: By default window server is unavailable for console applications
    // in WINS environment. An undocumented trick found in Epoc port of SDL
    // library by Hannu Viitala (http://koti.mbnet.fi/~haviital/) is used here.
    // Another alternative is to use exedll target type.

    IMPORT_C void RegisterWsExe(const TDesC& aName);
    RSemaphore sem;
    sem.CreateGlobal(_L("WsExeSem"), 0);
    RegisterWsExe(sem.FullName());
#endif // defined(__WINS__) && !defined(EXEDLL)

    int argc = 0;
    char** argv = 0;
    char** envp = 0;
    __crt0(argc, argv, envp);	
    TRAPD(err, main(argc, const_cast<const char**>(argv)));

    CloseSTDLIB();

    delete cleanup;

    return KErrNone;
}

#if defined(__WINS__) && defined(EXEDLL)

EXPORT_C TInt InitEmulator()
{
    return E32Main();
}

TInt E32Dll(TDllReason)
{
    return KErrNone;
}

#endif // defined(__WINS__) && defined(EXEDLL)
