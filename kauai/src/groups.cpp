/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Basic collection classes:
        General List (GL), Allocated List (AL),
        General Group (GG), Allocated Group (AG),
        General String Table (GST), Allocated String Table (AST).

        BASE ---> GRPB -+-> GLB -+-> GL
                        |        +-> AL
                        |
                        +-> GGB -+-> GG
                        |        +-> AG
                        |
                        +-> GSTB-+-> GST
                                 +-> AST

***************************************************************************/
#include "util.h"
ASSERTNAME

RTCLASS(GRPB)
RTCLASS(GLB)
RTCLASS(GL)
RTCLASS(AL)
RTCLASS(GGB)
RTCLASS(GG)
RTCLASS(AG)

/***************************************************************************
    GRPB:  Manages two sections of data.  Currently the two sections are
    in two separate hq's, but they could be in one without affecting the
    clients.  The actual data in the two sections is determined by the
    subclass (client).  This class just manages resizing the data sections.
***************************************************************************/

/***************************************************************************
    Destructor for GRPB.  Frees the hq.
***************************************************************************/
GRPB::~GRPB(void)
{
    AssertThis(0);
    FreePhq(&_hqData1);
    FreePhq(&_hqData2);
}

/***************************************************************************
    Ensure that the two sections are at least the given cb's large.
    if (grfgrp & fgrpShrink), makes them exact.
***************************************************************************/
bool GRPB::_FEnsureSizes(int32_t cbMin1, int32_t cbMin2, uint32_t grfgrp)
{
    AssertThis(0);
    Assert(cbMin1 >= 0 && cbMin2 >= 0, "negative sizes");

    if (grfgrp & fgrpShrink)
    {
        // shrink anything that's too big
        if (cbMin1 == 0)
        {
            FreePhq(&_hqData1);
            _cb1 = 0;
        }
        else if (cbMin1 < _cb1)
        {
            FResizePhq(&_hqData1, cbMin1, fmemClear, mprNormal);
            _cb1 = cbMin1;
        }

        if (cbMin2 == 0)
        {
            FreePhq(&_hqData2);
            _cb2 = 0;
        }
        else if (cbMin2 < _cb2)
        {
            FResizePhq(&_hqData2, cbMin2, fmemClear, mprNormal);
            _cb2 = cbMin2;
        }
    }

    if (cbMin1 > _cb1 && !_FEnsureHqCb(&_hqData1, cbMin1, _cbMinGrow1, &_cb1))
        return fFalse;
    if (cbMin2 > _cb2 && !_FEnsureHqCb(&_hqData2, cbMin2, _cbMinGrow2, &_cb2))
        return fFalse;

    AssertThis(0);
    return fTrue;
}

/***************************************************************************
    Ensure that the given HQ is large enough.
***************************************************************************/
bool GRPB::_FEnsureHqCb(HQ *phq, int32_t cb, int32_t cbMinGrow, int32_t *pcb)
{
    AssertVarMem(phq);
    AssertIn(cbMinGrow, 0, kcbMax);
    AssertVarMem(pcb);
    AssertIn(*pcb, 0, kcbMax);

    // limit the size
    if ((uint32_t)cb >= kcbMax)
        return fFalse;

    AssertIn(cb, *pcb + 1, kcbMax);
    if (hqNil != *phq)
    {
        // resize an existing hq
        AssertHq(*phq);

        if ((cbMinGrow += *pcb) > cb && FResizePhq(phq, cbMinGrow, fmemClear, mprForSpeed))
        {
            *pcb = cbMinGrow;
            return fTrue;
        }
        else if (FResizePhq(phq, cb, fmemClear, mprNormal))
        {
            *pcb = cb;
            return fTrue;
        }
        return fFalse;
    }

    // just allocate the thing
    Assert(*pcb == 0, "bad cb");
    if (cbMinGrow > cb && FAllocHq(phq, cbMinGrow, fmemClear, mprForSpeed))
    {
        *pcb = cbMinGrow;
        return fTrue;
    }
    else if (FAllocHq(phq, cb, fmemClear, mprNormal))
    {
        *pcb = cb;
        return fTrue;
    }
    return fFalse;
}

/***************************************************************************
    Make the given GRPB a duplicate of this one.
***************************************************************************/
bool GRPB::_FDup(PGRPB pgrpbDst, int32_t cb1, int32_t cb2)
{
    AssertThis(0);
    AssertPo(pgrpbDst, 0);
    AssertIn(cb1, 0, kcbMax);
    AssertIn(cb2, 0, kcbMax);

    if (!pgrpbDst->_FEnsureSizes(cb1, cb2, fgrpShrink))
        return fFalse;

    if (cb1 > 0)
        CopyPb(_Qb1(0), pgrpbDst->_Qb1(0), cb1);
    if (cb2 > 0)
        CopyPb(_Qb2(0), pgrpbDst->_Qb2(0), cb2);

    pgrpbDst->_cbMinGrow1 = _cbMinGrow1;
    pgrpbDst->_cbMinGrow2 = _cbMinGrow2;
    pgrpbDst->_ivMac = _ivMac;

    AssertPo(pgrpbDst, 0);
    return fTrue;
}

/***************************************************************************
    Write a group to a flo.
***************************************************************************/
bool GRPB::FWriteFlo(PFLO pflo, int16_t bo, int16_t osk)
{
    BLCK blck(pflo);
    return FWrite(&blck, bo, osk);
}

/***************************************************************************
    Write the GRPB data to the block.  First write the (pv, cb), then
    cb1 bytes from the first section and cb2 bytes from the second.
***************************************************************************/
bool GRPB::_FWrite(PBLCK pblck, void *pv, int32_t cb, int32_t cb1, int32_t cb2)
{
    AssertPo(pblck, 0);
    AssertIn(cb, 1, kcbMax);
    AssertPvCb(pv, cb);
    AssertIn(cb1, 0, _cb1 + 1);
    AssertIn(cb2, 0, _cb2 + 1);

    bool fRet = fFalse;

    if (pblck->Cb() != cb + cb1 + cb2)
    {
        Bug("blck wrong size");
        return fFalse;
    }
    if (!pblck->FWriteRgb(pv, cb, 0))
        return fFalse;

    if (cb1 > 0)
    {
        fRet = pblck->FWriteRgb(PvLockHq(_hqData1), cb1, cb);
        UnlockHq(_hqData1);
        if (!fRet)
            return fFalse;
    }
    if (cb2 > 0)
    {
        fRet = pblck->FWriteRgb(PvLockHq(_hqData2), cb2, cb + cb1);
        UnlockHq(_hqData2);
        if (!fRet)
            return fFalse;
    }

    return fTrue;
}

/***************************************************************************
    Read the two sections of data from the given location in the given
    block.
***************************************************************************/
bool GRPB::_FReadData(PBLCK pblck, int32_t cb1, int32_t cb2, int32_t ib)
{
    AssertPo(pblck, fblckUnpacked);
    AssertIn(cb1, 0, kcbMax);
    AssertIn(cb2, 0, kcbMax);

    bool fRet;

    if (cb1 == 0 && cb2 == 0)
        return fTrue;
    if (!_FEnsureSizes(cb1, cb2, fgrpNil))
        return fFalse;

    if (cb1 > 0)
    {
        fRet = pblck->FReadRgb(PvLockHq(_hqData1), cb1, ib);
        UnlockHq(_hqData1);
        if (!fRet)
            return fFalse;
    }
    if (cb2 > 0)
    {
        fRet = pblck->FReadRgb(PvLockHq(_hqData2), cb2, ib + cb1);
        UnlockHq(_hqData2);
        if (!fRet)
            return fFalse;
    }

    return fTrue;
}

#ifdef DEBUG
/***************************************************************************
    Assert the validity of the grpb stuff.
***************************************************************************/
void GRPB::AssertValid(uint32_t grfobj)
{
    GRPB_PAR::AssertValid(grfobj | fobjAllocated);
    AssertIn(_cb1, 0, kcbMax);
    AssertIn(_cb2, 0, kcbMax);
    Assert((_cb1 == 0) == (_hqData1 == hqNil), "cb's don't match _hqData1");
    Assert((_cb2 == 0) == (_hqData2 == hqNil), "cb's don't match _hqData2");
    Assert(_hqData1 == hqNil || CbOfHq(_hqData1) == _cb1, "_hqData1 wrong size");
    Assert(_hqData2 == hqNil || CbOfHq(_hqData2) == _cb2, "_hqData2 wrong size");
}

/***************************************************************************
    Mark the _hqData blocks.
***************************************************************************/
void GRPB::MarkMem(void)
{
    AssertThis(0);
    GRPB_PAR::MarkMem();
    MarkHq(_hqData1);
    MarkHq(_hqData2);
}
#endif // DEBUG

/***************************************************************************
    GLB:  Base class for GL (general list) and AL (general allocated list).
    The list data goes in section 1.  The GL class doesn't use section 2.
    The AL class uses section 2 for a bit array indicating whether an entry
    is free or in use.
***************************************************************************/

/***************************************************************************
    Constructor for the list base.
***************************************************************************/
GLB::GLB(int32_t cb)
{
    AssertIn(cb, 0, kcbMax);
    _cbEntry = cb;

    // use some reasonable values for _cbMinGrow* - code can always set
    // set these to something else
    _cbMinGrow1 = 128;
    _cbMinGrow2 = 16;
    AssertThis(0);
}

