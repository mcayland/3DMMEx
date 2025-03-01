/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Mac menu bar management.

    REVIEW shonk: this is broken! Menu item names should be stored as STNs
    in the GG, except for DA names, which should be raw schar's (since
    they can have zeros in them).

***************************************************************************/
#include "frame.h"
ASSERTNAME

const achar kchCid = '#';
const achar kchList = '_';
const achar kchFontList = '$';

PMUB vpmubCur;
RTCLASS(MUB)

/***************************************************************************
    Destructor - make sure vpmubCur is not this mub.
***************************************************************************/
MUB::~MUB(void)
{
    // REVIEW shonk: free the _hnmbar
    if (vpmubCur == this)
        vpmubCur = pvNil;
    _Free();
}

/***************************************************************************
    Static method to load and set a new menu bar.
***************************************************************************/
PMUB MUB::PmubNew(uint32_t ridMenuBar)
{
    PMUB pmub;

    if ((pmub = NewObj MUB) == pvNil)
        return pvNil;

    if (!pmub->_FFetchRes(ridMenuBar))
    {
        ReleasePpo(&pmub);
        return pvNil;
    }
    AssertPo(pmub, 0);
    return pmub;
}

/***************************************************************************
    Frees all mem associated with the menu bar.
***************************************************************************/
void MUB::_Free(void)
{
    int32_t imnu;
    MNU mnu;

    ReleasePpo(&_pglmlst);
    if (_pglmnu == pvNil)
        return;
    for (imnu = _pglmnu->IvMac(); imnu-- != 0;)
    {
        _pglmnu->Get(imnu, &mnu);
        ReleasePpo(&mnu.pglmni);
    }
    ReleasePpo(&_pglmnu);
}

