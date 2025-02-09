/* Copyright (c) Mark Cave-Ayland.
   Licensed under the MIT License. */

/***************************************************************************
    Author: Mark Cave-Ayland
    Project: Kauai
    Reviewed:
    Copyright (c) Mark Cave-Ayland

    Linux specific file management.

***************************************************************************/
#include "util.h"
#include <cassert>
ASSERTNAME

const ulong kfpError = 0xFFFFFFFF;

/***************************************************************************
    Open or create the file.  If the file is already open, sets the
    permissions according to grffil.
***************************************************************************/
bool FIL::_FOpen(bool fCreate, uint32_t grffil)
{
    AssertBaseThis(0);
    bool fRet = fFalse;

    _mutx.Enter();
    _mutx.Leave();

    assert(0);
    return fRet;
}

/***************************************************************************
    Close the file.
***************************************************************************/
void FIL::_Close(bool fFinal)
{
    AssertBaseThis(0);

    _mutx.Enter();
    _mutx.Leave();
    assert(0);
}

/***************************************************************************
    Flush the file (and its volume?).
***************************************************************************/
void FIL::Flush(void)
{
    AssertThis(0);

    _mutx.Enter();
    _mutx.Leave();
    assert(0);
}

/***************************************************************************
    Seek to the given fp - assumes the mutx is already entered.
***************************************************************************/
void FIL::_SetFpPos(FP fp)
{
    AssertThis(0);

    if (!_fOpen)
        _FOpen(fFalse, _grffil);

    assert(0);
}

/***************************************************************************
    Set the length of the file.  This doesn't zero the appended portion.
***************************************************************************/
bool FIL::FSetFpMac(FP fp)
{
    AssertThis(0);
    AssertIn(fp, 0, kcbMax);
    bool fRet;

    _mutx.Enter();

    Assert(_grffil & ffilWriteEnable, "can't write to read only file");
    _SetFpPos(fp);

    _mutx.Leave();

    assert(0);
    return fRet;
}

/***************************************************************************
    Return the length of the file.
***************************************************************************/
FP FIL::FpMac(void)
{
    AssertThis(0);
    FP fp;

    _mutx.Enter();
    fp = 0;
    _mutx.Leave();

    assert(0);
    return fp;
}

/***************************************************************************
    Read a block from the file.
***************************************************************************/
bool FIL::FReadRgb(void *pv, int32_t cb, FP fp)
{
    AssertThis(0);
    AssertIn(cb, 0, kcbMax);
    AssertIn(fp, 0, klwMax);
    AssertPvCb(pv, cb);
    bool fRet = fFalse;

    assert(0);
    return fRet;
}

/***************************************************************************
    Write a block to the file.
***************************************************************************/
bool FIL::FWriteRgb(const void *pv, int32_t cb, FP fp)
{
    AssertThis(0);
    AssertIn(cb, 0, kcbMax);
    AssertIn(fp, 0, klwMax);
    AssertPvCb(pv, cb);
    bool fRet = fFalse;

    assert(0);
    return fRet;
}

/***************************************************************************
    Swap the names of the two files.  They should be in the same directory.
***************************************************************************/
bool FIL::FSwapNames(PFIL pfil)
{
    AssertThis(0);
    AssertPo(pfil, 0);
    bool fRet = fFalse;

    if (this == pfil)
    {
        Bug("Why are you calling FSwapNames on the same file?");
        return fTrue;
    }

    AssertThis(0);
    AssertPo(pfil, 0);

    assert(0);
    return fRet;
}

/***************************************************************************
    Rename a file.  The new fni should be on the same volume.
    This may fail without an error code being set.
***************************************************************************/
bool FIL::FRename(FNI *pfni)
{
    AssertThis(0);
    AssertPo(pfni, ffniFile);
    bool fRet = fFalse;

    assert(0);
    return fRet;
}