/***************************************************************************
    Return a volatile pointer to a list entry.
    NOTE: don't assert !FFree(iv) for allocated lists.
***************************************************************************/
void *GLB::QvGet(int32_t iv)
{
    AssertThis(0);
    AssertIn(iv, 0, _ivMac + 1);
    return (0 == _ivMac) ? pvNil : _Qb1(LwMul(iv, _cbEntry));
}

/***************************************************************************
    Get the data for the iv'th element in the GLB.
***************************************************************************/
void GLB::Get(int32_t iv, void *pv)
{
    AssertThis(0);
    AssertIn(iv, 0, _ivMac);
    AssertPvCb(pv, _cbEntry);
    CopyPb(QvGet(iv), pv, _cbEntry);
}

/***************************************************************************
    Put data into the iv'th element in the GLB.
***************************************************************************/
void GLB::Put(int32_t iv, void *pv)
{
    AssertThis(0);
    AssertIn(iv, 0, _ivMac);
    AssertPvCb(pv, _cbEntry);
    CopyPb(pv, QvGet(iv), _cbEntry);
    AssertThis(0);
}

/***************************************************************************
    Lock the data and return a pointer to the ith item.
***************************************************************************/
void *GLB::PvLock(int32_t iv)
{
    Lock();
    return QvGet(iv);
}

/***************************************************************************
    Set the minimum that a GL should grow by.
***************************************************************************/
void GLB::SetMinGrow(int32_t cvAdd)
{
    AssertThis(0);
    AssertIn(cvAdd, 0, kcbMax);

    _cbMinGrow1 = CbRoundToLong(LwMul(cvAdd, _cbEntry));
    _cbMinGrow2 = CbRoundToLong(LwDivAway(cvAdd, 8));
}

#ifdef DEBUG
/***************************************************************************
    Assert the validity of a list (GL or AL).
***************************************************************************/
void GLB::AssertValid(uint32_t grfobj)
{
    GLB_PAR::AssertValid(grfobj);
    AssertIn(_cbEntry, 1, kcbMax);
    AssertIn(_ivMac, 0, kcbMax);
    Assert(_Cb1() >= LwMul(_cbEntry, _ivMac), "array area too small");
}
#endif // DEBUG

/***************************************************************************
    Allocate a new list and ensure that it has space for cvInit elements.
***************************************************************************/
PGL GL::PglNew(int32_t cb, int32_t cvInit)
{
    AssertIn(cb, 1, kcbMax);
    AssertIn(cvInit, 0, kcbMax);
    PGL pgl;

    if ((pgl = NewObj GL(cb)) == pvNil)
        return pvNil;
    if (cvInit > 0 && !pgl->FEnsureSpace(cvInit, fgrpNil))
    {
        ReleasePpo(&pgl);
        return pvNil;
    }
    AssertPo(pgl, 0);
    return pgl;
}

/***************************************************************************
    Read a list from a block and return it.
***************************************************************************/
PGL GL::PglRead(PBLCK pblck, int16_t *pbo, int16_t *posk)
{
    AssertPo(pblck, 0);
    AssertNilOrVarMem(pbo);
    AssertNilOrVarMem(posk);
    PGL pgl;

    /* the use of 4 for the cb is bogus, but _FRead overwrites the cb anyway */
    if ((pgl = NewObj GL(4)) == pvNil)
        goto LFail;
    if (!pgl->_FRead(pblck, pbo, posk))
    {
        ReleasePpo(&pgl);
    LFail:
        TrashVar(pbo);
        TrashVar(posk);
        return pvNil;
    }
    AssertPo(pgl, 0);
    return pgl;
}

/***************************************************************************
    Read a list from file and return it.
***************************************************************************/
PGL GL::PglRead(PFIL pfil, FP fp, int32_t cb, int16_t *pbo, int16_t *posk)
{
    BLCK blck(pfil, fp, cb);
    return PglRead(&blck, pbo, posk);
}

/***************************************************************************
    Constructor for GL.
***************************************************************************/
GL::GL(int32_t cb) : GLB(cb)
{
    AssertThis(0);
}

/***************************************************************************
    Provided for completeness (all GRPB's have an FFree routine).
    Returns false iff iv is a valid index for the GL.
***************************************************************************/
bool GL::FFree(int32_t iv)
{
    AssertThis(0);
    return !FIn(iv, 0, _ivMac);
}

/***************************************************************************
    Duplicate this GL.
***************************************************************************/
PGL GL::PglDup(void)
{
    AssertThis(0);
    PGL pgl;

    if (pvNil == (pgl = PglNew(_cbEntry)))
        return pvNil;

    if (!_FDup(pgl, LwMul(_ivMac, _cbEntry), 0))
        ReleasePpo(&pgl);

    AssertNilOrPo(pgl, 0);
    return pgl;
}

// List on file
struct GLF
{
    int16_t bo;
    int16_t osk;
    int32_t cbEntry;
    int32_t ivMac;
};
VERIFY_STRUCT_SIZE(GLF, 12);
const BOM kbomGlf = 0x5F000000L;

/***************************************************************************
    Return the amount of space on file needed for the list.
***************************************************************************/
int32_t GL::CbOnFile(void)
{
    AssertThis(0);
    return SIZEOF(GLF) + LwMul(_cbEntry, _ivMac);
}

/***************************************************************************
    Write the list to disk.
***************************************************************************/
bool GL::FWrite(PBLCK pblck, int16_t bo, int16_t osk)
{
    AssertThis(0);
    AssertPo(pblck, 0);
    Assert(kboCur == bo || kboOther == bo, "bad bo");
    AssertOsk(osk);

    GLF glf;

    glf.bo = kboCur;
    glf.osk = osk;
    glf.cbEntry = _cbEntry;
    glf.ivMac = _ivMac;
    if (kboOther == bo)
    {
        SwapBytesBom(&glf, kbomGlf);
        Assert(glf.bo == bo, "wrong bo");
        Assert(glf.osk == osk, "osk not invariant under byte swapping");
    }
    return _FWrite(pblck, &glf, SIZEOF(glf), LwMul(_cbEntry, _ivMac), 0);
}

/***************************************************************************
    Read list data from disk.
***************************************************************************/
bool GL::_FRead(PBLCK pblck, int16_t *pbo, int16_t *posk)
{
    AssertThis(0);
    AssertPo(pblck, 0);
    AssertNilOrVarMem(pbo);
    AssertNilOrVarMem(posk);

    GLF glf;
    int32_t cb;
    bool fRet = fFalse;

    if (!pblck->FUnpackData())
        goto LFail;

    cb = pblck->Cb();
    if (cb < SIZEOF(glf))
        goto LBug;

    if (!pblck->FReadRgb(&glf, SIZEOF(glf), 0))
        goto LFail;

    if (pbo != pvNil)
        *pbo = glf.bo;
    if (posk != pvNil)
        *posk = glf.osk;

    if (glf.bo == kboOther)
        SwapBytesBom(&glf, kbomGlf);

    cb -= SIZEOF(glf);
    if (glf.bo != kboCur || glf.cbEntry <= 0 || glf.ivMac < 0 || cb != glf.cbEntry * glf.ivMac)
    {
    LBug:
        Warn("file corrupt or not a GL");
        goto LFail;
    }

    _cbEntry = glf.cbEntry;
    _ivMac = glf.ivMac;
    fRet = _FReadData(pblck, cb, 0, SIZEOF(glf));

LFail:
    TrashVarIf(!fRet, pbo);
    TrashVarIf(!fRet, posk);
    return fRet;
}

/***************************************************************************
    Insert some items into a list at position iv.  iv should be <= IvMac().
***************************************************************************/
bool GL::FInsert(int32_t iv, void *pv, int32_t cv)
{
    AssertThis(0);
    AssertIn(iv, 0, _ivMac + 1);
    AssertIn(cv, 1, kcbMax);
    AssertNilOrPvCb(pv, LwMul(cv, _cbEntry));

    uint8_t *qb;
    int32_t cbTot, cbIns, ibIns;

    cbTot = LwMul(_ivMac + cv, _cbEntry);
    cbIns = LwMul(cv, _cbEntry);
    ibIns = LwMul(iv, _cbEntry);
    if (cbTot > _Cb1() && !_FEnsureSizes(cbTot, 0, fgrpNil))
        return fFalse;

    qb = _Qb1(ibIns);
    if (iv < _ivMac)
        BltPb(qb, qb + cbIns, cbTot - cbIns - ibIns);
    if (pvNil != pv)
        CopyPb(pv, qb, cbIns);
    _ivMac += cv;

    AssertThis(0);
    return fTrue;
}

/***************************************************************************
    Delete an element from the list.  This changes the indices of all
    later elements.
***************************************************************************/
void GL::Delete(int32_t iv)
{
    AssertThis(0);
    Delete(iv, 1);
}