/***************************************************************************
    Loads the menu bar with the given resource id and makes this MUB the
    current one.  Can only be called once per mub.
***************************************************************************/
bool MUB::_FFetchRes(uint32_t ridMenuBar)
{
    SMB **hnsmb;
    MNU mnu;
    int32_t cmnu, imnu;

    // get the menu list and record the menu id's
    if ((hnsmb = (SMB **)GetResource('MBAR', (ushort)ridMenuBar)) == hNil)
        return fFalse;

    cmnu = (*hnsmb)->cmid;
    if ((_pglmnu = GL::PglNew(size(MNU), cmnu)) == pvNil)
        return fFalse;

    mnu.hnsmu = hNil;
    mnu.pglmni = pvNil;
    for (imnu = 0; imnu < cmnu; imnu++)
    {
        mnu.mid = (*hnsmb)->rgmid[imnu];
        AssertDo(_pglmnu->FInsert(imnu, &mnu), "assured insert failed");
    }
    ReleaseResource((HN)hnsmb);

    // get the mbar and set it
    if ((_hnmbar = GetNewMBar((ushort)ridMenuBar)) == hNil)
        goto LFail;
    SetMenuBar(_hnmbar);

    for (imnu = 0; imnu < cmnu; imnu++)
    {
        int32_t cmni, imni;
        MNI mni;
        int32_t rtg;

        _pglmnu->Get(imnu, &mnu);
        if (hNil == (mnu.hnsmu = GetMHandle((ushort)mnu.mid)))
            goto LFail;
        cmni = CountMItems(mnu.hnsmu);
        if ((mnu.pglmni = GL::PglNew(size(MNI), cmni)) == pvNil)
            goto LFail;
        _pglmnu->Put(imnu, &mnu);

        for (imni = 0; imni < cmni; imni++)
        {
            achar stName[kcbMaxSt];
            achar stz[kcbMaxStz];
            achar *pch, *pchLim;
            int32_t cch;
            achar chList;
            MLST mlst;
            int32_t onn;

            // This loop looks for kchCid and kchList in the menu name.
            // Following a kchCid, it extracts the number (in ascii).
            // Following a kchList, an optional resource type (rtg)
            // is extracted.
            rtg = 0;
            mni.cid = cidNil;
            mni.lw0 = 0;
            GetItem(mnu.hnsmu, imni + 1, (uint8_t *)stName);
            pchLim = pvNil;
            chList = chNil;
            for (cch = CchSt(stName), pch = PrgchSt(stName); cch--; pch++)
            {
                if (pchLim != pvNil)
                {
                    AssertIn(*pch, '0', '9' + 1);
                    mni.cid = mni.cid * 10 + *pch - '0';
                }
                else if (*pch == kchCid)
                    pchLim = pch;
                else if ((*pch == kchList || *pch == kchFontList) && pch == PrgchSt(stName))
                    chList = *pch;
                else if (kchList == chList)
                    rtg = (rtg << 8) | *pch;
            }

            switch (chList)
            {
            default:
                Assert(chList == chNil, "unknown list type");
                // not a list
                if (pchLim != pvNil)
                {
                    SetStCch(stName, pchLim - stName - 1);
                    if (CchSt(stName) == 0)
                    {
                        SetStCch(stName, 1);
                        stName[1] = ' ';
                    }
                    SetItem(mnu.hnsmu, imni + 1, (uint8_t *)stName);
                }
                if (!mnu.pglmni->FInsert(imni, &mni))
                    goto LFail;
                break;

            case kchFontList:
                // insert all the fonts
                mlst.fSeparator = (0 < imni);
                mlst.cid = mni.cid;
                mlst.imnu = imnu;
                mlst.cmni = vntl.OnnMac();
                if (!mlst.fSeparator)
                {
                    DelMenuItem(mnu.hnsmu, imni + 1);
                    imni--;
                    cmni--;
                }
                else
                {
                    SetItem(mnu.hnsmu, imni + 1, "\p-");
                    DisableItem(mnu.hnsmu, imni + 1);
                    mni.cid = cidNil;
                    if (!mnu.pglmni->FInsert(imni, &mni))
                        goto LFail;
                }
                mlst.imniBase = imni + 1;

                for (onn = 0; onn < mlst.cmni; onn++)
                {
                    vntl.GetStz(onn, stz);
                    if (!_FInsertMni(imnu, ++imni, mlst.cid, 0, stz))
                        goto LFail;
                    cmni++;
                }
                goto LInsertMlst;

            case kchList:
                if (imni != cmni - 1)
                {
                    Assert(rtg == 0, "a resource list can only be the last item in a menu");
                    rtg = 0;
                }
                mlst.fSeparator = 0 != rtg && 0 < imni;
                mlst.cid = mni.cid;
                mlst.imnu = imnu;
                mlst.cmni = 0;
                if (!mlst.fSeparator)
                {
                    DelMenuItem(mnu.hnsmu, imni + 1);
                    imni--;
                    cmni--;
                }
                else
                {
                    SetItem(mnu.hnsmu, imni + 1, "\p-");
                    DisableItem(mnu.hnsmu, imni + 1);
                    mni.cid = cidNil;
                    if (!mnu.pglmni->FInsert(imni, &mni))
                        goto LFail;
                }
                mlst.imniBase = imni + 1;
                if (0 != rtg)
                {
                    AddResMenu(mnu.hnsmu, rtg);
                    mlst.cmni = cvNil;
                }

            LInsertMlst:
                if (pvNil == _pglmlst && pvNil == (_pglmlst = GL::PglNew(size(MLST), 1)) || !_pglmlst->FPush(&mlst))
                {
                    goto LFail;
                }
                break;
            }
        }
    }
    DrawMenuBar();
    vpmubCur = this;
    return fTrue;

LFail:
    _Free();
    if (pvNil != vpmubCur)
        SetMenuBar(vpmubCur->_hnmbar);
    return fFalse;
}

/***************************************************************************
    Make this the current menu bar.
***************************************************************************/
void MUB::Set(void)
{
    AssertThis(0);
    SetMenuBar(_hnmbar);
    DrawMenuBar();
    vpmubCur = this;
}

/***************************************************************************
    Handle a mouse down event in the menu bar.
***************************************************************************/
bool MUB::FDoClick(EVT *pevt)
{
    AssertThis(0);
    AssertVarMem(pevt);
    int32_t lwCode;
    CMD cmd;
    bool fRet = fFalse;

    Clean();
    lwCode = MenuSelect(pevt->where);
    if (SwHigh(lwCode) != 0 && _FGetCmdFromCode(lwCode, &cmd))
    {
        vpcex->EnqueueCmd(&cmd);
        fRet = fTrue;
    }
    HiliteMenu(0);
    return fRet;
}

