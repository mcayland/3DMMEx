/***************************************************************************
    Author:
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Entry points for a Kauai GUI application

***************************************************************************/

#include "frame.h"

ASSERTNAME

extern WIG vwig;

/***************************************************************************
    WinMain for any frame work app. Sets up vwig and calls FrameMain.
***************************************************************************/
int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hinstPrev, LPSTR pszs, int wShow)
{
    vwig.hinst = hinst;
    vwig.hinstPrev = hinstPrev;
    vwig.pszCmdLine = GetCommandLine();
    vwig.wShow = wShow;
    vwig.lwThreadMain = LwThreadCur();
#ifdef DEBUG
    APPB::CreateConsole();
#endif
    FrameMain();
    return 0;
}

#ifdef DEBUG

/***************************************************************************
    Assert proc - just calls the app's AssertProc.
***************************************************************************/
bool FAssertProc(PSZS pszsFile, int32_t lwLine, PSZS pszsMsg, void *pv, int32_t cb)
{
    if (vpappb == pvNil)
        return fTrue;
    return vpappb->FAssertProcApp(pszsFile, lwLine, pszsMsg, pv, cb);
}

/***************************************************************************
    Warning reporting proc.
***************************************************************************/
void WarnProc(PSZS pszsFile, int32_t lwLine, PSZS pszsMsg)
{
    if (vpappb == pvNil)
        Debugger();
    else
        vpappb->WarnProcApp(pszsFile, lwLine, pszsMsg);
}

#endif // DEBUG