/***************************************************************************
    Delete a range of elements.  This changes the indices of all later
    elements.
***************************************************************************/
void GL::Delete(int32_t ivMin, int32_t cv)
{
    AssertThis(0);
    AssertIn(ivMin, 0, _ivMac);
    AssertIn(cv, 1, _ivMac - ivMin + 1);

    if (ivMin < (_ivMac -= cv))
    {
        uint8_t *qb = _Qb1(LwMul(ivMin, _cbEntry));
        BltPb(qb + LwMul(cv, _cbEntry), qb, LwMul(_ivMac - ivMin, _cbEntry));
    }
    TrashPvCb(_Qb1(LwMul(_ivMac, _cbEntry)), LwMul(cv, _cbEntry));
    AssertThis(0);
}

/***************************************************************************
    Move the entry at ivSrc to be immediately before the element that is
    currently at ivTarget.  If ivTarget > ivSrc, the entry actually ends
    up at (ivTarget - 1) and the entry at ivTarget doesn't move.  If
    ivTarget < ivSrc, the entry ends up at ivTarget and the entry at
    ivTarget moves to (ivTarget + 1).  Everything in between is shifted
    appropriately.  ivTarget is allowed to be equal to IvMac().
***************************************************************************/
void GL::Move(int32_t ivSrc, int32_t ivTarget)
{
    AssertThis(0);
    AssertIn(ivSrc, 0, _ivMac);
    AssertIn(ivTarget, 0, _ivMac + 1);

    MoveElement(_Qb1(0), _cbEntry, ivSrc, ivTarget);
    AssertThis(0);
}

/***************************************************************************
    Add an element to the end of the list.  Returns the location in *piv.
    On failure, returns false and *piv is undefined.
***************************************************************************/
bool GL::FAdd(void *pv, int32_t *piv)
{
    AssertThis(0);
    AssertNilOrVarMem(piv);

    if (piv != pvNil)
        *piv = _ivMac;
    if (!FInsert(_ivMac, pv))
    {
        TrashVar(piv);
        return fFalse;
    }
    return fTrue;
}

/***************************************************************************
    Stack operation.  Returns fFalse on stack underflow.
***************************************************************************/
bool GL::FPop(void *pv)
{
    AssertThis(0);
    AssertNilOrPvCb(pv, _cbEntry);

    if (_ivMac == 0)
    {
        TrashPvCb(pv, _cbEntry);
        return fFalse;
    }
    if (pv != pvNil)
        Get(_ivMac - 1, pv);
    _ivMac--;
    TrashPvCb(_Qb1(LwMul(_ivMac, _cbEntry)), _cbEntry);
    return fTrue;
}

/***************************************************************************
    Set the number of elements.  Used rarely (to add a block of elements
    at a time or to "zero out" a list.
***************************************************************************/
bool GL::FSetIvMac(int32_t ivMacNew)
{
    AssertThis(0);
    AssertIn(ivMacNew, 0, kcbMax);
    int32_t cb;

    if (ivMacNew > _ivMac)
    {
        if (ivMacNew > kcbMax / _cbEntry)
        {
            Bug("who's trying to allocate a list this big?");
            return fFalse;
        }

        cb = LwMul(ivMacNew, _cbEntry);
        if (cb > _Cb1() && !_FEnsureSizes(cb, 0, fgrpNil))
            return fFalse;
        TrashPvCb(_Qb1(LwMul(_ivMac, _cbEntry)), LwMul(ivMacNew - _ivMac, _cbEntry));
    }
#ifdef DEBUG
    else if (ivMacNew < _ivMac)
    {
        TrashPvCb(_Qb1(LwMul(ivMacNew, _cbEntry)), LwMul(_ivMac - ivMacNew, _cbEntry));
    }
#endif // DEBUG

    _ivMac = ivMacNew;

    AssertThis(0);
    return fTrue;
}

/***************************************************************************
    Make sure there is room for at least cvAdd additional entries. If
    fgrpShrink is set, will shrink the list if it has more than cvAdd
    available entries.
***************************************************************************/
bool GL::FEnsureSpace(int32_t cvAdd, uint32_t grfgrp)
{
    AssertThis(0);
    AssertIn(cvAdd, 0, kcbMax);

    // limit the size of the list
    if (cvAdd > kcbMax / _cbEntry - _ivMac)
    {
        Bug("who's trying to allocate a list this big?");
        return fFalse;
    }

    return _FEnsureSizes(LwMul(cvAdd + _ivMac, _cbEntry), 0, grfgrp);
}

/***************************************************************************
    Allocate a new allocated list and ensure that it has space for
    cvInit elements.
***************************************************************************/
PAL AL::PalNew(int32_t cb, int32_t cvInit)
{
    AssertIn(cb, 1, kcbMax);
    AssertIn(cvInit, 0, kcbMax);
    PAL pal;

    if ((pal = NewObj AL(cb)) == pvNil)
        return pvNil;
    if (cvInit > 0 && !pal->FEnsureSpace(cvInit, fgrpNil))
    {
        ReleasePpo(&pal);
        return pvNil;
    }
    AssertPo(pal, 0);
    return pal;
}

/***************************************************************************
    Read an allocated list from the block and return it.
***************************************************************************/
PAL AL::PalRead(PBLCK pblck, int16_t *pbo, int16_t *posk)
{
    AssertPo(pblck, 0);
    AssertNilOrVarMem(pbo);
    AssertNilOrVarMem(posk);

    PAL pal;

    /* the use of 4 for the cb is bogus, but _FRead overwrites the cb anyway */
    if ((pal = NewObj AL(4)) == pvNil)
        goto LFail;
    if (!pal->_FRead(pblck, pbo, posk))
    {
        ReleasePpo(&pal);
    LFail:
        TrashVar(pbo);
        TrashVar(posk);
        return pvNil;
    }
    AssertPo(pal, 0);
    return pal;
}

/***************************************************************************
    Read an allocated list from file and return it.
***************************************************************************/
PAL AL::PalRead(PFIL pfil, FP fp, int32_t cb, int16_t *pbo, int16_t *posk)
{
    BLCK blck(pfil, fp, cb);
    return PalRead(&blck, pbo, posk);
}

/***************************************************************************
    Constructor for AL (allocated list) class.
***************************************************************************/
AL::AL(int32_t cb) : GLB(cb)
{
    AssertThis(fobjAssertFull);
}

/***************************************************************************
    Duplicate this AL.
***************************************************************************/
PAL AL::PalDup(void)
{
    AssertThis(fobjAssertFull);
    PAL pal;

    if (pvNil == (pal = PalNew(_cbEntry)))
        return pvNil;

    if (!_FDup(pal, LwMul(_ivMac, _cbEntry), CbFromCbit(_ivMac)))
        ReleasePpo(&pal);
    else
        pal->_cvFree = _cvFree;

    AssertNilOrPo(pal, fobjAssertFull);
    return pal;
}

// Allocated list on file
struct ALF
{
    int16_t bo;
    int16_t osk;
    int32_t cbEntry;
    int32_t ivMac;
    int32_t cvFree;
};
VERIFY_STRUCT_SIZE(ALF, 16);
const BOM kbomAlf = 0x5FC00000L;

/***************************************************************************
    Return the amount of space on file needed for the list.
***************************************************************************/
int32_t AL::CbOnFile(void)
{
    AssertThis(fobjAssertFull);

    return SIZEOF(ALF) + LwMul(_cbEntry, _ivMac) + CbFromCbit(_ivMac);
}

/***************************************************************************
    Write the list to disk.
***************************************************************************/
bool AL::FWrite(PBLCK pblck, int16_t bo, int16_t osk)
{
    AssertThis(fobjAssertFull);
    AssertPo(pblck, 0);
    Assert(kboCur == bo || kboOther == bo, "bad bo");
    AssertOsk(osk);

    ALF alf;

    alf.bo = kboCur;
    alf.osk = osk;
    alf.cbEntry = _cbEntry;
    alf.ivMac = _ivMac;
    alf.cvFree = _cvFree;
    if (kboOther == bo)
    {
        SwapBytesBom(&alf, kbomAlf);
        Assert(alf.bo == bo, "wrong bo");
        Assert(alf.osk == osk, "osk not invariant under byte swapping");
    }
    return _FWrite(pblck, &alf, SIZEOF(alf), LwMul(_cbEntry, _ivMac), CbFromCbit(_ivMac));
}

/***************************************************************************
    Read allocated list data from the block.
***************************************************************************/
bool AL::_FRead(PBLCK pblck, int16_t *pbo, int16_t *posk)
{
    AssertThis(0);
    AssertPo(pblck, 0);
    AssertNilOrVarMem(pbo);
    AssertNilOrVarMem(posk);

    ALF alf;
    int32_t cbT;
    int32_t cb;
    bool fRet = fFalse;

    if (!pblck->FUnpackData())
        goto LFail;

    cb = pblck->Cb();
    if (cb < SIZEOF(alf))
        goto LBug;

    if (!pblck->FReadRgb(&alf, SIZEOF(alf), 0))
        goto LFail;

    if (pbo != pvNil)
        *pbo = alf.bo;
    if (posk != pvNil)
        *posk = alf.osk;

    if (alf.bo == kboOther)
        SwapBytesBom(&alf, kbomAlf);

    cb -= SIZEOF(alf);
    cbT = alf.cbEntry * alf.ivMac;
    if (alf.bo != kboCur || alf.cbEntry <= 0 || alf.ivMac < 0 || cb != cbT + CbFromCbit(alf.ivMac) ||
        alf.cvFree >= LwMax(1, alf.ivMac))
    {
    LBug:
        Warn("file corrupt or not an AL");
        goto LFail;
    }

    _cbEntry = alf.cbEntry;
    _ivMac = alf.ivMac;
    _cvFree = alf.cvFree;
    fRet = _FReadData(pblck, cbT, cb - cbT, SIZEOF(alf));

LFail:
    TrashVarIf(!fRet, pbo);
    TrashVarIf(!fRet, posk);
    return fRet;
}