/***************************************************************************
    Handle a menu key event.
***************************************************************************/
bool MUB::FDoKey(EVT *pevt)
{
    AssertThis(0);
    AssertVarMem(pevt);
    int32_t lwCode;
    CMD cmd;
    bool fRet = fFalse;

    Clean();
    lwCode = MenuKey(pevt->message & charCodeMask);
    if (SwHigh(lwCode) != 0 && _FGetCmdFromCode(lwCode, &cmd))
    {
        vpcex->EnqueueCmd(&cmd);
        fRet = fTrue;
    }
    HiliteMenu(0);
    return fRet;
}

/***************************************************************************
    Make sure the menu's are clean - ie, items are enabled/disabled/marked
    correctly.  Called immediately before dropping the menus.
***************************************************************************/
void MUB::Clean(void)
{
    AssertThis(0);
    int32_t imnu, imni;
    uint32_t grfeds;
    MNU mnu;
    MNI mni;
    CMD cmd;
    achar st[kcbMaxSt];
    int32_t cch;

    for (imnu = _pglmnu->IvMac(); imnu-- > 0;)
    {
        _pglmnu->Get(imnu, &mnu);
        if ((imni = mnu.pglmni->IvMac()) == 0)
            continue;

        ClearPb(&cmd, size(cmd));
        while (imni-- > 0)
        {
            mnu.pglmni->Get(imni, &mni);
            if (mni.cid == cidNil)
                continue;

            cmd.cid = mni.cid;
            cmd.rglw[0] = mni.lw0;

            if (_FFindMlst(imnu, imni))
            {
                // need the item name in a GG
                GetItem(mnu.hnsmu, imni + 1, (uint8_t *)st);
                cch = (int32_t) * (uint8_t *)st;
                if ((cmd.pgg = GG::PggNew(0, 1, cch)) != pvNil)
                    AssertDo(cmd.pgg->FInsert(0, cch, st + 1), 0);
            }
            grfeds = vpcex->GrfedsForCmd(&cmd);
            ReleasePpo(&cmd.pgg);

            if (grfeds & fedsEnable)
                EnableItem(mnu.hnsmu, imni + 1);
            else if (grfeds & fedsDisable)
                DisableItem(mnu.hnsmu, imni + 1);

            if (grfeds & kgrfedsMark)
            {
                SetItemMark(mnu.hnsmu, imni + 1,
                            (grfeds & fedsCheck)    ? checkMark
                            : (grfeds & fedsBullet) ? diamondMark
                                                    : noMark);
            }
        }
    }
}

/***************************************************************************
    See if the given item is in a list.
***************************************************************************/
bool MUB::_FFindMlst(int32_t imnu, int32_t imni, MLST *pmlst, int32_t *pimlst)
{
    AssertThis(0);
    AssertNilOrVarMem(pmlst);
    AssertNilOrVarMem(pimlst);
    int32_t imlst;
    MLST mlst;

    if (pvNil == _pglmlst)
        return fFalse;

    for (imlst = _pglmlst->IvMac(); imlst-- > 0;)
    {
        _pglmlst->Get(imlst, &mlst);
        if (mlst.imnu == imnu && imni >= mlst.imniBase && (mlst.cmni == cvNil || imni < mlst.imniBase + mlst.cmni))
        {
            if (pvNil != pmlst)
                *pmlst = mlst;
            if (pvNil != pimlst)
                *pimlst = imlst;
            return fTrue;
        }
    }
    TrashVar(pmlst);
    TrashVar(pimlst);
    return fFalse;
}

