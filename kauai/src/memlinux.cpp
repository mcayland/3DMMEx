/* Copyright (c) Mark Cave-Ayland.
   Licensed under the MIT License. */

/***************************************************************************
    Author: Mark Cave-Ayland
    Project: Kauai
    Reviewed:
    Copyright (c) Mark Cave-Ayland

    Linux memory management.

***************************************************************************/
#include "util.h"
ASSERTNAME

struct HQH
{
    int32_t cb;       // size of client area
    int32_t cactLock; // lock count
#ifdef DEBUG
    int32_t lwMagic; // for detecting memory trashing
#endif               // DEBUG
};

#ifdef DEBUG
int32_t vcactSuspendCheckPointers = 0;
#endif // DEBUG

/***************************************************************************
    Allocates a new moveable block.
***************************************************************************/
#ifdef DEBUG
bool FAllocHqDebug(HQ *phq, int32_t cb, uint32_t grfmem, int32_t mpr, PSZS pszsFile, int32_t lwLine)
#else  //! DEBUG
bool FAllocHq(HQ *phq, int32_t cb, uint32_t grfmem, int32_t mpr)
#endif //! DEBUG
{
    AssertVarMem(phq);
    AssertIn(cb, 0, kcbMax);
    HQH *phqh;

    if (!FAllocPvDebug((void **)&phqh, cb + SIZEOF(HQH), grfmem, mpr, pszsFile, lwLine, &vdmglob.dmaglHq))
    {
        PushErc(ercOomHq);
        *phq = hqNil;
    }
    else
    {
        phqh->cb = cb;
        phqh->cactLock = 0;
        Debug(phqh->lwMagic = klwMagicMem;) *phq = (HQ)(phqh + 1);
        AssertHq(*phq);
    }

    return pvNil != *phq;
}

/***************************************************************************
    Resizes the given hq.  *phq may change.  If fmemClear, clears any
    newly added space.
***************************************************************************/
bool FResizePhq(HQ *phq, int32_t cb, uint32_t grfmem, int32_t mpr)
{
    AssertVarMem(phq);
    AssertHq(*phq);
    AssertIn(cb, 0, kcbMax);
    bool fRet = fFalse;
    HQH *phqh = (HQH *)PvSubBv(*phq, SIZEOF(HQH));

    if (phqh->cactLock > 0)
    {
        Bug("Resizing locked HQ");
        PushErc(ercOomHq);
    }
#ifdef DEBUG
#ifdef WIN
    else if (!_FResizePpvDebug((void **)&phqh, cb + SIZEOF(HQH), phqh->cb + SIZEOF(HQH), grfmem, mpr, NULL))
    {
        PushErc(ercOomHq);
        AssertHq(*phq);
    }
#endif
#endif
    else
    {
        phqh->cb = cb;
        *phq = (HQ)(phqh + 1);
        AssertHq(*phq);
        fRet = fTrue;
    }

    return fRet;
}

/***************************************************************************
    If hq is not nil, frees it.
***************************************************************************/
void FreePhq(HQ *phq)
{
    AssertVarMem(phq);

    if (*phq == hqNil)
        return;

    AssertHq(*phq);
    HQH *phqh = (HQH *)PvSubBv(*phq, SIZEOF(HQH));
    *phq = hqNil;
    Assert(phqh->cactLock == 0, "Freeing locked HQ");

    FreePpvDebug((void **)&phqh, &vdmglob.dmaglHq);
}

/***************************************************************************
    Create a new HQ the same size as hqSrc and copy hqSrc into it.
***************************************************************************/
bool FCopyHq(HQ hqSrc, HQ *phqDst, long mpr)
{
    AssertHq(hqSrc);
    AssertVarMem(phqDst);
    long cb;

    if (!FAllocHq(phqDst, cb = CbOfHq(hqSrc), fmemNil, mpr))
        return fFalse;
    CopyPb(QvFromHq(hqSrc), QvFromHq(*phqDst), cb);
    return fTrue;
}

/***************************************************************************
    Return the size of the hq (the client area of the block).
***************************************************************************/
int32_t CbOfHq(HQ hq)
{
    AssertHq(hq);
    HQH *phqh = (HQH *)PvSubBv(hq, SIZEOF(HQH));
    return phqh->cb;
}

#ifdef DEBUG
/***************************************************************************
    Returns a volatile pointer from an hq.
***************************************************************************/
void *QvFromHq(HQ hq)
{
    AssertHq(hq);
    return (void *)hq;
}
#endif // DEBUG

/***************************************************************************
    Lock the hq and return a pointer to the data.
***************************************************************************/
void *PvLockHq(HQ hq)
{
    AssertHq(hq);
    HQH *phqh = (HQH *)PvSubBv(hq, SIZEOF(HQH));
    phqh->cactLock++;
    Assert(phqh->cactLock > 0, "overflow in cactLock");
    return (void *)hq;
}

/***************************************************************************
    Unlock the hq.  Asserts and does nothing if the lock count is zero.
***************************************************************************/
void UnlockHq(HQ hq)
{
    AssertHq(hq);
    HQH *phqh = (HQH *)PvSubBv(hq, SIZEOF(HQH));
    Assert(phqh->cactLock > 0, "hq not locked");
    if (phqh->cactLock > 0)
        --phqh->cactLock;
}

#ifdef DEBUG
/***************************************************************************
    Assert that a given hq is valid.
***************************************************************************/
void AssertHq(HQ hq)
{
    // make sure hq isn't nil
    if (hq == hqNil)
    {
        Bug("hq is nil");
        return;
    }

    // verify the HQH
    HQH *phqh = (HQH *)PvSubBv(hq, SIZEOF(HQH));
    if (phqh->lwMagic != klwMagicMem)
    {
        BugVar("beginning of hq block is trashed", phqh);
        return;
    }
    AssertIn(phqh->cactLock, 0, kcbMax);
    AssertPvAlloced(phqh, phqh->cb + SIZEOF(HQH));
}

/***************************************************************************
    Increment the ref count on an hq.
***************************************************************************/
void MarkHq(HQ hq)
{
    if (hqNil != hq)
    {
        AssertHq(hq);
        MarkPv(PvSubBv(hq, SIZEOF(HQH)));
    }
}

/***************************************************************************
    Make sure we can access a pointer's memory.  If cb is 0, pv can be
    anything (including nil).
***************************************************************************/
void AssertPvCb(const void *pv, int32_t cb)
{
    if (vcactSuspendCheckPointers == 0 && cb != 0)
    {
        // This assert has been disabled because AssertPvCb is called on pointers to
        // globals which were previously read/write but are now read-only.

        // AssertVar(!IsBadWritePtr(pv, cb), "no write access to ptr", &pv);
        //  I (ShonK) am assuming that write access implies read access for
        //  memory, so it would just be a waste of time to call this.
        //  AssertVar(!IsBadReadPtr(pv, cb), "no read access to ptr", &pv);
    }
}
#endif // DEBUG