/***************************************************************************
    Delete all entries in the AL.
***************************************************************************/
void AL::DeleteAll(void)
{
    _ivMac = 0;
    _cvFree = 0;
}

/***************************************************************************
    Returns whether the given element of the allocated list is free.
***************************************************************************/
bool AL::FFree(int32_t iv)
{
    AssertThis(0);
    AssertIn(iv, 0, _ivMac);
    return (iv < _ivMac) && !(*_Qgrfbit(iv) & Fbit(iv));
}

/***************************************************************************
    Make sure there is room for at least cvAdd additional entries.  If
    fgrpShrink is set, will try to shrink the list if it has more than
    cvAdd available entries.
***************************************************************************/
bool AL::FEnsureSpace(int32_t cvAdd, uint32_t grfgrp)
{
    AssertIn(cvAdd, 0, kcbMax);
    AssertThis(0);

    // limit the size of the list
    cvAdd = LwMax(0, cvAdd - _cvFree);
    if (cvAdd > kcbMax / _cbEntry - _ivMac)
    {
        Bug("who's trying to allocate a list this big?");
        return fFalse;
    }

    return _FEnsureSizes(LwMul(cvAdd + _ivMac, _cbEntry), CbFromCbit(cvAdd + _ivMac), grfgrp);
}

/***************************************************************************
    Add an element to the list.
***************************************************************************/
bool AL::FAdd(void *pv, int32_t *piv)
{
    AssertThis(fobjAssertFull);
    AssertPvCb(pv, _cbEntry);
    AssertNilOrVarMem(piv);

    int32_t iv;

    if (_cvFree > 0)
    {
        /* find the first free one */
        uint8_t grfbit;
        uint8_t *qgrfbit, *qrgb;

        for (qgrfbit = qrgb = _Qgrfbit(0); *qgrfbit == 0xFF; qgrfbit++)
            ;
        iv = (qgrfbit - qrgb) * 8;
        for (grfbit = *qgrfbit; grfbit & 1; iv++, grfbit >>= 1)
            ;
        _cvFree--;
    }
    else
    {
        if (!FEnsureSpace(1, fgrpNil))
        {
            TrashVar(piv);
            return fFalse;
        }
        iv = _ivMac++;
    }
    AssertIn(iv, 0, _ivMac);

    /* mark the item used */
    *_Qgrfbit(iv) |= Fbit(iv);
    Assert(!FFree(iv), "why is this marked free?");
    Put(iv, pv);
    if (piv != pvNil)
        *piv = iv;

    AssertThis(fobjAssertFull);
    return fTrue;
}

/***************************************************************************
    Delete element iv from an allocated list.
***************************************************************************/
void AL::Delete(int32_t iv)
{
    AssertThis(fobjAssertFull);
    AssertIn(iv, 0, _ivMac);
    Assert(!FFree(iv), "already free!");

    // trash the thing
    TrashPvCb(QvGet(iv), _cbEntry);

    *_Qgrfbit(iv) &= ~Fbit(iv);
    _cvFree++;

    if (iv != _ivMac - 1)
    {
        AssertThis(fobjAssertFull);
        return;
    }

    // the last element was deleted, find the new _ivMac
    if (_ivMac <= _cvFree)
    {
        // none left, just nuke everything
        _ivMac = _cvFree = 0;
    }
    else
    {
        // find the new _ivMac
        uint8_t fbit;
        uint8_t *qgrfbit = _Qgrfbit(iv);

        while (iv >= 0)
        {
            fbit = Fbit(iv);
            if ((*qgrfbit & ((fbit << 1) - 1)) == 0) // check all bits from fbit on down
            {
                iv = (iv & ~0x0007L) - 1;
                qgrfbit--;
            }
            else if (!(*qgrfbit & fbit)) // check for the ith bit
                iv--;
            else
                break;
        }
        iv++;
        Assert(_cvFree >= _ivMac - iv, "everything is free!?");
        _cvFree -= _ivMac - iv;
        _ivMac = iv;
    }
    AssertThis(fobjAssertFull);
}

#ifdef DEBUG
/***************************************************************************
    Check the validity of an allocated list.
***************************************************************************/
void AL::AssertValid(uint32_t grfobj)
{
    int32_t cT, iv;

    AL_PAR::AssertValid(0);
    Assert(_Cb2() >= CbFromCbit(_ivMac), "flag area too small");
    if (grfobj & fobjAssertFull)
    {
        for (cT = 0, iv = _ivMac; iv--;)
        {
            if ((*_Qgrfbit(iv) & Fbit(iv)) == 0)
                cT++;
        }
        Assert(cT == _cvFree, "_cvFree is wrong");
    }
}
#endif // DEBUG

/***************************************************************************
    Constructor for GGB class.
***************************************************************************/
GGB::GGB(int32_t cbFixed, bool fAllowFree)
{
    AssertIn(cbFixed, 0, kcbMax);
    _clocFree = fAllowFree ? 0 : cvNil;
    _cbFixed = cbFixed;

    // use some reasonable values for _cbMinGrow* - code can always set
    // set these to something else
    _cbMinGrow1 = LwMin(1024, 16 * cbFixed);
    _cbMinGrow2 = 16 * SIZEOF(LOC);
    AssertThis(fobjAssertFull);
}

/***************************************************************************
    Duplicate the group.
***************************************************************************/
bool GGB::_FDup(PGGB pggbDst)
{
    AssertThis(fobjAssertFull);
    AssertPo(pggbDst, fobjAssertFull);
    Assert(_cbFixed == pggbDst->_cbFixed, "why do these have different sized fixed portions?");

    if (!GGB_PAR::_FDup(pggbDst, _bvMac, LwMul(_ivMac, SIZEOF(LOC))))
        return fFalse;

    pggbDst->_bvMac = _bvMac;
    pggbDst->_clocFree = _clocFree;
    pggbDst->_cbFixed = _cbFixed;
    AssertPo(pggbDst, fobjAssertFull);

    return fTrue;
}

// group on file
struct GGF
{
    int16_t bo;
    int16_t osk;
    int32_t ilocMac;
    int32_t bvMac;
    int32_t clocFree;
    int32_t cbFixed;
};
VERIFY_STRUCT_SIZE(GGF, 20);
const BOM kbomGgf = 0x5FF00000L;

/***************************************************************************
    Return the amount of space on file needed for the group.
***************************************************************************/
int32_t GGB::CbOnFile(void)
{
    AssertThis(fobjAssertFull);
    return SIZEOF(GGF) + LwMul(_ivMac, SIZEOF(LOC)) + _bvMac;
}

/***************************************************************************
    Write the group to disk.  The client must ensure that the data in the
    GGB has the correct byte order (as specified by the bo).
***************************************************************************/
bool GGB::FWrite(PBLCK pblck, int16_t bo, int16_t osk)
{
    AssertThis(fobjAssertFull);
    AssertPo(pblck, 0);
    Assert(kboCur == bo || kboOther == bo, "bad bo");
    AssertOsk(osk);

    GGF ggf;
    bool fRet;

    ggf.bo = kboCur;
    ggf.osk = osk;
    ggf.ilocMac = _ivMac;
    ggf.bvMac = _bvMac;
    ggf.clocFree = _clocFree;
    ggf.cbFixed = _cbFixed;
    AssertBomRglw(kbomLoc, SIZEOF(LOC));
    if (kboOther == bo)
    {
        // swap the stuff
        SwapBytesBom(&ggf, kbomGgf);
        Assert(ggf.bo == bo, "wrong bo");
        Assert(ggf.osk == osk, "osk not invariant under byte swapping");
        SwapBytesRglw(_Qb2(0), LwMulDiv(_ivMac, SIZEOF(LOC), SIZEOF(int32_t)));
    }
    fRet = _FWrite(pblck, &ggf, SIZEOF(ggf), _bvMac, LwMul(_ivMac, SIZEOF(LOC)));
    if (kboOther == bo)
    {
        // swap the rgloc back
        SwapBytesRglw(_Qb2(0), LwMulDiv(_ivMac, SIZEOF(LOC), SIZEOF(int32_t)));
    }
    return fRet;
}