/***************************************************************************
    Get a command struct for the command from the Mac menu item code.
***************************************************************************/
bool MUB::_FGetCmdFromCode(int32_t lwCode, CMD *pcmd)
{
    AssertThis(0);
    AssertVarMem(pcmd);
    int32_t mid = SwHigh(lwCode);
    int32_t imni = SwLow(lwCode);
    int32_t cmni;
    int32_t imnu, cmnu;
    MNU mnu;
    MNI mni;
    achar st[kcbMaxSt];
    int32_t cch;
    bool fNeedName;
    MLST mlst;

    // the code is one-based
    if (imni-- == 0)
        return fFalse;

    for (imnu = 0, cmnu = _pglmnu->IvMac();; imnu++)
    {
        if (imnu >= cmnu)
            return fFalse;
        _pglmnu->Get(imnu, &mnu);
        if (mnu.mid == mid)
            break;
    }
    cmni = mnu.pglmni->IvMac();

    ClearPb(pcmd, size(*pcmd));
    fNeedName = _FFindMlst(imnu, imni, &mlst);
    if (imni < cmni)
    {
        mnu.pglmni->Get(imni, &mni);
        if (mni.cid == cidNil)
            return fFalse;
        Assert(!fNeedName || mni.cid == mlst.cid, "bad list item");
        pcmd->cid = mni.cid;
        pcmd->rglw[0] = mni.lw0;
    }
    else if (!fNeedName)
        return fFalse;
    else
        pcmd->cid = mlst.cid;

    if (fNeedName)
    {
        // need the item name in a GG
        GetItem(mnu.hnsmu, imni + 1, (uint8_t *)st);
        cch = (int32_t) * (uint8_t *)st;
        if (pvNil == (pcmd->pgg = GG::PggNew(0, 1, cch)))
            return fFalse;
        AssertDo(pcmd->pgg->FInsert(0, cch, st + 1), 0);
    }

    return fTrue;
}

/***************************************************************************
    Adds an item identified by the given list cid, long parameter
    and string.
***************************************************************************/
bool MUB::FAddListCid(int32_t cid, int32_t lw0, PSTZ pstz)
{
    AssertThis(0);
    AssertStz(pstz);
    int32_t imlst;
    MLST mlst;
    int32_t imnuPrev;
    int32_t dimni;
    bool fSeparator;
    bool fRet = fTrue;

    if (pvNil == _pglmlst)
        return fTrue;

    imnuPrev = ivNil;
    dimni = 0;
    for (imlst = 0; imlst < _pglmlst->IvMac(); imlst++)
    {
        _pglmlst->Get(imlst, &mlst);
        if (mlst.imnu == imnuPrev)
        {
            mlst.imniBase += dimni;
            _pglmlst->Put(imlst, &mlst);
        }
        else
        {
            imnuPrev = mlst.imnu;
            dimni = 0;
        }
        if (cid != mlst.cid || mlst.cmni == cvNil)
            goto LAdjustSeparator;

        if (!_FInsertMni(mlst.imnu, mlst.imniBase + mlst.cmni, cid, lw0, pstz))
        {
            fRet = fFalse;
            goto LAdjustSeparator;
        }
        mlst.cmni++;
        _pglmlst->Put(imlst, &mlst);
        dimni++;

    LAdjustSeparator:
        fSeparator = mlst.imniBase > FPure(mlst.fSeparator) && (mlst.cmni > 0 || mlst.cmni == cvNil);
        if (fSeparator && !mlst.fSeparator)
        {
            // add a separator
            if (!_FInsertMni(mlst.imnu, mlst.imniBase, cidNil, 0,
                             "\x2"
                             "(-"))
                fRet = fFalse;
            else
            {
                mlst.imniBase++;
                mlst.fSeparator = fTrue;
                _pglmlst->Put(imlst, &mlst);
                dimni++;
            }
        }
        else if (!fSeparator && mlst.fSeparator)
        {
            // delete a separator
            _DeleteMni(mlst.imnu, --mlst.imniBase);
            mlst.fSeparator = fFalse;
            _pglmlst->Put(imlst, &mlst);
            dimni--;
        }
    }

    return fRet;
}

/***************************************************************************
    Insert a new menu item.
***************************************************************************/
bool MUB::_FInsertMni(int32_t imnu, int32_t imni, int32_t cid, int32_t lw0, PSTZ pstz)
{
    AssertThis(0);
    AssertIn(imnu, 0, _pglmnu->IvMac());
    AssertStz(pstz);
    MNU mnu;
    MNI mni;
    int32_t cmni;

    _pglmnu->Get(imnu, &mnu);
    AssertPo(mnu.pglmni, 0);
    AssertIn(imni, 0, mnu.pglmni->IvMac() + 1);
    mni.cid = cid;
    mni.lw0 = lw0;
    if (!mnu.pglmni->FInsert(imni, &mni))
        return fFalse;
    cmni = CountMItems(mnu.hnsmu);
    InsMenuItem(mnu.hnsmu, (uint8_t *)pstz, (short)imni);
    if (CountMItems(mnu.hnsmu) != cmni + 1)
    {
        mnu.pglmni->Delete(imni);
        return fFalse;
    }
    return fTrue;
}

