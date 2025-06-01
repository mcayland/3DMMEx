/***************************************************************************
    Author:
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Debug assert procedures for Kauai GUI applications

***************************************************************************/

#include "frame.h"

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