/***************************************************************************
    Read group data from disk.
***************************************************************************/
bool GGB::_FRead(PBLCK pblck, int16_t *pbo, int16_t *posk)
{
    AssertThis(0);
    AssertPo(pblck, 0);
    AssertNilOrVarMem(pbo);
    AssertNilOrVarMem(posk);

    GGF ggf;
    int32_t cbT;
    int16_t bo;
    int32_t cb;
    bool fRet = fFalse;

    if (!pblck->FUnpackData())
        goto LFail;

    cb = pblck->Cb();
    if (cb < SIZEOF(ggf))
        goto LBug;

    if (!pblck->FReadRgb(&ggf, SIZEOF(ggf), 0))
        goto LFail;

    if (pbo != pvNil)
        *pbo = ggf.bo;
    if (posk != pvNil)
        *posk = ggf.osk;

    if ((bo = ggf.bo) == kboOther)
        SwapBytesBom(&ggf, kbomGgf);

    cb -= SIZEOF(ggf);
    cbT = ggf.ilocMac * SIZEOF(LOC);
    if (ggf.bo != kboCur || ggf.bvMac < 0 || ggf.ilocMac < 0 || cb != cbT + ggf.bvMac || ggf.cbFixed < 0 ||
        ggf.cbFixed >= kcbMax || (ggf.clocFree == cvNil) != (_clocFree == cvNil) ||
        ggf.clocFree != cvNil && (ggf.clocFree < 0 || ggf.clocFree >= ggf.ilocMac))
    {
    LBug:
        Warn("file corrupt or not a GGB");
        goto LFail;
    }

    _ivMac = ggf.ilocMac;
    _bvMac = ggf.bvMac;
    _clocFree = ggf.clocFree;
    _cbFixed = ggf.cbFixed;
    fRet = _FReadData(pblck, cb - cbT, cbT, SIZEOF(ggf));
    AssertBomRglw(kbomLoc, SIZEOF(LOC));
    if (bo == kboOther && fRet)
    {
        // adjust the byte order on the loc's.
        SwapBytesRglw(_Qb2(0), LwMulDiv(_ivMac, SIZEOF(LOC), SIZEOF(int32_t)));
    }

LFail:
    TrashVarIf(!fRet, pbo);
    TrashVarIf(!fRet, posk);
    return fRet;
}

/***************************************************************************
    Returns true iff the loc.bv is nil or iloc is out of range.
***************************************************************************/
bool GGB::FFree(int32_t iv)
{
    AssertBaseThis(0);
    AssertIn(iv, 0, kcbMax);
    LOC *qloc;

    if (!FIn(iv, 0, _ivMac))
        return fTrue;
    qloc = _Qloc(iv);
    Assert(FIn(qloc->bv, 0, _bvMac) && FIn(qloc->cb, LwMax(_cbFixed, 1), _bvMac - qloc->bv + 1) ||
               0 == qloc->cb && (0 == qloc->bv || bvNil == qloc->bv),
           "bad loc");
    return bvNil == qloc->bv;
}

/***************************************************************************
    Ensures that there is room to add at least cvAdd new entries with
    a total of cbAdd bytes (among the variable parts of the elements).
    If there is more than enough room and fgrpShrink is passed, the GST
    will shrink.
***************************************************************************/
bool GGB::FEnsureSpace(int32_t cvAdd, int32_t cbAdd, uint32_t grfgrp)
{
    AssertThis(0);
    AssertIn(cvAdd, 0, kcbMax);
    AssertIn(cbAdd, 0, kcbMax);

    int32_t clocAdd;

    if (cvNil == _clocFree)
        clocAdd = cvAdd;
    else
        clocAdd = LwMax(0, cvAdd - _clocFree);

    // we waste at most (SIZEOF(int32_t) - 1) bytes per element
    if (clocAdd > kcbMax / SIZEOF(LOC) - _ivMac || cvAdd > (kcbMax / (_cbFixed + SIZEOF(int32_t) - 1)) - _bvMac ||
        cbAdd > kcbMax - _bvMac - cvAdd * (_cbFixed + SIZEOF(int32_t) - 1))
    {
        Bug("why is this group growing so large?");
        return fFalse;
    }

    return _FEnsureSizes(_bvMac + cbAdd + LwMul(cvAdd, _cbFixed + SIZEOF(int32_t) - 1),
                         LwMul(_ivMac + clocAdd, SIZEOF(LOC)), grfgrp);
}

/***************************************************************************
    Set the minimum that a GGB should grow by.
***************************************************************************/
void GGB::SetMinGrow(int32_t cvAdd, int32_t cbAdd)
{
    AssertThis(0);
    AssertIn(cvAdd, 0, kcbMax);
    AssertIn(cbAdd, 0, kcbMax);

    _cbMinGrow1 = CbRoundToLong(cbAdd + LwMul(cvAdd, _cbFixed + SIZEOF(int32_t) - 1));
    _cbMinGrow2 = LwMul(cvAdd, SIZEOF(LOC));
}

/***************************************************************************
    Private api to remove a block of bytes.
***************************************************************************/
void GGB::_RemoveRgb(int32_t bv, int32_t cb)
{
    AssertBaseThis(0);
    AssertIn(bv, 0, _bvMac);
    AssertIn(cb, 1, _bvMac - bv + 1);
    Assert(cb == CbRoundToLong(cb), "cb not divisible by SIZEOF(int32_t)");
    uint8_t *qb;

    if (bv + cb < _bvMac)
    {
        qb = _Qb1(bv);
        BltPb(qb + cb, qb, _bvMac - bv - cb);
        _AdjustLocs(bv + 1, _bvMac + 1, -cb);
    }
    else
        _bvMac -= cb;
    TrashPvCb(_Qb1(_bvMac), cb);
}

/***************************************************************************
    Private api to remove a block of bytes.
***************************************************************************/
void GGB::_AdjustLocs(int32_t bvMin, int32_t bvLim, int32_t dcb)
{
    AssertBaseThis(0);
    AssertIn(bvMin, 0, _bvMac + 2);
    AssertIn(bvLim, bvMin, _bvMac + 2);
    AssertIn(dcb, -_bvMac, kcbMax);
    Assert((dcb % SIZEOF(int32_t)) == 0, "dcb not divisible by SIZEOF(int32_t)");
    int32_t cloc;
    LOC *qloc;

    if (FIn(_bvMac, bvMin, bvLim))
        _bvMac += dcb;
    for (qloc = _Qloc(0), cloc = _ivMac; cloc > 0; cloc--, qloc++)
    {
        if (bvNil == qloc->bv)
            continue;
        if (FIn(qloc->bv, bvMin, bvLim))
            qloc->bv += dcb;
        AssertIn(qloc->bv, 0, _bvMac);
    }
}

/***************************************************************************
    Returns a volative pointer the the fixed sized data in the element.
    If pcbVar is not nil, fills *pcbVar with the size of the variable part.
***************************************************************************/
void *GGB::QvFixedGet(int32_t iv, int32_t *pcbVar)
{
    AssertThis(0);
    AssertIn(_cbFixed, 1, kcbMax);
    AssertIn(iv, 0, _ivMac);
    Assert(!FFree(iv), "element free!");
    AssertNilOrVarMem(pcbVar);

    LOC loc;

    loc = *_Qloc(iv);
    if (pcbVar != pvNil)
        *pcbVar = loc.cb - _cbFixed;
    AssertIn(loc.cb, _cbFixed, _bvMac - loc.bv + 1);
    return _Qb1(loc.bv);
}

/***************************************************************************
    Lock the data and return a pointer to the fixed sized data.
***************************************************************************/
void *GGB::PvFixedLock(int32_t iv, int32_t *pcbVar)
{
    AssertThis(0);
    Lock();
    return QvFixedGet(iv, pcbVar);
}

/***************************************************************************
    Get the fixed sized data for the element.
***************************************************************************/
void GGB::GetFixed(int32_t iv, void *pv)
{
    AssertThis(0);
    AssertIn(_cbFixed, 1, kcbMax);
    AssertIn(iv, 0, _ivMac);
    Assert(!FFree(iv), "element free!");
    AssertPvCb(pv, _cbFixed);

    LOC loc;

    loc = *_Qloc(iv);
    AssertIn(loc.cb, _cbFixed, _bvMac - loc.bv + 1);
    CopyPb(_Qb1(loc.bv), pv, _cbFixed);
}

/***************************************************************************
    Put the fixed sized data for the element.
***************************************************************************/
void GGB::PutFixed(int32_t iv, void *pv)
{
    AssertThis(0);
    AssertIn(_cbFixed, 1, kcbMax);
    AssertIn(iv, 0, _ivMac);
    Assert(!FFree(iv), "element free!");
    AssertPvCb(pv, _cbFixed);

    LOC loc;

    loc = *_Qloc(iv);
    AssertIn(loc.cb, _cbFixed, _bvMac - loc.bv + 1);
    CopyPb(pv, _Qb1(loc.bv), _cbFixed);
    AssertThis(0);
}

/***************************************************************************
    Return the length of the variable part of the iv'th element.
***************************************************************************/
int32_t GGB::Cb(int32_t iv)
{
    AssertThis(0);
    AssertIn(iv, 0, _ivMac);
    Assert(!FFree(iv), "element free!");

    return _Qloc(iv)->cb - _cbFixed;
}

/***************************************************************************
    Return a volatile pointer to the variable part of the iv'th element.
    If pcb is not nil, sets *pcb to the length of the (variable part of the)
    item.
***************************************************************************/
void *GGB::QvGet(int32_t iv, int32_t *pcb)
{
    AssertThis(0);
    AssertIn(iv, 0, _ivMac);
    Assert(!FFree(iv), "element free!");
    AssertNilOrVarMem(pcb);

    LOC loc;

    loc = *_Qloc(iv);
    if (pcb != pvNil)
        *pcb = loc.cb - _cbFixed;
    AssertIn(loc.cb, _cbFixed, _bvMac - loc.bv + 1);
    return _Qb1(loc.bv + _cbFixed);
}