/***************************************************************************
    Delete a menu item.
***************************************************************************/
void MUB::_DeleteMni(int32_t imnu, int32_t imni)
{
    AssertThis(0);
    AssertIn(imnu, 0, _pglmnu->IvMac());
    MNU mnu;

    _pglmnu->Get(imnu, &mnu);
    AssertPo(mnu.pglmni, 0);
    AssertIn(imni, 0, mnu.pglmni->IvMac());
    mnu.pglmni->Delete(imni);
    DelMenuItem(mnu.hnsmu, imni + 1);
}

/***************************************************************************
    Removes all items identified by the given list cid, and long parameter
    or string.  If pstz is non-nil, it is used to find the item.
    If pstz is nil, lw0 is used to identify the item.
***************************************************************************/
bool MUB::FRemoveListCid(int32_t cid, int32_t lw0, PSTZ pstz)
{
    AssertThis(0);
    AssertNilOrStz(pstz);
    int32_t imlst;
    MLST mlst;
    MNU mnu;
    MNI mni;
    achar st[kcbMaxSt];
    int32_t imnuPrev;
    int32_t dimni, imni;
    bool fSeparator;
    bool fRet = fTrue;

    if (pvNil == _pglmlst)
        return fTrue;

    imnuPrev = ivNil;
    dimni = 0;
    for (imlst = 0; imlst < _pglmlst->IvMac(); imlst++)
    {
        _pglmlst->Get(imlst, &mlst);
        if (mlst.imnu == imnuPrev)
        {
            mlst.imniBase += dimni;
            Assert(mlst.imniBase >= FPure(mlst.fSeparator), "bad imniBase");
            _pglmlst->Put(imlst, &mlst);
        }
        else
        {
            imnuPrev = mlst.imnu;
            dimni = 0;
            _pglmnu->Get(mlst.imnu, &mnu);
        }
        if (cid != mlst.cid || mlst.cmni <= 0)
            goto LAdjustSeparator;

        for (imni = mlst.imniBase; imni < mlst.imniBase + mlst.cmni; imni++)
        {
            if (pvNil == pstz)
            {
                mnu.pglmni->Get(imni, &mni);
                Assert(mni.cid == mlst.cid, "bad mni");
                if (mni.lw0 != lw0)
                    continue;
            }
            else
            {
                GetItem(mnu.hnsmu, imni + 1, (uint8_t *)st);
                if (!FEqualSt(st, pstz))
                    continue;
            }
            _DeleteMni(mlst.imnu, imni--);
            mlst.cmni--;
            _pglmlst->Put(imlst, &mlst);
            dimni--;
        }

    LAdjustSeparator:
        fSeparator = mlst.imniBase > FPure(mlst.fSeparator) && (mlst.cmni > 0 || mlst.cmni == cvNil);
        if (fSeparator && !mlst.fSeparator)
        {
            // add a separator
            if (!_FInsertMni(mlst.imnu, mlst.imniBase, cidNil, 0,
                             "\x2"
                             "(-"))
                fRet = fFalse;
            else
            {
                mlst.imniBase++;
                mlst.fSeparator = fTrue;
                _pglmlst->Put(imlst, &mlst);
                dimni++;
            }
        }
        else if (!fSeparator && mlst.fSeparator)
        {
            // delete a separator
            _DeleteMni(mlst.imnu, --mlst.imniBase);
            mlst.fSeparator = fFalse;
            _pglmlst->Put(imlst, &mlst);
            dimni--;
        }
    }

    return fRet;
}

