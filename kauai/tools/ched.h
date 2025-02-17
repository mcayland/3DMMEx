/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************

    Chunky editor main header file

***************************************************************************/
#ifndef CHED_H
#define CHED_H

#include "kidframe.h"
#include "chedres.h"

// creator type for the chunky editor
#define kctgChed 'CHED'

#include "chdoc.h"

#define APP_PAR APPB
#define kclsAPP 'APP'
class APP : public APP_PAR
{
    RTCLASS_DEC
    CMD_MAP_DEC(APP)

  protected:
    virtual bool _FInit(uint32_t grfapp, uint32_t grfgob, int32_t ginDef);
    virtual void _FastUpdate(PGOB pgob, PREGN pregnClip, uint32_t grfapp = fappNil, PGPT pgpt = pvNil);

  public:
    virtual void GetStnAppName(PSTN pstn);
    virtual void UpdateHwnd(HWND hwnd, RC *prc, uint32_t grfapp = fappNil);

    virtual bool FCmdOpen(PCMD pcmd);
};

#endif //! CHED_H