/***************************************************************************
    Lock the data and return a pointer to the (variable part of the) iv'th
    item.  If pcb is not nil, sets *pcb to the length of the (variable part
    of the) item.
***************************************************************************/
void *GGB::PvLock(int32_t iv, int32_t *pcb)
{
    AssertThis(0);
    Lock();
    return QvGet(iv, pcb);
}

/***************************************************************************
    Copy the (variable part of the) iv'th element to pv.
***************************************************************************/
void GGB::Get(int32_t iv, void *pv)
{
    AssertThis(0);
    AssertIn(iv, 0, _ivMac);
    Assert(!FFree(iv), "element free!");

    LOC loc;

    loc = *_Qloc(iv);
    AssertPvCb(pv, loc.cb - _cbFixed);
    CopyPb(_Qb1(loc.bv + _cbFixed), pv, loc.cb - _cbFixed);
}

/***************************************************************************
    Copy *pv to the (variable part of the) iv'th element.
***************************************************************************/
void GGB::Put(int32_t iv, void *pv)
{
    AssertThis(0);
    AssertIn(iv, 0, _ivMac);
    Assert(!FFree(iv), "element free!");

    LOC loc;

    loc = *_Qloc(iv);
    AssertPvCb(pv, loc.cb - _cbFixed);
    CopyPb(pv, _Qb1(loc.bv + _cbFixed), loc.cb - _cbFixed);
}

/***************************************************************************
    Replace the (variable part of the) iv'th element with the stuff in pv
    (cb bytes worth).  pv may be nil (effectively resizing the block).
***************************************************************************/
bool GGB::FPut(int32_t iv, int32_t cb, void *pv)
{
    AssertThis(0);
    AssertIn(iv, 0, _ivMac);
    Assert(!FFree(iv), "element free!");
    AssertIn(cb, 0, kcbMax);

    int32_t cbCur = Cb(iv);

    if (cb > cbCur)
    {
        if (!FInsertRgb(iv, cbCur, cb - cbCur, pvNil))
            return fFalse;
    }
    else if (cb < cbCur)
        DeleteRgb(iv, cb, cbCur - cb);

    if (pv != pvNil && cb > 0)
    {
        AssertPvCb(pv, cb);
        CopyPb(pv, QvGet(iv), cb);
    }

    AssertThis(0);
    return fTrue;
}

/***************************************************************************
    Get a portion of the element.
***************************************************************************/
void GGB::GetRgb(int32_t iv, int32_t bv, int32_t cb, void *pv)
{
    AssertThis(0);
    AssertIn(iv, 0, _ivMac);
    Assert(!FFree(iv), "element free!");
    AssertPvCb(pv, cb);

    LOC loc;

    bv += _cbFixed;
    loc = *_Qloc(iv);
    AssertIn(bv, _cbFixed, loc.cb);
    AssertIn(cb, 1, loc.cb - bv + 1);

    CopyPb(_Qb1(loc.bv + bv), pv, cb);
}

/***************************************************************************
    Put a portion of the element.
***************************************************************************/
void GGB::PutRgb(int32_t iv, int32_t bv, int32_t cb, void *pv)
{
    AssertThis(0);
    AssertIn(iv, 0, _ivMac);
    Assert(!FFree(iv), "element free!");
    AssertPvCb(pv, cb);

    LOC loc;

    bv += _cbFixed;
    loc = *_Qloc(iv);
    AssertIn(bv, _cbFixed, loc.cb);
    AssertIn(cb, 1, loc.cb - bv + 1);

    CopyPb(pv, _Qb1(loc.bv + bv), cb);
    AssertThis(0);
}

/***************************************************************************
    Remove a portion of element iv (can't be all of it).
***************************************************************************/
void GGB::DeleteRgb(int32_t iv, int32_t bv, int32_t cb)
{
    AssertThis(0);
    AssertIn(iv, 0, _ivMac);
    Assert(!FFree(iv), "element free!");

    LOC loc;
    LOC *qloc;
    int32_t cbDel;
    uint8_t *qb;

    bv += _cbFixed;
    loc = *_Qloc(iv);
    AssertIn(bv, _cbFixed, loc.cb);
    AssertIn(cb, 1, loc.cb - bv + 1);

    if (bv + cb < loc.cb)
    {
        // shift usable stuff down
        qb = _Qb1(loc.bv + bv);
        BltPb(qb + cb, qb, loc.cb - bv - cb);
    }

    // determine the number of bytes to nuke
    cbDel = CbRoundToLong(loc.cb) - CbRoundToLong(loc.cb - cb);
    if (cbDel > 0)
        _RemoveRgb(loc.bv + loc.cb - cb, cbDel);

    qloc = _Qloc(iv);
    if (0 == (qloc->cb -= cb))
    {
        Assert(_cbFixed == 0, "oops!");
        qloc->bv = 0; // empty element
    }
    AssertThis(0);
}

/***************************************************************************
    Insert cb new bytes at location bv into the iv'th element.  pv may
    be nil.
***************************************************************************/
bool GGB::FInsertRgb(int32_t iv, int32_t bv, int32_t cb, const void *pv)
{
    AssertThis(0);
    AssertIn(iv, 0, _ivMac);
    Assert(!FFree(iv), "element free!");
    AssertIn(cb, 1, kcbMax);

    LOC loc;
    int32_t cbAdd;
    uint8_t *qb;

    bv += _cbFixed;
    loc = *_Qloc(iv);
    AssertIn(bv, _cbFixed, loc.cb + 1);
    if (loc.cb == 0)
        loc.bv = _bvMac;

    // need to add this many bytes to _bvMac
    cbAdd = CbRoundToLong(loc.cb + cb) - CbRoundToLong(loc.cb);
    if (cbAdd > 0)
    {
        int32_t bvT;

        if (!_FEnsureSizes(_bvMac + cbAdd, LwMul(_ivMac, SIZEOF(LOC)), fgrpNil))
            return fFalse;

        // move later entries back
        bvT = loc.bv + CbRoundToLong(loc.cb);
        if (bvT < _bvMac)
        {
            qb = _Qb1(bvT);
            BltPb(qb, qb + cbAdd, _bvMac - bvT);
            _AdjustLocs(loc.bv + 1, _bvMac + 1, cbAdd);
        }
        else
            _bvMac += cbAdd;
    }

    // move data within this element
    if (bv < loc.cb)
    {
        qb = _Qb1(loc.bv + bv);
        BltPb(qb, qb + cb, loc.cb - bv);
    }

    if (pv != pvNil)
    {
        AssertPvCb(pv, cb);
        CopyPb(pv, _Qb1(loc.bv + bv), cb);
    }
    else
        TrashPvCb(_Qb1(loc.bv + bv), cb);

    // copy the entire loc in case loc.bv got set to _bvMac (if the item was empty)
    loc.cb += cb;
    *_Qloc(iv) = loc;
    AssertThis(0);
    return fTrue;
}

/***************************************************************************
    Move cb bytes from position bvSrc in ivSrc to position bvDst in ivDst.
    This can fail only because of the padding used for each entry (at most
    SIZEOF(int32_t) additional bytes will need to be allocated).
***************************************************************************/
bool GGB::FMoveRgb(int32_t ivSrc, int32_t bvSrc, int32_t ivDst, int32_t bvDst, int32_t cb)
{
    AssertThis(fobjAssertFull);
    AssertIn(ivSrc, 0, _ivMac);
    Assert(!FFree(ivSrc), "element free!");
    AssertIn(ivDst, 0, _ivMac);
    Assert(!FFree(ivDst), "element free!");
    AssertIn(bvSrc, 0, Cb(ivSrc) + 1);
    AssertIn(cb, 0, Cb(ivSrc) + 1 - bvSrc);
    AssertIn(bvDst, 0, Cb(ivDst) + 1);

    LOC *qloc;
    LOC locSrc, locDst;
    int32_t cbMove, cbT;

    locSrc = *_Qloc(ivSrc);
    locDst = *_Qloc(ivDst);

    // determine the number of bytes to resize by
    cbT = (CbRoundToLong(locDst.cb + cb) - CbRoundToLong(locDst.cb)) -
          (CbRoundToLong(locSrc.cb) - CbRoundToLong(locSrc.cb - cb));
    if (cbT > 0)
    {
        Assert(cb % SIZEOF(int32_t) != 0, "why are we here when cb is a multiple of SIZEOF(int32_t)?");
        if (!_FEnsureSizes(_bvMac + cbT, LwMul(_ivMac, SIZEOF(LOC)), fgrpNil))
            return fFalse;
    }

    // move most of the bytes
    cbMove = LwRoundToward(cb, SIZEOF(int32_t));
    AssertIn(cb, cbMove, cbMove + SIZEOF(int32_t));
    if (cbMove > 0)
    {
        int32_t bv1 = locSrc.bv + bvSrc + _cbFixed;
        int32_t bv2 = locDst.bv + bvDst + _cbFixed;

        qloc = _Qloc(ivSrc);
        if (0 == (qloc->cb -= cbMove))
        {
            Assert(_cbFixed == 0, "what?");
            qloc->bv = 0;
        }
        qloc = _Qloc(ivDst);
        if (qloc->cb == 0)
        {
            Assert(_cbFixed == 0, "what?");
            bv2 = qloc->bv = _bvMac;
        }
        qloc->cb += cbMove;
        if (bv1 < bv2)
        {
            SwapBlocks(_Qb1(bv1), cbMove, bv2 - bv1 - cbMove);
            _AdjustLocs(locSrc.bv + 1, locDst.bv + 1, -cbMove);
        }
        else
        {
            SwapBlocks(_Qb1(bv2), bv1 - bv2, cbMove);
            _AdjustLocs(locDst.bv + 1, locSrc.bv + 1, cbMove);
        }
        AssertThis(fobjAssertFull);
    }

    // move the last few bytes
    if (cb > cbMove)
    {
        uint8_t rgb[SIZEOF(int32_t)];

        GetRgb(ivSrc, bvSrc, cb - cbMove, rgb);
        DeleteRgb(ivSrc, bvSrc, cb - cbMove);
        AssertDo(FInsertRgb(ivDst, bvDst + cbMove, cb - cbMove, rgb), "logic error caused failure");
    }
    AssertThis(fobjAssertFull);
    return fTrue;
}