/***************************************************************************
    Removes all items identified by the given list cid.
***************************************************************************/
bool MUB::FRemoveAllListCid(int32_t cid)
{
    AssertThis(0);
    int32_t imlst;
    MLST mlst;
    int32_t imnuPrev;
    int32_t dimni;
    bool fSeparator;
    bool fRet = fTrue;

    if (pvNil == _pglmlst)
        return fTrue;

    imnuPrev = ivNil;
    dimni = 0;
    for (imlst = 0; imlst < _pglmlst->IvMac(); imlst++)
    {
        _pglmlst->Get(imlst, &mlst);
        if (mlst.imnu == imnuPrev)
        {
            mlst.imniBase += dimni;
            Assert(mlst.imniBase >= FPure(mlst.fSeparator), "bad imniBase");
            _pglmlst->Put(imlst, &mlst);
        }
        else
        {
            imnuPrev = mlst.imnu;
            dimni = 0;
        }
        if (cid == mlst.cid)
        {
            while (mlst.cmni > 0)
            {
                _DeleteMni(mlst.imnu, mlst.imniBase);
                mlst.cmni--;
                dimni--;
            }
        }

        fSeparator = mlst.imniBase > FPure(mlst.fSeparator) && (mlst.cmni > 0 || mlst.cmni == cvNil);
        if (fSeparator && !mlst.fSeparator)
        {
            // add a separator
            if (!_FInsertMni(mlst.imnu, mlst.imniBase, cidNil, 0,
                             "\x2"
                             "(-"))
                fRet = fFalse;
            else
            {
                mlst.imniBase++;
                mlst.fSeparator = fTrue;
                _pglmlst->Put(imlst, &mlst);
                dimni++;
            }
        }
        else if (!fSeparator && mlst.fSeparator)
        {
            // delete a separator
            _DeleteMni(mlst.imnu, --mlst.imniBase);
            mlst.fSeparator = fFalse;
            _pglmlst->Put(imlst, &mlst);
            dimni--;
        }
    }

    return fRet;
}

/***************************************************************************
    Changes the long parameter and the menu text associated with a menu
    list item.  If pstzOld is non-nil, it is used to find the item.
    If pstzOld is nil, lwOld is used to identify the item.  In either case
    lwNew is set as the new long parameter and if pstzNew is non-nil,
    it is used as the new menu item text.
***************************************************************************/
bool MUB::FChangeListCid(int32_t cid, int32_t lwOld, PSTZ pstzOld, int32_t lwNew, PSTZ pstzNew)
{
    AssertThis(0);
    AssertNilOrStz(pstzOld);
    AssertNilOrStz(pstzNew);
    int32_t imlst;
    MLST mlst;
    int32_t imni;
    MNI mni;
    MNU mnu;
    achar st[kcbMaxSt];

    if (pvNil == _pglmlst)
        return fTrue;

    for (imlst = 0; imlst < _pglmlst->IvMac(); imlst++)
    {
        _pglmlst->Get(imlst, &mlst);
        if (cid != mlst.cid || mlst.cmni <= 0)
            continue;

        for (imni = mlst.imniBase; imni < mlst.imniBase + mlst.cmni; imni++)
        {
            _pglmnu->Get(mlst.imnu, &mnu);
            if (pvNil == pstzOld)
            {
                mnu.pglmni->Get(imni, &mni);
                if (mni.lw0 != lwOld)
                    continue;
            }
            else
            {
                GetItem(mnu.hnsmu, imni + 1, (uint8_t *)st);
                if (!FEqualSt(st, pstzOld))
                    continue;
                mnu.pglmni->Get(imni, &mni);
            }
            mni.lw0 = lwNew;
            mnu.pglmni->Put(imni, &mni);
            if (pvNil != pstzNew)
            {
                // change the string
                SetItem(mnu.hnsmu, imni + 1, (uint8_t *)pstzNew);
            }
        }
    }

    return fTrue;
}

#ifdef DEBUG
/***************************************************************************
    Mark mem used by the menu bar.
***************************************************************************/
void MUB::MarkMem(void)
{
    AssertThis(0);
    int32_t imnu;
    MNU mnu;

    MUB_PAR::MarkMem();
    MarkMemObj(_pglmnu);
    if (_pglmnu == pvNil || (imnu = _pglmnu->IvMac()) == 0)
        return;

    while (imnu--)
    {
        _pglmnu->Get(imnu, &mnu);
        MarkMemObj(mnu.pglmni);
    }
}
#endif // DEBUG