/***************************************************************************
    Append the variable data of ivSrc to ivDst, and delete ivSrc.

    NOTE: this is kind of goofy.  The only time FMoveRgb could possibly
    fail if we just do the naive thing (FMoveRgb the entire var data,
    then delete the source element) is if _cbFixed is not a multiple
    of SIZEOF(int32_t).
***************************************************************************/
void GGB::Merge(int32_t ivSrc, int32_t ivDst)
{
    AssertThis(fobjAssertFull);
    AssertIn(ivSrc, 0, _ivMac);
    Assert(!FFree(ivSrc), "element free!");
    AssertIn(ivDst, 0, _ivMac);
    Assert(!FFree(ivDst), "element free!");
    Assert(ivSrc != ivDst, "can't merge an element with itself!");
    int32_t cb, cbMove, bv;
    uint8_t rgb[SIZEOF(int32_t)];

    cb = Cb(ivSrc);
    cbMove = LwRoundToward(cb, SIZEOF(int32_t));
    if (cb > cbMove)
        GetRgb(ivSrc, cbMove, cb - cbMove, rgb); // get the tail bytes

    bv = Cb(ivDst);
    if (cbMove > 0)
    {
        // move the main section
        AssertDo(FMoveRgb(ivSrc, 0, ivDst, bv, cbMove), "why did FMoveRgb fail?");
    }

    // delete the source item
    Delete(ivSrc);
    if (ivSrc < ivDst)
        ivDst--;

    if (cb > cbMove)
    {
        // insert the remaining few bytes - there should already be room
        // for these
        AssertDo(FInsertRgb(ivDst, bv + cbMove, cb - cbMove, rgb), "why did FInsertRgb fail?");
    }
    AssertThis(fobjAssertFull);
}

#ifdef DEBUG
/***************************************************************************
    Validate a group.
***************************************************************************/
void GGB::AssertValid(uint32_t grfobj)
{
    LOC loc;
    int32_t iloc;
    int32_t cbTot, clocFree;

    GGB_PAR::AssertValid(grfobj);
    AssertIn(_ivMac, 0, kcbMax);
    AssertIn(_bvMac, 0, kcbMax);
    Assert(_Cb1() >= _bvMac, "group area too small");
    Assert(_Cb2() >= LwMul(_ivMac, SIZEOF(LOC)), "rgloc area too small");
    Assert(_clocFree == cvNil || _clocFree == 0 || _clocFree > 0 && _clocFree < _ivMac, "_clocFree is wrong");
    AssertIn(_cbFixed, 0, kcbMax);

    if (grfobj & fobjAssertFull)
    {
        for (clocFree = cbTot = iloc = 0; iloc < _ivMac; iloc++)
        {
            loc = *_Qloc(iloc);
            if (bvNil == loc.bv)
            {
                Assert(iloc < _ivMac - 1, "Last element free");
                Assert(loc.cb == 0, "bad cb in free loc");
                clocFree++;
                continue;
            }
            AssertIn(loc.cb, _cbFixed, _bvMac + 1);
            AssertIn(loc.bv, 0, _bvMac);
            Assert(loc.cb > 0 || loc.bv == 0, "zero sized item doesn't have zero bv");
            loc.cb = CbRoundToLong(loc.cb);
            Assert(loc.bv + loc.cb <= _bvMac, "loc extends past _bvMac");
            cbTot += loc.cb;
        }
        Assert(cbTot == _bvMac, "group wrong size");
        Assert(clocFree == _clocFree || _clocFree == cvNil && clocFree == 0, "bad _clocFree");
    }
}
#endif // DEBUG

/***************************************************************************
    Allocate a new group with room for at least cvInit elements containing
    at least cbInit bytes worth of (total) space.
***************************************************************************/
PGG GG::PggNew(int32_t cbFixed, int32_t cvInit, int32_t cbInit)
{
    AssertIn(cbFixed, 0, kcbMax);
    AssertIn(cvInit, 0, kcbMax);
    AssertIn(cbInit, 0, kcbMax);

    PGG pgg;

    if ((pgg = NewObj GG(cbFixed)) == pvNil)
        return pvNil;
    if ((cvInit > 0 || cbInit > 0) && !pgg->FEnsureSpace(cvInit, cbInit, fgrpNil))
    {
        ReleasePpo(&pgg);
        return pvNil;
    }
    AssertPo(pgg, fobjAssertFull);
    return pgg;
}

/***************************************************************************
    Read a group from a block and return it.
***************************************************************************/
PGG GG::PggRead(PBLCK pblck, int16_t *pbo, int16_t *posk)
{
    AssertPo(pblck, 0);
    AssertNilOrVarMem(pbo);
    AssertNilOrVarMem(posk);

    PGG pgg;

    if ((pgg = NewObj GG(0)) == pvNil)
        goto LFail;
    if (!pgg->_FRead(pblck, pbo, posk))
    {
        ReleasePpo(&pgg);
    LFail:
        TrashVar(pbo);
        TrashVar(posk);
        return pvNil;
    }
    AssertPo(pgg, fobjAssertFull);
    return pgg;
}

/***************************************************************************
    Read a group from file and return it.
***************************************************************************/
PGG GG::PggRead(PFIL pfil, FP fp, int32_t cb, int16_t *pbo, int16_t *posk)
{
    BLCK blck(pfil, fp, cb);
    return PggRead(&blck, pbo, posk);
}

/***************************************************************************
    Duplicate this GG.
***************************************************************************/
PGG GG::PggDup(void)
{
    AssertThis(0);
    PGG pgg;

    if (pvNil == (pgg = PggNew(_cbFixed)))
        return pvNil;

    if (!_FDup(pgg))
        ReleasePpo(&pgg);

    AssertNilOrPo(pgg, 0);
    return pgg;
}

/***************************************************************************
    Insert an element into the group.
***************************************************************************/
bool GG::FInsert(int32_t iv, int32_t cb, const void *pv, const void *pvFixed)
{
    AssertThis(fobjAssertFull);
    AssertIn(cb, 0, kcbMax);
    AssertIn(iv, 0, _ivMac + 1);

    uint8_t *qb;
    LOC loc;
    LOC *qloc;

    cb += _cbFixed;
    loc.cb = cb;
    loc.bv = cb == 0 ? 0 : _bvMac;
    cb = CbRoundToLong(cb);

    if (!_FEnsureSizes(_bvMac + cb, LwMul(_ivMac + 1, SIZEOF(LOC)), fgrpNil))
        return fFalse;

    // make room for the entry
    qloc = _Qloc(iv);
    if (iv < _ivMac)
        BltPb(qloc, qloc + 1, LwMul(_ivMac - iv, SIZEOF(LOC)));
    *qloc = loc;

    if (pvNil != pv && cb > 0)
    {
        AssertPvCb(pv, cb - _cbFixed);
        qb = _Qb1(loc.bv);
        TrashPvCb(qb, _cbFixed);
        CopyPb(pv, qb + _cbFixed, loc.cb - _cbFixed);
        TrashPvCb(qb + loc.cb, cb - loc.cb);
    }
    else
        TrashPvCb(_Qb1(loc.bv), cb);

    if (pvNil != pvFixed)
    {
        Assert(_cbFixed > 0, "why is pvFixed not nil?");
        AssertPvCb(pvFixed, _cbFixed);
        qb = _Qb1(loc.bv);
        CopyPb(pvFixed, qb, _cbFixed);
    }

    _bvMac += cb;
    _ivMac++;

    AssertThis(fobjAssertFull);
    return fTrue;
}

/***************************************************************************
    Takes cv entries from pggSrc at ivSrc and inserts (a copy of) them into
    this GG at ivDst.
***************************************************************************/
bool GG::FCopyEntries(PGG pggSrc, int32_t ivSrc, int32_t ivDst, int32_t cv)
{
    AssertThis(fobjAssertFull);
    AssertPo(pggSrc, 0);
    AssertIn(cv, 0, pggSrc->IvMac() + 1);
    AssertIn(ivSrc, 0, pggSrc->IvMac() + 1 - cv);
    AssertIn(ivDst, 0, _ivMac + 1);
    int32_t cb, cbFixed, iv, ivLim;
    LOC loc;
    uint8_t *pb;

    if ((cbFixed = pggSrc->CbFixed()) != CbFixed())
    {
        Bug("Groups have different fixed sizes");
        return fFalse;
    }
    if (pggSrc == this)
    {
        Bug("Cant insert from same group");
        return fFalse;
    }

    cb = 0;
    for (iv = ivSrc, ivLim = ivSrc + cv; iv < ivLim; iv++)
        cb += pggSrc->Cb(iv);

    if (!FEnsureSpace(cv, cb))
        return fFalse;

    pggSrc->Lock();
    for (iv = ivSrc; iv < ivLim; iv++)
    {
        loc = *pggSrc->_Qloc(iv);
        pb = pggSrc->_Qb1(loc.bv);
        AssertDo(FInsert(ivDst + iv - ivSrc, loc.cb - cbFixed, pb + cbFixed, cbFixed == 0 ? pvNil : pb), 0);
    }
    pggSrc->Unlock();

    AssertThis(fobjAssertFull);
    return fTrue;
}

/***************************************************************************
    Append an element to the group.
***************************************************************************/
bool GG::FAdd(int32_t cb, int32_t *piv, const void *pv, void *pvFixed)
{
    AssertThis(0);
    AssertNilOrVarMem(piv);

    if (piv != pvNil)
        *piv = _ivMac;
    if (!FInsert(_ivMac, cb, pv, pvFixed))
    {
        TrashVar(piv);
        return fFalse;
    }
    return fTrue;
}

/***************************************************************************
    Delete an element from the group.
***************************************************************************/
void GG::Delete(int32_t iv)
{
    AssertThis(fobjAssertFull);
    AssertIn(iv, 0, _ivMac);
    LOC *qloc;
    LOC loc;

    qloc = _Qloc(iv);
    loc = *qloc;
    if (iv < --_ivMac)
        BltPb(qloc + 1, qloc, LwMul(_ivMac - iv, SIZEOF(LOC)));
    TrashPvCb(_Qloc(_ivMac), SIZEOF(LOC));
    if (loc.cb > 0)
        _RemoveRgb(loc.bv, CbRoundToLong(loc.cb));
    AssertThis(fobjAssertFull);
}

/***************************************************************************
    Move the entry at ivSrc to be immediately before the element that is
    currently at ivTarget.  If ivTarget > ivSrc, the entry actually ends
    up at (ivTarget - 1) and the entry at ivTarget doesn't move.  If
    ivTarget < ivSrc, the entry ends up at ivTarget and the entry at
    ivTarget moves to (ivTarget + 1).  Everything in between is shifted
    appropriately.  ivTarget is allowed to be equal to IvMac().
***************************************************************************/
void GG::Move(int32_t ivSrc, int32_t ivTarget)
{
    AssertThis(0);
    AssertIn(ivSrc, 0, _ivMac);
    AssertIn(ivTarget, 0, _ivMac + 1);

    MoveElement(_Qloc(0), SIZEOF(LOC), ivSrc, ivTarget);
    AssertThis(0);
}

/***************************************************************************
    Swap two elements in a GG.
***************************************************************************/
void GG::Swap(int32_t iv1, int32_t iv2)
{
    AssertThis(0);
    AssertIn(iv1, 0, _ivMac);
    AssertIn(iv2, 0, _ivMac);

    SwapPb(_Qloc(iv1), _Qloc(iv2), SIZEOF(LOC));
    AssertThis(0);
}

#ifdef DEBUG
/***************************************************************************
    Validate a group.
***************************************************************************/
void GG::AssertValid(uint32_t grfobj)
{
    GG_PAR::AssertValid(grfobj);
    AssertVar(_clocFree == cvNil, "bad _clocFree in GG", &_clocFree);
}
#endif // DEBUG

/***************************************************************************
    Allocate a new allocated group with romm for at least cvInit elements
    containing at least cbInit bytes worth of (total) space.
***************************************************************************/
PAG AG::PagNew(int32_t cbFixed, int32_t cvInit, int32_t cbInit)
{
    AssertIn(cbFixed, 0, kcbMax);
    AssertIn(cvInit, 0, kcbMax);
    AssertIn(cbInit, 0, kcbMax);

    PAG pag;

    if ((pag = NewObj AG(cbFixed)) == pvNil)
        return pvNil;
    if ((cvInit > 0 || cbInit > 0) && !pag->FEnsureSpace(cvInit, cbInit, fgrpNil))
    {
        ReleasePpo(&pag);
        return pvNil;
    }
    AssertPo(pag, fobjAssertFull);
    return pag;
}

/***************************************************************************
    Read an allocated group from a block and return it.
***************************************************************************/
PAG AG::PagRead(PBLCK pblck, int16_t *pbo, int16_t *posk)
{
    AssertPo(pblck, 0);
    AssertNilOrVarMem(pbo);
    AssertNilOrVarMem(posk);

    PAG pag;

    if ((pag = NewObj AG(0)) == pvNil)
        goto LFail;
    if (!pag->_FRead(pblck, pbo, posk))
    {
        ReleasePpo(&pag);
    LFail:
        TrashVar(pbo);
        TrashVar(posk);
        return pvNil;
    }
    AssertPo(pag, fobjAssertFull);
    return pag;
}

/***************************************************************************
    Read an allocated group from file and return it.
***************************************************************************/
PAG AG::PagRead(PFIL pfil, FP fp, int32_t cb, int16_t *pbo, int16_t *posk)
{
    BLCK blck(pfil, fp, cb);
    return PagRead(&blck, pbo, posk);
}

/***************************************************************************
    Duplicate this AG.
***************************************************************************/
PAG AG::PagDup(void)
{
    AssertThis(0);
    PAG pag;

    if (pvNil == (pag = PagNew(_cbFixed)))
        return pvNil;

    if (!_FDup(pag))
        ReleasePpo(&pag);

    AssertNilOrPo(pag, 0);
    return pag;
}

/***************************************************************************
    Add an element to the allocated group.
***************************************************************************/
bool AG::FAdd(int32_t cb, int32_t *piv, const void *pv, void *pvFixed)
{
    AssertThis(fobjAssertFull);
    AssertIn(cb, 0, kcbMax);
    AssertNilOrVarMem(piv);

    int32_t iloc;
    uint8_t *qb;
    LOC loc;
    LOC *qloc;

    cb += _cbFixed;
    AssertIn(cb, 0, kcbMax);

    if (0 < _clocFree)
    {
        // find the first free element
        qloc = _Qloc(0);
        for (iloc = 0; iloc < _ivMac; iloc++, qloc++)
        {
            if (qloc->bv == bvNil)
                break;
        }
        Assert(iloc < _ivMac - 1, "_clocFree wrong");
        _clocFree--;
    }
    else
        iloc = _ivMac;
    if (iloc == _ivMac)
        _ivMac++;

    loc.cb = cb;
    loc.bv = cb == 0 ? 0 : _bvMac;
    cb = CbRoundToLong(cb);

    if (!_FEnsureSizes(_bvMac + cb, LwMul(_ivMac, SIZEOF(LOC)), fgrpNil))
    {
        if (iloc == _ivMac - 1)
            _ivMac--;
        else
            _clocFree++;
        TrashVar(piv);
        return fFalse;
    }

    // fill in the loc and copy the data
    *_Qloc(iloc) = loc;
    if (pv != pvNil)
    {
        AssertPvCb(pv, cb - _cbFixed);
        qb = _Qb1(loc.bv);
        TrashPvCb(qb, _cbFixed);
        CopyPb(pv, qb + _cbFixed, loc.cb - _cbFixed);
        TrashPvCb(qb + loc.cb, cb - loc.cb);
    }
    else
        TrashPvCb(_Qb1(loc.bv), cb);

    if (pvNil != pvFixed)
    {
        Assert(_cbFixed > 0, "why is pvFixed not nil?");
        AssertPvCb(pvFixed, _cbFixed);
        qb = _Qb1(loc.bv);
        CopyPb(pvFixed, qb, _cbFixed);
    }

    _bvMac += cb;
    if (pvNil != piv)
        *piv = iloc;

    AssertThis(fobjAssertFull);
    return fTrue;
}

/***************************************************************************
    Delete an element from the group.
***************************************************************************/
void AG::Delete(int32_t iv)
{
    AssertThis(fobjAssertFull);
    AssertIn(iv, 0, _ivMac);
    Assert(!FFree(iv), "entry already free!");

    LOC *qloc;
    LOC loc;

    qloc = _Qloc(iv);
    loc = *qloc;

    if (iv == _ivMac - 1)
    {
        // move _ivMac back past any free entries on the end
        while (--_ivMac > 0 && (--qloc)->bv == bvNil)
            _clocFree--;
        TrashPvCb(_Qloc(_ivMac), LwMul(iv - _ivMac + 1, SIZEOF(LOC)));
    }
    else
    {
        qloc->bv = bvNil;
        qloc->cb = 0;
        _clocFree++;
    }
    if (loc.cb > 0)
        _RemoveRgb(loc.bv, CbRoundToLong(loc.cb));
    AssertThis(fobjAssertFull);
}

#ifdef DEBUG
/***************************************************************************
    Validate a group.
***************************************************************************/
void AG::AssertValid(uint32_t grfobj)
{
    AG_PAR::AssertValid(grfobj);
    AssertIn(_clocFree, 0, LwMax(1, _ivMac));
}
#endif // DEBUG
