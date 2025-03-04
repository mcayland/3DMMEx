/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Rich text document and associated DDG.

***************************************************************************/
#include "frame.h"
ASSERTNAME

RTCLASS(TXTB)
RTCLASS(TXPD)
RTCLASS(TXRD)
RTCLASS(TXTG)
RTCLASS(TXLG)
RTCLASS(TXRG)
RTCLASS(RTUN)

BEGIN_CMD_MAP(TXRG, DDG)
ON_CID_GEN(cidPlain, &TXRG::FCmdApplyProperty, &TXRG::FEnablePropCmd)
ON_CID_GEN(cidBold, &TXRG::FCmdApplyProperty, &TXRG::FEnablePropCmd)
ON_CID_GEN(cidItalic, &TXRG::FCmdApplyProperty, &TXRG::FEnablePropCmd)
ON_CID_GEN(cidUnderline, &TXRG::FCmdApplyProperty, &TXRG::FEnablePropCmd)
ON_CID_GEN(cidJustifyLeft, &TXRG::FCmdApplyProperty, &TXRG::FEnablePropCmd)
ON_CID_GEN(cidJustifyCenter, &TXRG::FCmdApplyProperty, &TXRG::FEnablePropCmd)
ON_CID_GEN(cidJustifyRight, &TXRG::FCmdApplyProperty, &TXRG::FEnablePropCmd)
ON_CID_GEN(cidIndentNone, &TXRG::FCmdApplyProperty, &TXRG::FEnablePropCmd)
ON_CID_GEN(cidIndentFirst, &TXRG::FCmdApplyProperty, &TXRG::FEnablePropCmd)
ON_CID_GEN(cidIndentRest, &TXRG::FCmdApplyProperty, &TXRG::FEnablePropCmd)
ON_CID_GEN(cidIndentAll, &TXRG::FCmdApplyProperty, &TXRG::FEnablePropCmd)
ON_CID_GEN(cidChooseFont, &TXRG::FCmdApplyProperty, &TXRG::FEnablePropCmd)
ON_CID_GEN(cidChooseFontSize, &TXRG::FCmdApplyProperty, &TXRG::FEnablePropCmd)
ON_CID_GEN(cidChooseSubSuper, &TXRG::FCmdApplyProperty, &TXRG::FEnablePropCmd)
END_CMD_MAP_NIL()

#ifdef DEBUG
/***************************************************************************
    Assert the validity of the rich text doc.
***************************************************************************/
void TXTB::AssertValid(uint32_t grfobj)
{
    achar ch;
    int32_t ibMac;
    int32_t cpMac;

    TXTB_PAR::AssertValid(grfobj);
    AssertPo(_pbsf, 0);
    ibMac = _pbsf->IbMac();
    cpMac = ibMac / SIZEOF(achar);
    Assert(ibMac % SIZEOF(achar) == 0, "ibMac not divisible by SIZEOF(achar)");
    AssertIn(ibMac, SIZEOF(achar), kcbMax);
    AssertIn(_cpMinCache, 0, cpMac + 1);
    AssertIn(_cpLimCache, 0, cpMac + 1);
    AssertIn(_cpLimCache, _cpMinCache, _cpMinCache + SIZEOF(_rgchCache) + 1);
    AssertNilOrPo(_pfil, 0);
    if (grfobj & fobjAssertFull)
    {
        _pbsf->FetchRgb(ibMac - SIZEOF(achar), SIZEOF(achar), &ch);
        Assert(ch == kchReturn, "stream doesn't end with a carriage return");
    }
}

/***************************************************************************
    Mark memory for the TXTB.
***************************************************************************/
void TXTB::MarkMem(void)
{
    AssertThis(fobjAssertFull);
    TXTB_PAR::MarkMem();
    MarkMemObj(_pbsf);
}
#endif // DEBUG

/***************************************************************************
    Constructor for the base text document class
***************************************************************************/
TXTB::TXTB(PDOCB pdocb, uint32_t grfdoc) : TXTB_PAR(pdocb, grfdoc)
{
    _acrBack = kacrWhite;
    _dxpDef = kdxpDocDef;
}

/***************************************************************************
    Destructor for the base text document class
***************************************************************************/
TXTB::~TXTB(void)
{
    ReleasePpo(&_pbsf);
    ReleasePpo(&_pfil);
}

/***************************************************************************
    Initializer for the base text document class.
***************************************************************************/
bool TXTB::_FInit(PFNI pfni, PBSF pbsf, int16_t osk)
{
    AssertNilOrPo(pfni, ffniFile);
    AssertNilOrPo(pbsf, 0);
    achar ch;

    if (pvNil != pfni && pvNil == (_pfil = FIL::PfilOpen(pfni)))
        return fFalse;

    if (pvNil != pbsf)
    {
        if (osk != koskCur)
        {
            Bug("Can't translate a BSF");
            return fFalse;
        }
        if (pbsf->IbMac() % SIZEOF(achar) != 0)
        {
            Bug("BSF has a partial character");
            return fFalse;
        }
        pbsf->AddRef();
        _pbsf = pbsf;
    }
    else if (pvNil == (_pbsf = NewObj BSF))
        return fFalse;
    else if (!_FLoad(osk))
        return fFalse;

    // append a return character
    ch = kchReturn;
    if (!_pbsf->FReplace(&ch, SIZEOF(achar), _pbsf->IbMac(), 0))
        return fFalse;

    AssertThis(fobjAssertFull);
    return fTrue;
}

/***************************************************************************
    Load the document from its file
***************************************************************************/
bool TXTB::_FLoad(int16_t osk)
{
    // initialize the BSF to just point to the file
    FLO flo;
    bool fRet = fFalse;

    if (pvNil == (flo.pfil = _pfil) || 0 == (flo.cb = _pfil->FpMac()))
        return fTrue;

    flo.pfil->AddRef();
    flo.fp = 0;
    if (flo.FTranslate(osk))
    {
        fRet = flo.cb == 0 || _pbsf->FReplaceFlo(&flo, fFalse, 0, _pbsf->IbMac());
    }

    ReleasePpo(&flo.pfil);
    return fRet;
}

/***************************************************************************
    Return the length of the text in the text document.
***************************************************************************/
int32_t TXTB::CpMac(void)
{
    // Note: we don't do an AssertThis(0) here for debug performance
    AssertThisMem();
    AssertVarMem(_pbsf);
    Assert(_pbsf->IbMac() % SIZEOF(achar) == 0, "IbMac() not divisible by SIZEOF(achar)");

    return _pbsf->IbMac() / SIZEOF(achar);
}

/***************************************************************************
    Suspend undo. This increments a count.
***************************************************************************/
void TXTB::SuspendUndo(void)
{
    AssertThis(0);
    _cactSuspendUndo++;
}

/***************************************************************************
    Resume undo. This decrements a count.
***************************************************************************/
void TXTB::ResumeUndo(void)
{
    AssertThis(0);
    Assert(_cactSuspendUndo > 0, "bad _cactSuspendUndo");
    _cactSuspendUndo--;
}

/***************************************************************************
    Set up undo for an action. If this succeeds, you must call either
    CancelUndo or CommitUndo. Default TXTB doesn't create an undo record.
***************************************************************************/
bool TXTB::FSetUndo(int32_t cp1, int32_t cp2, int32_t ccpIns)
{
    AssertThis(0);
    AssertIn(cp1, 0, CpMac() + 1);
    AssertIn(cp2, 0, CpMac() + 1);
    AssertIn(ccpIns, 0, kcbMax);

    _cactSuspendUndo++;
    return fTrue;
}

/***************************************************************************
    Cancel undo.
***************************************************************************/
void TXTB::CancelUndo(void)
{
    ResumeUndo();
}

/***************************************************************************
    Commit the setup undo.
***************************************************************************/
void TXTB::CommitUndo(void)
{
    ResumeUndo();
}

/***************************************************************************
    Increments the _cactCombineUndo counter. This is used to determine
    whether the new undo record can be combined with the previous one
    (during typing etc).
***************************************************************************/
void TXTB::BumpCombineUndo(void)
{
    AssertThis(0);
    if (_cactSuspendUndo <= 0)
        _cactCombineUndo++;
}

/***************************************************************************
    Search for some text. If fCaseSensitive is false, we map prgch to all
    lower case. The pcpLim parameter is in case we support regular
    expressions in the future.
***************************************************************************/
bool TXTB::FFind(const achar *prgch, int32_t cch, int32_t cpMin, int32_t *pcpMin, int32_t *pcpLim, bool fCaseSensitive)
{
    AssertThis(fobjAssertFull);
    AssertIn(cch, 1, kcbMax);
    AssertPvCb(prgch, cch * SIZEOF(achar));
    AssertIn(cpMin, 0, CpMac() + 1);
    AssertVarMem(pcpMin);
    AssertVarMem(pcpLim);
    const int32_t kcbCharSet = 256 / 8;
    uint8_t grfbitUsed[kcbCharSet];
    int32_t ibit;
    const achar *pch, *pchLast;
    achar ch;
    int32_t cpT, cpMac;

    if (!fCaseSensitive)
    {
        // TODO: Can't convert this string to lowercase as it's now constant
        // LowerRgch(prgch, cch);
        RawRtn();
    }

    cch--;
    pchLast = prgch + cch;

    // calculate the grfbitUsed bit field - indicating, which characters
    // appear in the search string.
    ClearPb(grfbitUsed, SIZEOF(grfbitUsed));
    for (pch = prgch; pch <= pchLast; pch++)
    {
        ibit = (int32_t)(uint8_t)*pch;
        AssertIn(ibit, 0, 256);
        grfbitUsed[IbFromIbit(ibit)] |= Fbit(ibit);
    }

    for (cpMac = CpMac(); (cpT = cpMin + cch) < cpMac;)
    {
        for (pch = pchLast;; cpT--, pch--)
        {
            // get the character and map it to lower case if we're doing
            // a case insensitive search
            ch = _ChFetch(cpT);
            if (!fCaseSensitive)
                ch = ChLower(ch);

            // see if the character is used anywhere within the search string
            ibit = (int32_t)(uint8_t)ch;
            if (!(grfbitUsed[IbFromIbit(ibit)] & Fbit(ibit)))
            {
                // this character isn't anywhere in the search string,
                // so we can jump by a lot.
                cpMin = cpT + 1;
                break;
            }

            // see if the character matches the corresponding character in
            // the search string
            if (ch != *pch)
            {
                // character doesn't match, just jump by one
                cpMin++;
                break;
            }

            // if cpT is back to cpMin, we've got a match
            if (cpT == cpMin)
            {
                // we matched
                *pcpMin = cpMin;
                *pcpLim = cpMin + cch + 1;
                return fTrue;
            }
        }
    }

    TrashVar(pcpMin);
    TrashVar(pcpLim);
    return fFalse;
}

/***************************************************************************
    Turn the selection of this TXTB's active DDG off.
***************************************************************************/
void TXTB::HideSel(void)
{
    AssertThis(0);
    PDDG pddg;

    if (pvNil != (pddg = PddgActive()) && pddg->FIs(kclsTXTG))
        ((PTXTG)pddg)->HideSel();
}

/***************************************************************************
    Set the selection of this TXTB's active DDG to the given range.
***************************************************************************/
void TXTB::SetSel(int32_t cpAnchor, int32_t cpOther, int32_t gin)
{
    AssertThis(0);
    PDDG pddg;

    if (pvNil != (pddg = PddgActive()) && pddg->FIs(kclsTXTG))
        ((PTXTG)pddg)->SetSel(cpAnchor, cpOther, gin);
}

/***************************************************************************
    Make sure the selection of this TXTB's acttive DDG is visible (at least
    the _cpOther end of it).
***************************************************************************/
void TXTB::ShowSel(void)
{
    AssertThis(0);
    PDDG pddg;

    if (pvNil != (pddg = PddgActive()) && pddg->FIs(kclsTXTG))
        ((PTXTG)pddg)->ShowSel();
}

/***************************************************************************
    Fetch a character of the stream through the cache.
***************************************************************************/
void TXTB::_CacheRange(int32_t cpMin, int32_t cpLim)
{
    AssertThis(0);
    AssertIn(cpMin, 0, CpMac() + 1);
    AssertIn(cpLim, cpMin, CpMac() + 1);
    AssertIn(cpLim - cpMin, 0, kcchMaxTxtbCache + 1);
    int32_t cpMac;

    if (cpMin >= _cpMinCache && cpLim <= _cpLimCache)
        return;

    cpMac = CpMac();
    if (_cpLimCache <= _cpMinCache)
    {
        // nothing currently cached
        goto LNewCache;
    }

    if (cpMin < _cpMinCache)
    {
        int32_t cpT;

        if (_cpMinCache >= cpMin + kcchMaxTxtbCache)
            goto LNewCache;

        // keep front end of old stuff
        cpT = LwMax(0, LwMin(cpMin, _cpLimCache - kcchMaxTxtbCache));
        _cpLimCache = LwMin(cpT + kcchMaxTxtbCache, _cpLimCache);
        BltPb(_rgchCache, _rgchCache + (_cpMinCache - cpT), (_cpLimCache - _cpMinCache) * SIZEOF(achar));
        _pbsf->FetchRgb(cpT * SIZEOF(achar), (_cpMinCache - cpT) * SIZEOF(achar), _rgchCache);
        _cpMinCache = cpT;
    }
    else
    {
        Assert(cpLim > _cpLimCache, 0);
        if (cpLim >= _cpLimCache + kcchMaxTxtbCache)
        {
        LNewCache:
            // just cache around [cpMin, cpLim)
            _cpMinCache = LwMax(0, LwMin((cpMin + cpLim - kcchMaxTxtbCache) / 2, cpMac - kcchMaxTxtbCache));
            _cpLimCache = LwMin(_cpMinCache + kcchMaxTxtbCache, cpMac);
            _pbsf->FetchRgb(_cpMinCache * SIZEOF(achar), (_cpLimCache - _cpMinCache) * SIZEOF(achar), _rgchCache);
        }
        else
        {
            // keep back end old stuff
            int32_t cpLimCache, cpMinCache;

            cpLimCache = LwMin(cpMac, LwMax(cpLim, _cpMinCache + kcchMaxTxtbCache));
            cpMinCache = LwMax(cpLimCache - kcchMaxTxtbCache, _cpMinCache);
            if (cpMinCache != _cpMinCache)
            {
                BltPb(_rgchCache + (cpMinCache - _cpMinCache), _rgchCache, (_cpLimCache - cpMinCache) * SIZEOF(achar));
            }
            _pbsf->FetchRgb(_cpLimCache * SIZEOF(achar), (cpLimCache - _cpLimCache) * SIZEOF(achar),
                            _rgchCache + _cpLimCache - cpMinCache);
            _cpMinCache = cpMinCache;
            _cpLimCache = cpLimCache;
        }
    }
}

/***************************************************************************
    Characters have changed, so fix the cache.
***************************************************************************/
void TXTB::_InvalCache(int32_t cp, int32_t ccpIns, int32_t ccpDel)
{
    int32_t dcpFront, dcpBack;

    if (_cpLimCache <= cp || _cpLimCache <= _cpMinCache || ccpIns == 0 && ccpDel == 0)
    {
        // cache is before the edit or cache is already empty
        return;
    }

    if (_cpMinCache >= cp + ccpDel)
    {
        // cache is after the edit
        _cpMinCache += ccpIns - ccpDel;
        _cpLimCache += ccpIns - ccpDel;
        return;
    }

    dcpFront = cp - _cpMinCache;
    dcpBack = _cpLimCache - cp - ccpDel;
    if (dcpFront <= 0 && dcpBack <= 0)
        _cpMinCache = _cpLimCache = 0;
    else if (dcpFront >= dcpBack)
    {
        // keep the front end of the cache
        _cpLimCache = cp;
    }
    else
    {
        // keep the tail end of the cache
        BltPb(_rgchCache + cp + ccpDel - _cpMinCache, _rgchCache, dcpBack * SIZEOF(achar));
        _cpMinCache = cp + ccpIns;
        _cpLimCache = _cpMinCache + dcpBack;
    }
}

/***************************************************************************
    Fetch a character of the stream through the cache.
***************************************************************************/
achar TXTB::_ChFetch(int32_t cp)
{
    AssertThis(0);
    AssertIn(cp, 0, CpMac() + 1);

    if (!FIn(cp, _cpMinCache, _cpLimCache))
    {
        // not a cache hit
        _CacheRange(cp, cp + 1);
    }

    return _rgchCache[cp - _cpMinCache];
}

/***************************************************************************
    Fetch some characters from the text document.
***************************************************************************/
void TXTB::FetchRgch(int32_t cp, int32_t ccp, achar *prgch)
{
    AssertThis(0);
    AssertIn(cp, 0, CpMac() + 1);
    AssertIn(ccp, 0, CpMac() - cp + 1);
    AssertPvCb(prgch, ccp * SIZEOF(achar));
    int32_t ccpT;

    while (ccp > 0)
    {
        ccpT = LwMin(ccp, kcchMaxTxtbCache);
        _CacheRange(cp, cp + ccpT);
        CopyPb(_rgchCache + cp - _cpMinCache, prgch, ccpT * SIZEOF(achar));
        prgch += ccpT;
        cp += ccpT;
        ccp -= ccpT;
    }
}

/***************************************************************************
    Returns non-zero iff cp is the beginning of a paragraph.
***************************************************************************/
bool TXTB::FMinPara(int32_t cp)
{
    AssertThis(0);
    uint32_t grfch;

    if (cp <= 0)
        return cp == 0;
    if (cp >= CpMac())
        return fFalse;
    if (GrfchFromCh(_ChFetch(cp)) & fchIgnore)
        return fFalse;

    // return true iff the first non-ignore character we see
    // is a break character
    AssertIn(cp, 1, CpMac());
    while (cp-- > 0)
    {
        grfch = GrfchFromCh(_ChFetch(cp));
        if (fchBreak & grfch)
            return fTrue;
        if (!(fchIgnore & grfch))
            return fFalse;
    }

    // just line feeds!
    Warn("isolated line feeds");
    return fFalse;
}

/***************************************************************************
    Find the beginning of the paragraph that cp is in. If cp <= 0,
    returns 0. If cp >= CpMac(), returns the beginning of the last
    paragraph.
***************************************************************************/
int32_t TXTB::CpMinPara(int32_t cp)
{
    AssertThis(0);
    uint32_t grfch;
    int32_t cpOrig;
    int32_t dcpLine = 1;

    if (cp <= 0)
        return 0;

    if (cp >= CpMac())
        cp = CpMac() - 1;
    else if (GrfchFromCh(_ChFetch(cp)) & fchIgnore)
        dcpLine++;
    cpOrig = cp;

    AssertIn(cp, 1, CpMac());
    while (cp-- > 0)
    {
        grfch = GrfchFromCh(_ChFetch(cp));
        if (fchBreak & grfch)
        {
            if (cp + dcpLine <= cpOrig)
                return cp + dcpLine;
            dcpLine = 1;
        }
        else if (fchIgnore & grfch)
            dcpLine++;
        else
            dcpLine = 1;
    }

    return 0;
}

/***************************************************************************
    Find the end of the paragraph that cp is in. If cp < 0, returns 0.
    If cp >= CpMac(), returns CpMac().
***************************************************************************/
int32_t TXTB::CpLimPara(int32_t cp)
{
    AssertThis(0);
    uint32_t grfch;
    bool fCr = fFalse;
    int32_t cpLim = CpMac();

    if (cp < 0)
        return 0;
    if (cp >= cpLim)
        return cpLim;

    // cp should now be a legal index into the character stream
    AssertIn(cp, 0, cpLim);
    while (cp > 0 && (GrfchFromCh(_ChFetch(cp)) & fchIgnore))
        cp--;

    for (; cp < cpLim; cp++)
    {
        grfch = GrfchFromCh(_ChFetch(cp));
        if (fchBreak & grfch)
        {
            if (fCr)
                return cp;
            fCr = fTrue;
        }
        else if (!(fchIgnore & grfch) && fCr)
            return cp;
    }
    Assert(fCr, "last character is not a return character");
    return cpLim;
}

/***************************************************************************
    Return cp of the previous character, skipping line feed characters. If
    fWord is true, skip to the beginning of a word.
***************************************************************************/
int32_t TXTB::CpPrev(int32_t cp, bool fWord)
{
    AssertThis(0);
    AssertIn(cp, 0, CpMac() + 1);

    if (!fWord)
    {
        while (cp > 0 && (fchIgnore & GrfchFromCh(_ChFetch(--cp))))
            ;
    }
    else
    {
        while (cp > 0 && ((fchIgnore | fchMayBreak) & GrfchFromCh(_ChFetch(cp - 1))))
        {
            cp--;
        }
        while (cp > 0 && !((fchIgnore | fchMayBreak) & GrfchFromCh(_ChFetch(cp - 1))))
        {
            cp--;
        }
    }

    return cp;
}

/***************************************************************************
    Return cp of the next character, skipping line feed characters. If
    fWord is true, skip to the beginning of the next word.
***************************************************************************/
int32_t TXTB::CpNext(int32_t cp, bool fWord)
{
    AssertThis(0);
    AssertIn(cp, 0, CpMac() + 1);
    int32_t cpMac = CpMac();

    if (cp >= cpMac)
        return cpMac;

    if (!fWord)
    {
        while (++cp < cpMac && (fchIgnore & GrfchFromCh(_ChFetch(cp))))
            ;
    }
    else
    {
        while (cp < cpMac && !((fchIgnore | fchMayBreak) & GrfchFromCh(_ChFetch(cp))))
        {
            cp++;
        }
        while (cp < cpMac && ((fchIgnore | fchMayBreak) & GrfchFromCh(_ChFetch(cp))))
        {
            cp++;
        }
    }
    return cp;
}

/***************************************************************************
    Invalidate all DDGs on this text doc. Also dirties the document.
    Should be called by any code that edits the document.
***************************************************************************/
void TXTB::InvalAllDdg(int32_t cp, int32_t ccpIns, int32_t ccpDel, uint32_t grfdoc)
{
    AssertThis(0);
    AssertIn(cp, 0, CpMac() + 1);
    AssertIn(ccpIns, 0, CpMac() + 1 - cp);
    AssertIn(ccpDel, 0, kcbMax);
    int32_t ipddg;
    PDDG pddg;

    // mark the document dirty
    SetDirty();
    if (fdocNil == grfdoc)
        return;

    // inform the DDGs
    for (ipddg = 0; pvNil != (pddg = PddgGet(ipddg)); ipddg++)
    {
        if ((grfdoc & fdocUpdate) && pddg->FIs(kclsTXTG))
            ((PTXTG)pddg)->InvalCp(cp, ccpIns, ccpDel);
        else
            pddg->InvalRc(pvNil);
    }
}

/***************************************************************************
    Set the background color of the document.
***************************************************************************/
void TXTB::SetAcrBack(ACR acr, uint32_t grfdoc)
{
    AssertThis(0);
    AssertPo(&acr, 0);
    Assert(acr != kacrInvert, "can't use invert as a background color");

    if (_acrBack != acr)
    {
        _acrBack = acr;
        SetDirty();
        if (grfdoc & (fdocUpdate | fdocInval))
            InvalAllDdg(0, 0, 0, fdocInval);
    }
}

/***************************************************************************
    Set the default width of the document.
***************************************************************************/
void TXTB::SetDxpDef(int32_t dxp)
{
    AssertThis(0);
    AssertIn(dxp, 1, kcbMax);
    int32_t cpMac;

    if (_dxpDef == dxp)
        return;

    _dxpDef = dxp;
    SetDirty();
    HideSel();
    cpMac = CpMac();
    InvalAllDdg(0, cpMac, cpMac);
}

/***************************************************************************
    Replace cp to cp + ccpDel with ccpIns characters from prgch. If ccpIns
    is zero, prgch can be nil. The last character should never be replaced.
***************************************************************************/
bool TXTB::FReplaceRgch(const void *prgch, int32_t ccpIns, int32_t cp, int32_t ccpDel, uint32_t grfdoc)
{
    AssertThis(fobjAssertFull);
    AssertIn(ccpIns, 0, kcbMax);
    AssertPvCb(prgch, ccpIns * SIZEOF(achar));
    AssertIn(cp, 0, CpMac());
    AssertIn(ccpDel, 0, CpMac() - cp);

    if (!_pbsf->FReplace(prgch, ccpIns * SIZEOF(achar), cp * SIZEOF(achar), ccpDel * SIZEOF(achar)))
    {
        return fFalse;
    }
    _InvalCache(cp, ccpIns, ccpDel);
    AssertThis(fobjAssertFull);
    InvalAllDdg(cp, ccpIns, ccpDel, grfdoc);
    return fTrue;
}

/***************************************************************************
    Replace cp to cp + ccpDel with the characters in the given FLO.
***************************************************************************/
bool TXTB::FReplaceFlo(PFLO pflo, bool fCopy, int32_t cp, int32_t ccpDel, int16_t osk, uint32_t grfdoc)
{
    AssertThis(fobjAssertFull);
    AssertPo(pflo, 0);
    AssertIn(cp, 0, CpMac());
    AssertIn(ccpDel, 0, CpMac() - cp);
    bool fRet = fFalse;
    FLO flo = *pflo;

    flo.pfil->AddRef();
    if (flo.FTranslate(osk) &&
        _pbsf->FReplaceFlo(&flo, fCopy && flo.pfil == pflo->pfil, cp * SIZEOF(achar), ccpDel * SIZEOF(achar)))
    {
        _InvalCache(cp, flo.cb / SIZEOF(achar), ccpDel);
        AssertThis(fobjAssertFull);
        InvalAllDdg(cp, flo.cb / SIZEOF(achar), ccpDel, grfdoc);
        fRet = fTrue;
    }

    ReleasePpo(&flo.pfil);
    return fRet;
}

/***************************************************************************
    Replace cp to cpDst + ccpDel with ccpSrc characters from pbsfSrc starting
    at cpSrc.
***************************************************************************/
bool TXTB::FReplaceBsf(PBSF pbsfSrc, int32_t cpSrc, int32_t ccpSrc, int32_t cpDst, int32_t ccpDel, uint32_t grfdoc)
{
    AssertThis(fobjAssertFull);
    AssertPo(pbsfSrc, 0);
    AssertIn(cpSrc, 0, pbsfSrc->IbMac() / SIZEOF(achar) + 1);
    AssertIn(ccpSrc, 0, pbsfSrc->IbMac() / SIZEOF(achar) + 1 - cpSrc);
    AssertIn(cpDst, 0, CpMac());
    AssertIn(ccpDel, 0, CpMac() - cpDst);

    if (!_pbsf->FReplaceBsf(pbsfSrc, cpSrc * SIZEOF(achar), ccpSrc * SIZEOF(achar), cpDst * SIZEOF(achar),
                            ccpDel * SIZEOF(achar)))
    {
        return fFalse;
    }
    _InvalCache(cpDst, ccpSrc, ccpDel);
    AssertThis(fobjAssertFull);
    InvalAllDdg(cpDst, ccpSrc, ccpDel, grfdoc);
    return fTrue;
}

/***************************************************************************
    Replace cp to cpDst + ccpDel with ccpSrc characters from ptxtbSrc starting
    at cpSrc.
***************************************************************************/
bool TXTB::FReplaceTxtb(PTXTB ptxtbSrc, int32_t cpSrc, int32_t ccpSrc, int32_t cpDst, int32_t ccpDel, uint32_t grfdoc)
{
    AssertPo(ptxtbSrc, 0);
    AssertIn(cpSrc, 0, ptxtbSrc->CpMac() + 1);
    AssertIn(ccpSrc, 0, ptxtbSrc->CpMac() + 1 - cpSrc);

    return FReplaceBsf(ptxtbSrc->_pbsf, cpSrc, ccpSrc, cpDst, ccpDel, grfdoc);
}

/***************************************************************************
    Get the bounds of an object - since plain text doesn't have objects,
    just return false.
***************************************************************************/
bool TXTB::FGetObjectRc(int32_t cp, PGNV pgnv, PCHP pchp, RC *prc)
{
    AssertThis(0);
    AssertIn(cp, 0, CpMac());
    AssertPo(pgnv, 0);
    AssertVarMem(pchp);
    AssertVarMem(prc);

    TrashVar(prc);
    return fFalse;
}

/***************************************************************************
    Draw an object - since plain text doesn't have objects, just return
    false.
***************************************************************************/
bool TXTB::FDrawObject(int32_t cp, PGNV pgnv, int32_t *pxp, int32_t yp, PCHP pchp, RC *prcClip)
{
    AssertThis(0);
    AssertIn(cp, 0, CpMac());
    AssertPo(pgnv, 0);
    AssertVarMem(pxp);
    AssertVarMem(pchp);
    AssertVarMem(prcClip);

    return fFalse;
}

/***************************************************************************
    Get the current FNI for the doc. Return false if the doc is not
    currently based on an FNI (it's a new doc or an internal one).
***************************************************************************/
bool TXTB::FGetFni(FNI *pfni)
{
    AssertThis(0);
    AssertBasePo(pfni, 0);

    if (pvNil == _pfil || _pfil->FTemp())
        return fFalse;

    _pfil->GetFni(pfni);
    return fTrue;
}

/***************************************************************************
    Export the text.
***************************************************************************/
void TXTB::ExportFormats(PCLIP pclip)
{
    AssertThis(0);
    AssertPo(pclip, 0);
    void *pv;
    int32_t ccp = CpMac() - 1;

    if (ccp <= 0)
        return;

    if (!pclip->FInitExport())
        return;

    if (pvNil != (pv = pclip->PvExport(ccp * SIZEOF(achar), kclfmText)))
        _pbsf->FetchRgb(0, ccp * SIZEOF(achar), pv);

    pclip->EndExport();
}

/***************************************************************************
    Constructor for plain text doc.
***************************************************************************/
TXPD::TXPD(PDOCB pdocb, uint32_t grfdoc) : TXPD_PAR(pdocb, grfdoc)
{
}

/***************************************************************************
    Static method to create a new plain text document.
***************************************************************************/
PTXPD TXPD::PtxpdNew(PFNI pfni, PBSF pbsf, int16_t osk, PDOCB pdocb, uint32_t grfdoc)
{
    AssertNilOrPo(pfni, ffniFile);
    AssertNilOrPo(pbsf, 0);
    AssertNilOrPo(pdocb, 0);
    PTXPD ptxpd;

    if (pvNil != (ptxpd = NewObj TXPD(pdocb, grfdoc)) && !ptxpd->_FInit(pfni, pbsf, osk))
    {
        ReleasePpo(&ptxpd);
    }
    return ptxpd;
}

/***************************************************************************
    Create a new TXLG to display the TXPD.
***************************************************************************/
PDDG TXPD::PddgNew(PGCB pgcb)
{
    AssertThis(fobjAssertFull);
    int32_t onn = vpappb->OnnDefFixed();
    int32_t dypFont = vpappb->DypTextDef();

    return TXLG::PtxlgNew(this, pgcb, onn, fontNil, dypFont, 4);
}

/***************************************************************************
    Save the document and optionally set this fni as the current one.
    If the doc is currently based on an FNI, pfni may be nil, indicating
    that this is a normal save (not save as). If pfni is not nil and
    fSetFni is false, this just writes a copy of the doc but doesn't change
    the doc one bit.
***************************************************************************/
bool TXPD::FSaveToFni(FNI *pfni, bool fSetFni)
{
    AssertThis(fobjAssertFull);
    AssertNilOrPo(pfni, ffniFile);
    FLO flo;
    FNI fniT;

    if (pvNil == pfni)
    {
        if (pvNil == _pfil)
        {
            Bug("Can't do a normal save - no file");
            return fFalse;
        }
        _pfil->GetFni(&fniT);
        pfni = &fniT;
        fSetFni = fTrue;
    }

    if (pvNil == (flo.pfil = FIL::PfilCreateTemp(pfni)))
        goto LFail;

    flo.fp = 0;
    flo.cb = _pbsf->IbMac() - SIZEOF(achar);
#ifdef UNICODE
    flo.cb += SIZEOF(achar);
    achar ch;

    ch = kchwUnicode;
    if (!flo.FWriteRgb(&ch, SIZEOF(achar), 0))
        goto LFail;
    flo.fp += SIZEOF(achar);
    flo.cb -= SIZEOF(achar);
#endif // UNICODE

    if (!_pbsf->FWriteRgb(&flo))
        goto LFail;

    // redirect the BSF to the new file
    if (fSetFni)
        _pbsf->FReplaceFlo(&flo, fFalse, 0, flo.cb);

    if (!flo.pfil->FSetFni(pfni))
    {
    LFail:
        ReleasePpo(&flo.pfil);
        return fFalse;
    }
    flo.pfil->SetTemp(fFalse);

    if (fSetFni)
    {
        ReleasePpo(&_pfil);
        _pfil = flo.pfil;
        _fDirty = fFalse;
    }
    else
        ReleasePpo(&flo.pfil);

    return fTrue;
}

/***************************************************************************
    Constructor for a rich text document.
***************************************************************************/
TXRD::TXRD(PDOCB pdocb, uint32_t grfdoc) : TXRD_PAR(pdocb, grfdoc)
{
}

/***************************************************************************
    Destructor for a rich text document.
***************************************************************************/
TXRD::~TXRD(void)
{
    ReleasePpo(&_pglmpe);
    ReleasePpo(&_pagcact);
    ReleasePpo(&_pcfl);
}

#ifdef DEBUG
/***************************************************************************
    Assert the validity of a TXRD.
***************************************************************************/
void TXRD::AssertValid(uint32_t grfobj)
{
    TXRD_PAR::AssertValid(grfobj);
    AssertPo(_pglmpe, 0);
    AssertNilOrPo(_pagcact, 0);
    AssertIn(_dypFontDef, 1, kswMax);
    AssertIn(_pbsf->IbMac() / SIZEOF(achar), 1, kcpMaxTxrd);

    if (grfobj & fobjAssertFull)
    {
        MPE mpePrev, mpe;
        int32_t impe;

        mpePrev.spcp = (uint32_t)-1;
        for (impe = _pglmpe->IvMac(); impe-- > 0;)
        {
            _pglmpe->Get(impe, &mpe);
            Assert(mpe.spcp < mpePrev.spcp, "non-increasing mpe's");
            mpePrev = mpe;
        }
        // REVIEW shonk: TXRD::AssertValid: fill out
    }
}

/***************************************************************************
    Mark memory for the TXRD.
***************************************************************************/
void TXRD::MarkMem(void)
{
    AssertThis(fobjAssertFull);
    TXRD_PAR::MarkMem();
    MarkMemObj(_pglmpe);
    MarkMemObj(_pagcact);
}
#endif // DEBUG

/***************************************************************************
    Static method to create or open a rich text document.
***************************************************************************/
PTXRD TXRD::PtxrdNew(PFNI pfni)
{
    AssertNilOrPo(pfni, ffniFile);
    PTXRD ptxrd;

    if (pvNil != (ptxrd = NewObj TXRD) && !ptxrd->_FInit(pfni))
        ReleasePpo(&ptxrd);
    return ptxrd;
}

/***************************************************************************
    Initialize the rich text document.
***************************************************************************/
bool TXRD::_FInit(PFNI pfni, CTG ctg)
{
    AssertBaseThis(0);
    AssertNilOrPo(pfni, 0);

    if (pvNil != pfni)
    {
        PCFL pcfl;
        CKI cki;

        if (pvNil == (pcfl = CFL::PcflOpen(pfni, fcflNil)))
        {
            PushErc(ercRtxdReadFailed);
            return fFalse;
        }
        if (!pcfl->FGetCkiCtg(ctg, 0, &cki) || !_FReadChunk(pcfl, cki.ctg, cki.cno, fFalse))
        {
            ReleasePpo(&pcfl);
            PushErc(ercRtxdReadFailed);
            return fFalse;
        }
        _pcfl = pcfl;
        AssertThis(fobjAssertFull);
        return fTrue;
    }

    if (!TXRD_PAR::_FInit())
        return fFalse;
    if (pvNil == (_pglmpe = GL::PglNew(SIZEOF(MPE))))
        return fFalse;
    _pglmpe->SetMinGrow(10);
    _onnDef = vpappb->OnnDefVariable();
    _oskFont = koskCur;
    vntl.GetStn(_onnDef, &_stnFontDef);

    // NOTE: don't use vpappb->DypTextDef() so all TXRD's will have the
    // same _dypFontDef unless explicitly changed in code.
    _dypFontDef = 12;

    AssertThis(fobjAssertFull);
    return fTrue;
}

/***************************************************************************
    Static method to read a rich text document from a chunk.
***************************************************************************/
PTXRD TXRD::PtxrdReadChunk(PCFL pcfl, CTG ctg, CNO cno, bool fCopyText)
{
    AssertPo(pcfl, 0);
    PTXRD ptxrd;

    if (pvNil != (ptxrd = NewObj TXRD) && !ptxrd->_FReadChunk(pcfl, ctg, cno, fCopyText))
    {
        PushErc(ercRtxdReadFailed);
        ReleasePpo(&ptxrd);
    }
    return ptxrd;
}

/***************************************************************************
    Read the given chunk into this TXRD.
***************************************************************************/
bool TXRD::_FReadChunk(PCFL pcfl, CTG ctg, CNO cno, bool fCopyText)
{
    AssertPo(pcfl, 0);
    BLCK blck;
    FLO floText;
    PFIL pfilT;
    KID kid;
    int32_t icact;
    int32_t cact;
    RDOP rdop;
    int16_t bo, osk;

    if (!TXRD_PAR::_FInit())
        return fFalse;

    if (!pcfl->FFind(ctg, cno, &blck) || !blck.FUnpackData() || blck.Cb() < SIZEOF(RDOP) ||
        !blck.FReadRgb(&rdop, SIZEOF(RDOP), 0))
    {
        return fFalse;
    }

    if (rdop.bo == kboOther)
        SwapBytesBom(&rdop, kbomRdop);
    else if (rdop.bo != kboCur)
        return fFalse;

    // do a sanity check on the default font size and width
    if (!FIn(rdop.dypFont, 4, 256) || !FIn(rdop.dxpDef, 1, kcbMax))
    {
        Bug("bad default font size");
        return fFalse;
    }
    _dxpDef = rdop.dxpDef;
    _dypFontDef = rdop.dypFont;
    _acrBack.SetFromLw(rdop.lwAcrBack);

    if (_stnFontDef.FRead(&blck, SIZEOF(rdop)) && _stnFontDef.Cch() > 0)
        _oskFont = rdop.oskFont;
    else
    {
        _oskFont = koskCur;
        _stnFontDef.SetNil();
    }
    _onnDef = vntl.OnnMapStn(&_stnFontDef, _oskFont);

    // get the text
    if (!pcfl->FGetKidChidCtg(ctg, cno, 0, kctgText, &kid) || !pcfl->FFindFlo(kid.cki.ctg, kid.cki.cno, &floText) ||
        floText.cb < SIZEOF(int16_t) || !floText.FReadRgb(&osk, SIZEOF(int16_t), 0))
    {
        return fFalse;
    }
    floText.fp += SIZEOF(int16_t);
    floText.cb -= SIZEOF(int16_t);
    floText.pfil->AddRef();
    pfilT = floText.pfil;
    if (!floText.FTranslate(osk) || !_pbsf->FReplaceFlo(&floText, fCopyText && pfilT == floText.pfil, 0, CpMac() - 1))
    {
        ReleasePpo(&floText.pfil);
        return fFalse;
    }
    ReleasePpo(&floText.pfil);

    // get the text properties
    if (!pcfl->FGetKidChidCtg(ctg, cno, 0, kctgTxtProps, &kid) || !pcfl->FFind(kid.cki.ctg, kid.cki.cno, &blck) ||
        pvNil == (_pglmpe = GL::PglRead(&blck, &bo)) || SIZEOF(MPE) != _pglmpe->CbEntry())
    {
        return fFalse;
    }
    if (bo == kboOther)
    {
        SwapBytesRglw(_pglmpe->QvGet(0), LwMul(_pglmpe->IvMac(), SIZEOF(MPE)) / SIZEOF(int32_t));
    }

    // get the text property arguments
    if (pcfl->FGetKidChidCtg(ctg, cno, 0, kctgLTxtPropArgs, &kid))
    {
        if (!pcfl->FFind(kid.cki.ctg, kid.cki.cno, &blck) || pvNil == (_pagcact = AG::PagRead(&blck, &bo, &osk)) ||
            SIZEOF(int32_t) != _pagcact->CbFixed())
        {
            return fFalse;
        }
        for (icact = _pagcact->IvMac(); icact-- > 0;)
        {
            if (_pagcact->FFree(icact))
                continue;
            _pagcact->GetFixed(icact, &cact);
            if (bo == kboOther)
            {
                SwapBytesRglw(&cact, 1);
                _pagcact->PutFixed(icact, &cact);
            }
            if (!_FOpenArg(icact, B3Lw(cact), bo, osk))
                return fFalse;
        }
    }

    AssertThis(fobjAssertFull);
    return fTrue;
}

/***************************************************************************
    Do any necessary munging of the AG entry on open. Return false if
    we don't recognize this argument type.
***************************************************************************/
bool TXRD::_FOpenArg(int32_t icact, uint8_t sprm, int16_t bo, int16_t osk)
{
    AssertBaseThis(0);
    int32_t onn, cb;
    uint8_t *prgb;
    STN stn;

    cb = _pagcact->Cb(icact);
    switch (sprm)
    {
    case sprmFont:
        cb -= SIZEOF(int32_t) + SIZEOF(int16_t); // onn, osk
        if (cb < 0)
        {
            Bug("bad font entry");
            return fFalse;
        }
        _pagcact->GetRgb(icact, SIZEOF(int32_t), SIZEOF(int16_t), &osk);
        prgb = (uint8_t *)PvAddBv(_pagcact->PvLock(icact), SIZEOF(int32_t) + SIZEOF(int16_t));
        if (!stn.FSetData(prgb, cb))
        {
            Bug("bad font entry");
            _pagcact->Unlock();
            return fFalse;
        }
        _pagcact->Unlock();

        onn = vntl.OnnMapStn(&stn, osk);
        _pagcact->PutRgb(icact, 0, SIZEOF(int32_t), &onn);
        break;

    default:
        return fFalse;
    }

    return fTrue;
}

/***************************************************************************
    Get the current FNI for the doc. Return false if the doc is not
    currently based on an FNI (it's a new doc or an internal one).
***************************************************************************/
bool TXRD::FGetFni(FNI *pfni)
{
    AssertThis(0);
    AssertBasePo(pfni, 0);

    if (pvNil == _pcfl)
        return fFalse;

    _pcfl->GetFni(pfni);
    return fTrue;
}

/***************************************************************************
    Ask the user what file they want to save to.
***************************************************************************/
bool TXRD::FGetFniSave(FNI *pfni)
{
    // REVIEW shonk: what file type to use on Mac?
    AssertThis(0);
    return FGetFniSaveMacro(pfni, 'CHN2',
                            "\x9"
                            "Save As: ",
                            "", PszLit("All files\0*.*\0"), vwig.hwndApp);
}

/***************************************************************************
    Save the rich text document and optionally set this fni as the current
    one. If the doc is currently based on an FNI, pfni may be nil, indicating
    that this is a normal save (not save as). If pfni is not nil and
    fSetFni is false, this just writes a copy of the doc but doesn't change
    the doc one bit.
***************************************************************************/
bool TXRD::FSaveToFni(FNI *pfni, bool fSetFni)
{
    AssertThis(fobjAssertFull);
    AssertNilOrPo(pfni, ffniFile);
    FNI fni;
    PCFL pcfl;
    CKI cki;

    if (pvNil == pfni)
    {
        if (pvNil == _pcfl)
        {
            Bug("Can't do a normal save - no file");
            return fFalse;
        }
        fSetFni = fTrue;
        _pcfl->GetFni(&fni);
        pfni = &fni;
    }

    if (pvNil == (pcfl = CFL::PcflCreateTemp(pfni)))
        goto LFail;

    if (!FSaveToChunk(pcfl, &cki, fSetFni) || !pcfl->FSave(kctgLFramework, pvNil))
        goto LFail;

    if (!pcfl->FSetFni(pfni))
    {
    LFail:
        ReleasePpo(&pcfl);
        PushErc(ercRtxdSaveFailed);
        return fFalse;
    }
    pcfl->SetTemp(fFalse);

    if (fSetFni)
    {
        ReleasePpo(&_pcfl);
        _pcfl = pcfl;
        _fDirty = fFalse;
    }
    else
        ReleasePpo(&pcfl);

    return fTrue;
}

/***************************************************************************
    Save a rich text document to the given chunky file. Fill in *pcki with
    where we put the root chunk.
***************************************************************************/
bool TXRD::FSaveToChunk(PCFL pcfl, CKI *pcki, bool fRedirectText)
{
    AssertThis(fobjAssertFull);
    AssertPo(pcfl, 0);
    AssertVarMem(pcki);
    RDOP rdop;
    BLCK blck, blckText;
    CNO cno, cnoText;
    int32_t cb;
    int16_t osk = koskCur;

    pcki->ctg = kctgLRichText;
    rdop.bo = kboCur;
    rdop.oskFont = _oskFont;
    rdop.dxpDef = _dxpDef;
    rdop.dypFont = _dypFontDef;
    rdop.lwAcrBack = _acrBack.LwGet();
    cb = _stnFontDef.CbData();
    if (!pcfl->FAdd(SIZEOF(RDOP) + cb, pcki->ctg, &pcki->cno, &blck))
    {
        PushErc(ercRtxdSaveFailed);
        return fFalse;
    }
    if (!blck.FWriteRgb(&rdop, SIZEOF(RDOP), 0) || !_stnFontDef.FWrite(&blck, SIZEOF(RDOP)))
    {
        goto LFail;
    }

    // add the text chunk and write it
    if (!pcfl->FAddChild(pcki->ctg, pcki->cno, 0, _pbsf->IbMac() - SIZEOF(achar) + SIZEOF(int16_t), kctgText, &cnoText,
                         &blckText) ||
        !blckText.FWriteRgb(&osk, SIZEOF(int16_t), 0))
    {
        goto LFail;
    }
    AssertDo(blckText.FMoveMin(SIZEOF(int16_t)), 0);
    if (!_pbsf->FWriteRgb(&blckText))
        goto LFail;

    cb = _pglmpe->CbOnFile();
    if (!pcfl->FAddChild(pcki->ctg, pcki->cno, 0, cb, kctgLTxtProps, &cno, &blck) || !_pglmpe->FWrite(&blck))
    {
        goto LFail;
    }

    if (pvNil != _pagcact && 0 < _pagcact->IvMac())
    {
        cb = _pagcact->CbOnFile();
        if (!pcfl->FAddChild(pcki->ctg, pcki->cno, 0, cb, kctgLTxtPropArgs, &cno, &blck) || !_pagcact->FWrite(&blck))
        {
            goto LFail;
        }
    }

    if (fRedirectText)
    {
        FLO flo;
        AssertDo(blckText.FGetFlo(&flo), 0);
        _pbsf->FReplaceFlo(&flo, fFalse, 0, _pbsf->IbMac() - SIZEOF(achar));
        ReleasePpo(&flo.pfil);
    }
    return fTrue;

LFail:
    pcfl->Delete(pcki->ctg, pcki->cno);
    PushErc(ercRtxdSaveFailed);
    return fFalse;
}

/***************************************************************************
    Create a new TXRG to display the TXRD.
***************************************************************************/
PDDG TXRD::PddgNew(PGCB pgcb)
{
    AssertThis(fobjAssertFull);
    return TXRG::PtxrgNew(this, pgcb);
}

/***************************************************************************
    Look for an MPE for the given spcp. Return false iff there isn't one.
    In either event, fill pimpe with where one would go if it did exist.
***************************************************************************/
bool TXRD::_FFindMpe(uint32_t spcp, MPE *pmpe, int32_t *pcpLim, int32_t *pimpe)
{
    AssertThis(0);
    AssertNilOrVarMem(pmpe);
    AssertNilOrVarMem(pcpLim);
    AssertNilOrVarMem(pimpe);
    int32_t impe, impeMin, impeLim;
    bool fRet;
    MPE mpe;
    uint8_t sprm = _SprmFromSpcp(spcp);

    for (impeMin = 0, impeLim = _pglmpe->IvMac(); impeMin < impeLim;)
    {
        impe = (impeMin + impeLim) / 2;
        _pglmpe->Get(impe, &mpe);
        if (mpe.spcp < spcp)
            impeMin = impe + 1;
        else if (mpe.spcp > spcp)
            impeLim = impe;
        else
        {
            impeMin = impe + 1;
            goto LSuccess;
        }
    }

    // assume there isn't one
    if (pvNil != pimpe)
        *pimpe = impeMin;
    TrashVar(pmpe);
    fRet = fFalse;

    if (impeMin > 0)
    {
        _pglmpe->Get(impeMin - 1, &mpe);
        if (_SprmFromSpcp(mpe.spcp) == sprm)
        {
        LSuccess:
            if (pvNil != pimpe)
                *pimpe = impeMin - 1;
            if (pvNil != pmpe)
                *pmpe = mpe;
            fRet = fTrue;
        }
        else
            Assert(_SprmFromSpcp(mpe.spcp) < sprm, 0);
    }

    if (pvNil != pcpLim)
    {
        *pcpLim = CpMac();
        if (impeMin < _pglmpe->IvMac())
        {
            _pglmpe->Get(impeMin, &mpe);
            if (_SprmFromSpcp(mpe.spcp) == sprm)
                *pcpLim = _CpFromSpcp(mpe.spcp);
        }
    }
    return fRet;
}

/***************************************************************************
    Fetch the impe'th property, returning all the relevant info about it.
***************************************************************************/
bool TXRD::_FFetchProp(int32_t impe, uint8_t *psprm, int32_t *plw, int32_t *pcpMin, int32_t *pcpLim)
{
    MPE mpe;

    if (!FIn(impe, 0, _pglmpe->IvMac()))
        return fFalse;

    _pglmpe->Get(impe, &mpe);
    if (pvNil != psprm)
        *psprm = _SprmFromSpcp(mpe.spcp);
    if (pvNil != plw)
        *plw = mpe.lw;
    if (pvNil != pcpMin)
        *pcpMin = _CpFromSpcp(mpe.spcp);
    if (pvNil != pcpLim)
    {
        *pcpLim = CpMac();
        if (impe + 1 < _pglmpe->IvMac())
        {
            MPE mpeNext;

            _pglmpe->Get(impe + 1, &mpeNext);
            if (_SprmFromSpcp(mpeNext.spcp) == _SprmFromSpcp(mpe.spcp))
                *pcpLim = _CpFromSpcp(mpeNext.spcp);
        }
    }
    return fTrue;
}

/***************************************************************************
    Adjust the MPE's after an edit. This may involve deleting some MPE's
    and/or updating the cp's.
***************************************************************************/
void TXRD::_AdjustMpe(int32_t cp, int32_t ccpIns, int32_t ccpDel)
{
    AssertBaseThis(0);
    AssertIn(cp, 0, CpMac());
    AssertIn(ccpIns, 0, CpMac() - cp);
    AssertIn(ccpDel, 0, kcbMax);
    MPE *qmpe;
    int32_t impe, impeMin, cpT;
    uint8_t sprm;
    bool fBefore, fKeep;

    _cpMinChp = _cpLimChp = _cpMinPap = _cpLimPap = 0;
    for (impe = 0; impe < _pglmpe->IvMac();)
    {
        qmpe = (MPE *)_pglmpe->QvGet(impe);
        cpT = _CpFromSpcp(qmpe->spcp);
        AssertIn(cpT, 0, kcpMaxTxrd);
        if (cpT < cp)
        {
            impe++;
            continue;
        }
        sprm = _SprmFromSpcp(qmpe->spcp);
        if (cpT >= cp + ccpDel + (sprm < sprmMinObj))
        {
            AssertIn(cpT + ccpIns - ccpDel, 0, kcpMaxTxrd);
            qmpe->spcp = _SpcpFromSprmCp(sprm, cpT + ccpIns - ccpDel);
            impe++;
            continue;
        }
        AssertIn(cpT, cp, cp + ccpDel + (sprm < sprmMinObj));

        // special case object sprms
        if (sprm >= sprmMinObj)
        {
            // delete it
            AssertIn(cpT, cp, cp + ccpDel);
            _ReleaseSprmLw(sprm, qmpe->lw);
            _pglmpe->Delete(impe);
            continue;
        }

        // fBefore indicates if the last MPE for this sprm should go at cp
        // or at the beginning of the paragraph at or after cp + ccpIns.
        // This is only used for paragraph sprms.
        fBefore = (cpT == cp);

        impeMin = impe;
        while (cpT <= cp + ccpDel)
        {
            if (++impe >= _pglmpe->IvMac())
                break;
            qmpe = (MPE *)_pglmpe->QvGet(impe);
            if (_SprmFromSpcp(qmpe->spcp) != sprm)
                break;
            cpT = _CpFromSpcp(qmpe->spcp);
            AssertIn(cpT, 0, kcpMaxTxrd);
        }

        // the entries from impeMin to impe are all in or
        // at the end of the deleted zone
        Assert(impe > impeMin, "no MPE's!");

        // fKeep indicates whether we kept the last MPE
        fKeep = fTrue;
        if (sprm < sprmMinPap)
        {
            // a character sprm - put the last one at cp + ccpIns
            AssertIn(cp + ccpIns, 0, CpMac());
            qmpe = (MPE *)_pglmpe->QvGet(--impe);
            qmpe->spcp = _SpcpFromSprmCp(sprm, cp + ccpIns);
        }
        else if (fBefore)
        {
            // a paragraph sprm - the last one goes at cp
            AssertIn(cp, 0, kcpMaxTxrd);
            qmpe = (MPE *)_pglmpe->QvGet(--impe);
            qmpe->spcp = _SpcpFromSprmCp(sprm, cp);
        }
        else
        {
            // a paragraph sprm - the last MPE goes at the beginning of the
            // next paragraph (if there is one and it doesn't already have an MPE).
            cpT = CpLimPara(cp + ccpIns - 1);
            if (cpT < CpMac() &&
                (impe >= _pglmpe->IvMac() || _SprmFromSpcp((qmpe = (MPE *)_pglmpe->QvGet(impe))->spcp) != sprm ||
                 _CpFromSpcp(qmpe->spcp) + ccpIns - ccpDel > cpT))
            {
                // put the last one at cpT - if there isn't already one
                AssertIn(cpT, 0, kcpMaxTxrd);
                qmpe = (MPE *)_pglmpe->QvGet(--impe);
                qmpe->spcp = _SpcpFromSprmCp(sprm, cpT);
            }
            else
                fKeep = fFalse;
        }

        // delete all the previous ones
        while (impeMin < impe)
        {
            qmpe = (MPE *)_pglmpe->QvGet(--impe);
            _ReleaseSprmLw(sprm, qmpe->lw);
            _pglmpe->Delete(impe);
        }

        // skip the one we kept at the end
        if (fKeep)
            impe++;
    }
    AssertThis(0);
}

/***************************************************************************
    Make sure there is an entry in the AG with the given data. If it's
    already there, increment its reference count. Otherwise, set its
    reference count to 1.
***************************************************************************/
bool TXRD::_FEnsureInAg(uint8_t sprm, void *pv, int32_t cb, int32_t *pjv)
{
    AssertIn(cb, 1, kcbMax);
    AssertPvCb(pv, cb);
    AssertVarMem(pjv);
    int32_t cact, iv, cbT;
    void *qv;

    if (pvNil == _pagcact && pvNil == (_pagcact = AG::PagNew(SIZEOF(int32_t), 1, cb)))
    {
        TrashVar(pjv);
        return fFalse;
    }

    // see if it's already there
    for (iv = _pagcact->IvMac(); iv-- > 0;)
    {
        if (_pagcact->FFree(iv))
            continue;
        qv = _pagcact->QvGet(iv, &cbT);
        if (cb != cbT)
            continue;
        if (FEqualRgb(qv, pv, cb))
        {
            _pagcact->GetFixed(iv, &cact);
            if (B3Lw(cact) == sprm)
            {
                *pjv = iv + 1;
                AssertIn(cact & 0x00FFFFFFL, 1, 0x00FFFFFFL);
                cact++;
                Assert(B3Lw(cact) == sprm, "cact overflowed");
                _pagcact->PutFixed(iv, &cact);
                return fTrue;
            }
        }
    }

    // need to add it
    cact = (((uint32_t)sprm) << 24) | 1;
    if (!_pagcact->FAdd(cb, pjv, pv, &cact))
    {
        TrashVar(pjv);
        return fFalse;
    }
    (*pjv)++;
    return fTrue;
}

/***************************************************************************
    Decrement the reference count on the given element of the AG and if
    the reference count becomes zero, delete the element.
***************************************************************************/
void TXRD::_ReleaseInAg(int32_t jv)
{
    int32_t cact, cactT;

    if (pvNil == _pagcact || !FIn(jv, 1, _pagcact->IvMac() + 1) || _pagcact->FFree(jv - 1))
    {
        Bug("bad index into AG");
        return;
    }

    _pagcact->GetFixed(jv - 1, &cact);
    cactT = cact & 0x00FFFFFFL;
    if (--cactT <= 0)
        _pagcact->Delete(jv - 1);
    else
    {
        cact--;
        _pagcact->PutFixed(jv - 1, &cact);
    }
}

/***************************************************************************
    Increment the reference count on the given element of the AG.
***************************************************************************/
void TXRD::_AddRefInAg(int32_t jv)
{
    int32_t cact;

    if (pvNil == _pagcact || !FIn(jv, 1, _pagcact->IvMac() + 1) || _pagcact->FFree(jv - 1))
    {
        Bug("bad index into AG");
        return;
    }

    _pagcact->GetFixed(jv - 1, &cact);
    AssertIn(cact & 0x00FFFFFFL, 1, 0x00FFFFFFL);
    ++cact;
    _pagcact->PutFixed(jv - 1, &cact);
}

/***************************************************************************
    Static method: return true iff the given sprm has its argument in
    the _pagcact.
***************************************************************************/
bool TXRD::_FSprmInAg(uint8_t sprm)
{
    if (sprm < sprmMinChpClient || FIn(sprm, sprmMinPap, sprmMinPapClient))
    {
        // a base TXRD sprm
        return sprm == sprmFont;
    }

    // The sprm is a client or object sprm. Even ones are in the AG, odd
    // ones aren't. See note in rtxt.h where the sprms are defined.
    return !(sprm & 1);
}

/***************************************************************************
    If the sprm allocates stuff in the ag, release it. The inverse operation
    of _AddRefSprmLw.
***************************************************************************/
void TXRD::_ReleaseSprmLw(uint8_t sprm, int32_t lw)
{
    if (lw > 0 && _FSprmInAg(sprm))
        _ReleaseInAg(lw);
}

/***************************************************************************
    If the sprm allocates stuff in the ag, addref it. The inverse operation
    of _ReleaseSprmLw.
***************************************************************************/
void TXRD::_AddRefSprmLw(uint8_t sprm, int32_t lw)
{
    if (lw > 0 && _FSprmInAg(sprm))
        _AddRefInAg(lw);
}

/***************************************************************************
    Get an array of SPVM's that describe the differences between pchp and
    pchpDiff. If pchpDiff is nil, pchp is just described. If this
    succeeds, either _ApplyRgspvm or _ReleaseRgspvm should be called.
***************************************************************************/
bool TXRD::_FGetRgspvmFromChp(PCHP pchp, PCHP pchpDiff, SPVM *prgspvm, int32_t *pcspvm)
{
    AssertVarMem(pchp);
    AssertNilOrVarMem(pchpDiff);
    AssertPvCb(prgspvm, LwMul(SIZEOF(SPVM), sprmLimChp - sprmMinChp));
    AssertVarMem(pcspvm);
    uint8_t sprm;
    int32_t ispvm;
    SPVM spvm;

    // first get the values and masks for the sprm's we have to deal with
    ispvm = 0;
    for (sprm = sprmMinChp; sprm < sprmLimChp; sprm++)
    {
        switch (_TGetLwFromChp(sprm, pchp, pchpDiff, &spvm.lw, &spvm.lwMask))
        {
        case tYes:
            spvm.sprm = sprm;
            prgspvm[ispvm++] = spvm;
            break;
        case tMaybe:
            _ReleaseRgspvm(prgspvm, ispvm);
            TrashVar(pcspvm);
            return fFalse;
        }
    }

    *pcspvm = ispvm;
    return fTrue;
}

/***************************************************************************
    Get an array of SPVM's that describe the differences between ppap and
    ppapDiff. If ppapDiff is nil, ppap is just described. If this
    succeeds, either _ApplyRgspvm or _ReleaseRgspvm should be called.
***************************************************************************/
bool TXRD::_FGetRgspvmFromPap(PPAP ppap, PPAP ppapDiff, SPVM *prgspvm, int32_t *pcspvm)
{
    AssertVarMem(ppap);
    AssertNilOrVarMem(ppapDiff);
    AssertPvCb(prgspvm, LwMul(SIZEOF(SPVM), sprmLimPap - sprmMinPap));
    AssertVarMem(pcspvm);
    uint8_t sprm;
    int32_t ispvm;
    SPVM spvm;

    // first get the values and masks for the sprm's we have to deal with
    ispvm = 0;
    for (sprm = sprmMinPap; sprm < sprmLimPap; sprm++)
    {
        switch (_TGetLwFromPap(sprm, ppap, ppapDiff, &spvm.lw, &spvm.lwMask))
        {
        case tYes:
            spvm.sprm = sprm;
            prgspvm[ispvm++] = spvm;
            break;
        case tMaybe:
            _ReleaseRgspvm(prgspvm, ispvm);
            TrashVar(pcspvm);
            return fFalse;
        }
    }

    *pcspvm = ispvm;
    return fTrue;
}

/***************************************************************************
    Release the SPVM's.
***************************************************************************/
void TXRD::_ReleaseRgspvm(SPVM *prgspvm, int32_t cspvm)
{
    AssertIn(cspvm, 0, kcbMax);
    AssertPvCb(prgspvm, LwMul(SIZEOF(SPVM), cspvm));
    int32_t ispvm;

    for (ispvm = 0; ispvm < cspvm; ispvm++)
        _ReleaseSprmLw(prgspvm[ispvm].sprm, prgspvm[ispvm].lw);
}

/***************************************************************************
    Apply the SPVM's to the given range of characters. Assumes that the
    inserts can't fail. If you call this, don't also call _ReleaseRgspvm.
    The caller should have ensured that _pglmpe has room for 2 * cspvm
    additional entries.
***************************************************************************/
void TXRD::_ApplyRgspvm(int32_t cp, int32_t ccp, SPVM *prgspvm, int32_t cspvm)
{
    AssertIn(cp, 0, CpMac());
    AssertIn(ccp, 1, CpMac() + 1 - cp);
    MPE mpe, mpeT;
    int32_t lwRevert, lwLast;
    int32_t cpLimT, cpLim;
    int32_t ispvm, impe;
    SPVM spvm;

    _cpMinChp = _cpLimChp = _cpMinPap = _cpLimPap = 0;
    cpLim = LwMin(cp + ccp, CpMac());
    for (ispvm = cspvm; ispvm-- > 0;)
    {
        spvm = prgspvm[ispvm];
        mpe.spcp = _SpcpFromSprmCp(spvm.sprm, cp);
        mpe.lw = spvm.lw;

        if (!_FFindMpe(mpe.spcp, &mpeT, pvNil, &impe))
        {
            // there are no MPE's for this sprm at or before this cp
            lwRevert = 0;
            if (mpe.lw != 0)
            {
                // add the mpe
                _AddRefSprmLw(spvm.sprm, mpe.lw);
                AssertDo(_pglmpe->FInsert(impe++, &mpe), 0);
            }
            lwLast = mpe.lw;
        }
        else
        {
            // see if the MPE we're in the range of starts before this cp
            // or at it
            lwRevert = mpeT.lw;
            if (_CpFromSpcp(mpeT.spcp) < cp)
            {
                // before
                impe++;
                if (mpe.lw != mpeT.lw)
                {
                    // add an mpe
                    mpeT.lw = mpe.lw | (mpeT.lw & ~spvm.lwMask);
                    mpeT.spcp = mpe.spcp;
                    _AddRefSprmLw(spvm.sprm, mpeT.lw);
                    AssertDo(_pglmpe->FInsert(impe++, &mpeT), 0);
                }
                lwLast = mpeT.lw;
            }
            else
            {
                // at it - get the valid lwLast value
                Assert(_CpFromSpcp(mpeT.spcp) == cp, 0);
                lwLast = 0;
                if (impe > 0)
                {
                    _pglmpe->Get(impe - 1, &mpeT);
                    if (_SprmFromSpcp(mpeT.spcp) == spvm.sprm)
                        lwLast = mpeT.lw;
                }
            }
        }
        _AddRefSprmLw(spvm.sprm, lwRevert);

        // adjust all mpe's before cpLim
        for (;;)
        {
            if (impe >= _pglmpe->IvMac())
                goto LEndOfSprm;
            _pglmpe->Get(impe, &mpeT);
            if (_SprmFromSpcp(mpeT.spcp) != spvm.sprm)
            {
            LEndOfSprm:
                cpLimT = CpMac();
                break;
            }
            if ((cpLimT = _CpFromSpcp(mpeT.spcp)) >= cpLim)
            {
                // see if this MPE can be deleted
                if (cpLimT == cpLim && lwLast == mpeT.lw)
                {
                    _ReleaseSprmLw(spvm.sprm, mpeT.lw);
                    _pglmpe->Delete(impe);
                }
                break;
            }

            // modify the mpeT
            _ReleaseSprmLw(spvm.sprm, lwRevert);
            lwRevert = mpeT.lw;
            mpeT.lw = mpe.lw | (mpeT.lw & ~spvm.lwMask);
            // note: the ref count for this MPE was transferred to lwRevert,
            // so a _ReleaseSprmLw call is not needed after the Delete and Put
            // below
            if (lwLast == mpeT.lw)
                _pglmpe->Delete(impe);
            else
            {
                _AddRefSprmLw(spvm.sprm, mpeT.lw);
                _pglmpe->Put(impe++, &mpeT);
                lwLast = mpeT.lw;
            }
        }

        Assert(cpLimT >= cpLim, 0);
        if (cpLimT > cpLim && lwRevert != lwLast)
        {
            // add another mpe at cpLim with lwRevert
            mpeT.spcp = _SpcpFromSprmCp(spvm.sprm, cpLim);
            mpeT.lw = lwRevert;
            AssertDo(_pglmpe->FInsert(impe, &mpeT), 0);
        }
        else
            _ReleaseSprmLw(spvm.sprm, lwRevert);
        _ReleaseSprmLw(spvm.sprm, spvm.lw);
    }
}

/***************************************************************************
    Get the long associated with the sprm and the difference between the
    two CHPs. pchpOld may be nil. *plwMask indicates which portion of
    the lw is significant. Returns tYes or tNo depending on whether the
    values of pchpOld and pchpNew differ for the given sprm. Returns
    tMaybe on error.
***************************************************************************/
tribool TXRD::_TGetLwFromChp(uint8_t sprm, PCHP pchpNew, PCHP pchpOld, int32_t *plw, int32_t *plwMask)
{
    AssertIn(sprm, sprmMinChp, sprmLimChp);
    AssertVarMem(pchpNew);
    AssertNilOrVarMem(pchpOld);
    AssertVarMem(plw);
    AssertVarMem(plwMask);

    *plwMask = -1;
    switch (sprm)
    {
    default:
        *plwMask = 0;
        break;

    case sprmStyle:
        *plw = LwHighLow((int16_t)(pchpNew->dypFont - _dypFontDef),
                         SwHighLow((uint8_t)pchpNew->dypOffset, (uint8_t)pchpNew->grfont));
        if (pvNil != pchpOld)
        {
            *plwMask = LwHighLow(
                -(pchpOld->dypFont != pchpNew->dypFont),
                SwHighLow(-(pchpNew->dypOffset != pchpOld->dypOffset), (uint8_t)(pchpOld->grfont ^ pchpNew->grfont)));
        }
        break;

    case sprmFont:
        if (pvNil != pchpOld && pchpOld->onn == pchpNew->onn)
            *plwMask = 0;
        else if (pchpNew->onn == _onnDef)
            *plw = 0;
        else
        {
            // data for sprmFont is: int32_t onn, int16_t osk, stn data
            STN stn;
            int32_t cb;
            int16_t osk = koskCur;
            uint8_t rgb[kcbMaxDataStn + SIZEOF(int32_t) + SIZEOF(int16_t)];

            CopyPb(&pchpNew->onn, rgb, SIZEOF(int32_t));
            CopyPb(&osk, rgb + SIZEOF(int32_t), SIZEOF(int16_t));
            vntl.GetStn(pchpNew->onn, &stn);
            cb = SIZEOF(int32_t) + SIZEOF(int16_t);
            stn.GetData(rgb + cb);
            cb += stn.CbData();
            if (!_FEnsureInAg(sprm, rgb, cb, plw))
            {
                *plwMask = 0;
                TrashVar(plw);
                return tMaybe;
            }
        }
        break;

    case sprmForeColor:
        if (pvNil != pchpOld && pchpOld->acrFore == pchpNew->acrFore)
            *plwMask = 0;
        else
            *plw = pchpNew->acrFore.LwGet() - kacrBlack.LwGet();
        break;

    case sprmBackColor:
        if (pvNil != pchpOld && pchpOld->acrBack == pchpNew->acrBack)
            *plwMask = 0;
        else
            *plw = pchpNew->acrBack.LwGet() - kacrClear.LwGet();
        break;
    }

    if (0 != *plwMask)
    {
        *plw &= *plwMask;
        return tYes;
    }
    TrashVar(plw);
    return tNo;
}

/***************************************************************************
    Get the character properties for the given character.
***************************************************************************/
void TXRD::FetchChp(int32_t cp, PCHP pchp, int32_t *pcpMin, int32_t *pcpLim)
{
    AssertThis(0);
    AssertIn(cp, 0, CpMac());
    AssertVarMem(pchp);
    AssertNilOrVarMem(pcpMin);
    AssertNilOrVarMem(pcpLim);
    MPE mpe;
    uint8_t sprm;
    uint32_t spcp;
    int32_t cpLimT;
    int32_t cb;
    bool fRet;

    if (FIn(cp, _cpMinChp, _cpLimChp))
        goto LDone;

    _chp.Clear();
    _chp.onn = _onnDef;
    _chp.dypFont = _dypFontDef;
    _chp.acrFore = kacrBlack;
    _chp.acrBack = kacrClear;

    _cpLimChp = CpMac();
    _cpMinChp = 0;
    for (sprm = sprmMinChp; sprm < sprmLimChp; sprm++)
    {
        spcp = _SpcpFromSprmCp(sprm, cp);
        fRet = _FFindMpe(spcp, &mpe, &cpLimT);
        _cpLimChp = LwMin(_cpLimChp, cpLimT);
        if (!fRet)
            continue;
        _cpMinChp = LwMax(_cpMinChp, _CpFromSpcp(mpe.spcp));

        switch (sprm)
        {
        case sprmStyle:
            _chp.grfont = B0Lw(mpe.lw);
            _chp.dypOffset = (int32_t)(achar)B1Lw(mpe.lw);
            _chp.dypFont += SwHigh(mpe.lw);
            break;

        case sprmFont:
            if (mpe.lw > 0)
            {
                uint8_t *qrgb;

                qrgb = (uint8_t *)_pagcact->QvGet(mpe.lw - 1, &cb);
                cb -= SIZEOF(int32_t) + SIZEOF(int16_t); // onn, osk
                if (!FIn(cb, 0, kcbMaxDataStn + 1))
                {
                    Warn("bad group element");
                    break;
                }
                CopyPb(qrgb, &_chp.onn, SIZEOF(int32_t));
                Assert(vntl.FValidOnn(_chp.onn), "invalid onn");
            }
            break;

        case sprmForeColor:
            if (mpe.lw != 0)
                _chp.acrFore.SetFromLw(kacrBlack.LwGet() + mpe.lw);
            break;

        case sprmBackColor:
            if (mpe.lw != 0)
                _chp.acrBack.SetFromLw(kacrClear.LwGet() + mpe.lw);
            break;
        }
    }

LDone:
    *pchp = _chp;
    if (pvNil != pcpMin)
        *pcpMin = _cpMinChp;
    if (pvNil != pcpLim)
        *pcpLim = _cpLimChp;
}

/***************************************************************************
    Apply the given character properties to the given range of characters.
***************************************************************************/
bool TXRD::FApplyChp(int32_t cp, int32_t ccp, PCHP pchp, PCHP pchpDiff, uint32_t grfdoc)
{
    AssertThis(fobjAssertFull);
    AssertIn(cp, 0, CpMac() - 1);
    AssertIn(ccp, 1, CpMac() - cp);
    int32_t cspvm;
    SPVM rgspvm[sprmLimChp - sprmMinChp];
    PRTUN prtun = pvNil;

    BumpCombineUndo();
    if (!FSetUndo(cp, cp + ccp, ccp))
        return fFalse;

    if (!_FGetRgspvmFromChp(pchp, pchpDiff, rgspvm, &cspvm))
    {
        CancelUndo();
        return fFalse;
    }

    // now make sure that _pglmpe has enough room - we need at most 2
    // entries per sprm.
    if (!_pglmpe->FEnsureSpace(2 * cspvm))
    {
        _ReleaseRgspvm(rgspvm, cspvm);
        CancelUndo();
        AssertThis(fobjAssertFull);
        return fFalse;
    }

    _ApplyRgspvm(cp, ccp, rgspvm, cspvm);
    CommitUndo();
    BumpCombineUndo();

    AssertThis(fobjAssertFull);
    InvalAllDdg(cp, ccp, ccp, grfdoc);
    return fTrue;
}

/***************************************************************************
    Find the bounds of the whole paragraphs overlapping the range
    [*pcpMin, *pcpMin + *pccp). If fExpand is false, the bounds are of
    whole paragraphs totally contained in the range, with the possible
    exception of the trailing end of paragraph marker. If fExpand is
    true, *pcpMin will only decrease and *pcpLim will only increase;
    while if fExpand is false, *pcpMin will only increase and
    *pcpLim may decrease or at most increase past an end of
    paragraph marker and its associated line feed characters.
***************************************************************************/
void TXRD::_GetParaBounds(int32_t *pcpMin, int32_t *pcpLim, bool fExpand)
{
    AssertVarMem(pcpMin);
    AssertVarMem(pcpLim);
    AssertIn(*pcpMin, 0, CpMac());
    AssertIn(*pcpLim, *pcpMin, CpMac() + 1);

    if (fExpand)
    {
        *pcpLim = CpLimPara(*pcpLim - (*pcpLim > 0 && *pcpLim > *pcpMin));
        *pcpMin = CpMinPara(*pcpMin);
    }
    else
    {
        if (!FMinPara(*pcpMin))
            *pcpMin = CpLimPara(*pcpMin);
        if (!FMinPara(*pcpLim))
        {
            *pcpLim = CpNext(*pcpLim);
            if (*pcpLim < CpMac())
                *pcpLim = CpMinPara(*pcpLim);
        }
    }
}

/***************************************************************************
    Get the long associated with the sprm and the difference between the
    two PAPs. ppapOld may be nil. *plwMask indicates which portion of
    the lw is significant. Returns tYes or tNo depending on whether the
    values of pchpOld and pchpNew differ for the given sprm. Returns
    tMaybe on error.
***************************************************************************/
tribool TXRD::_TGetLwFromPap(uint8_t sprm, PPAP ppapNew, PPAP ppapOld, int32_t *plw, int32_t *plwMask)
{
    AssertIn(sprm, sprmMinPap, sprmLimPap);
    AssertVarMem(ppapNew);
    AssertNilOrVarMem(ppapOld);
    AssertVarMem(plw);

    *plwMask = -1;
    switch (sprm)
    {
    case sprmHorz:
        if (pvNil != ppapOld)
        {
            if (ppapOld->jc == ppapNew->jc)
                *plwMask &= 0x00FFFFFFL;
            if (ppapOld->nd == ppapNew->nd)
                *plwMask &= 0xFF00FFFFL;
            if (ppapOld->dxpTab == ppapNew->dxpTab)
                *plwMask &= 0xFFFF0000L;
        }
        *plw = LwHighLow(SwHighLow(ppapNew->jc, ppapNew->nd), ppapNew->dxpTab - kdxpTabDef);
        break;
    case sprmVert:
        if (pvNil != ppapOld)
        {
            if (ppapOld->numLine == ppapNew->numLine)
                *plwMask &= 0x0000FFFFL;
            if (ppapOld->dypExtraLine == ppapNew->dypExtraLine)
                *plwMask &= 0xFFFF0000L;
        }
        *plw = LwHighLow(ppapNew->numLine - kdenLine, ppapNew->dypExtraLine);
        break;
    case sprmAfter:
        if (pvNil != ppapOld)
        {
            if (ppapOld->numAfter == ppapNew->numAfter)
                *plwMask &= 0x0000FFFFL;
            if (ppapOld->dypExtraAfter == ppapNew->dypExtraAfter)
                *plwMask &= 0xFFFF0000L;
        }
        *plw = LwHighLow(ppapNew->numAfter - kdenAfter, ppapNew->dypExtraAfter);
        break;
    }

    if (0 != *plwMask)
    {
        *plw &= *plwMask;
        return tYes;
    }
    TrashVar(plw);
    return tNo;
}

/***************************************************************************
    Get the paragraph properties for the paragraph containing the given
    character.
***************************************************************************/
void TXRD::FetchPap(int32_t cp, PPAP ppap, int32_t *pcpMin, int32_t *pcpLim)
{
    AssertThis(0);
    AssertIn(cp, 0, CpMac());
    AssertVarMem(ppap);
    AssertNilOrVarMem(pcpMin);
    AssertNilOrVarMem(pcpLim);
    MPE mpe;
    uint8_t sprm;
    uint32_t spcp;
    int32_t cpLimT;
    bool fRet;

    if (FIn(cp, _cpMinPap, _cpLimPap))
        goto LDone;

    ClearPb(&_pap, SIZEOF(PAP));
    _pap.dxpTab = kdxpTabDef;
    _pap.numLine = kdenLine;
    _pap.numAfter = kdenAfter;

    _cpLimPap = CpMac();
    _cpMinPap = 0;
    for (sprm = sprmMinPap; sprm < sprmLimPap; sprm++)
    {
        spcp = _SpcpFromSprmCp(sprm, cp);
        fRet = _FFindMpe(spcp, &mpe, &cpLimT);
        _cpLimPap = LwMin(_cpLimPap, cpLimT);
        if (!fRet)
            continue;
        _cpMinPap = LwMax(_cpMinPap, _CpFromSpcp(mpe.spcp));

        switch (sprm)
        {
        case sprmHorz:
            _pap.jc = B3Lw(mpe.lw);
            _pap.nd = B2Lw(mpe.lw);
            _pap.dxpTab += SwLow(mpe.lw);
            break;
        case sprmVert:
            _pap.numLine += SwHigh(mpe.lw);
            _pap.dypExtraLine = SwLow(mpe.lw);
            break;
        case sprmAfter:
            _pap.numAfter += SwHigh(mpe.lw);
            _pap.dypExtraAfter = SwLow(mpe.lw);
            break;
        }
    }

LDone:
    *ppap = _pap;
    if (pvNil != pcpMin)
        *pcpMin = _cpMinPap;
    if (pvNil != pcpLim)
        *pcpLim = _cpLimPap;
}

/***************************************************************************
    Apply the given paragraph properties to the given range of characters.
    If fExpand is true, the properties of the current paragraph containing
    cp are set regardless of whether cp is at the beginning of the
    paragraph.
***************************************************************************/
bool TXRD::FApplyPap(int32_t cp, int32_t ccp, PPAP ppap, PPAP ppapDiff, int32_t *pcpMin, int32_t *pcpLim, bool fExpand,
                     uint32_t grfdoc)
{
    AssertThis(fobjAssertFull);
    AssertIn(cp, 0, CpMac());
    AssertIn(ccp, 0, CpMac() + 1 - cp);
    AssertVarMem(ppap);
    AssertNilOrVarMem(ppapDiff);
    AssertNilOrVarMem(pcpMin);
    AssertNilOrVarMem(pcpLim);
    int32_t cpLim;
    int32_t cspvm;
    SPVM rgspvm[sprmLimPap - sprmMinPap];

    // see if there are any paragraphs to deal with
    cpLim = cp + ccp;
    _GetParaBounds(&cp, &cpLim, fExpand);
    if (pvNil != pcpMin)
        *pcpMin = cp;
    if (pvNil != pcpLim)
        *pcpLim = cpLim;

    if (cpLim <= cp)
    {
        // no paragraphs to affect
        return fTrue;
    }

    BumpCombineUndo();
    if (!FSetUndo(cp, cpLim, cpLim - cp))
        return fFalse;

    if (!_FGetRgspvmFromPap(ppap, ppapDiff, rgspvm, &cspvm))
    {
        CancelUndo();
        return fFalse;
    }

    // now make sure that _pglmpe has enough room - we need at most 2
    // entries per sprm.
    if (!_pglmpe->FEnsureSpace(2 * cspvm))
    {
        _ReleaseRgspvm(rgspvm, cspvm);
        CancelUndo();
        AssertThis(fobjAssertFull);
        return fFalse;
    }

    _ApplyRgspvm(cp, cpLim - cp, rgspvm, cspvm);
    CommitUndo();
    BumpCombineUndo();

    AssertThis(fobjAssertFull);
    InvalAllDdg(cp, cpLim - cp, cpLim - cp, grfdoc);
    return fTrue;
}

/***************************************************************************
    Set up undo for an action. If this succeeds, you must call either
    CancelUndo or CommitUndo.
***************************************************************************/
bool TXRD::FSetUndo(int32_t cp1, int32_t cp2, int32_t ccpIns)
{
    if (_cactSuspendUndo++ > 0 || _cundbMax == 0)
        return fTrue;

    Assert(_prtun == pvNil, "why is there an active undo record?");
    if (cp1 == cp2 && ccpIns == 0)
        return fTrue;

    if (pvNil == (_prtun = RTUN::PrtunNew(_cactCombineUndo, this, cp1, cp2, ccpIns)))
    {
        ResumeUndo();
        return fFalse;
    }
    return fTrue;
}

/***************************************************************************
    Cancel undo.
***************************************************************************/
void TXRD::CancelUndo(void)
{
    ResumeUndo();
    if (_cactSuspendUndo == 0)
        ReleasePpo(&_prtun);
}

/***************************************************************************
    Commit the setup undo.
***************************************************************************/
void TXRD::CommitUndo(void)
{
    PRTUN prtunPrev;

    ResumeUndo();
    if (_cactSuspendUndo != 0 || _prtun == pvNil)
        return;

    AssertPo(_prtun, 0);
    // see if this one can be combined with the last one
    if (_ipundbLimDone > 0)
    {
        _pglpundb->Get(_ipundbLimDone - 1, &prtunPrev);
        if (prtunPrev->FIs(kclsRTUN) && prtunPrev->FCombine(_prtun))
        {
            ClearRedo();
            goto LDone;
        }
    }
    FAddUndo(_prtun);

LDone:
    ReleasePpo(&_prtun);
}

/***************************************************************************
    Replace cp to cp + ccpDel with ccpIns characters from prgch. If ccpIns
    is zero, prgch can be nil.
***************************************************************************/
bool TXRD::FReplaceRgch(void *prgch, int32_t ccpIns, int32_t cp, int32_t ccpDel, uint32_t grfdoc)
{
    AssertThis(fobjAssertFull);
    AssertIn(ccpIns, 0, kcbMax);
    AssertPvCb(prgch, ccpIns * SIZEOF(achar));
    AssertIn(cp, 0, CpMac());
    AssertIn(ccpDel, 0, CpMac() - cp);

    if (CpMac() + ccpIns >= kcpMaxTxrd)
    {
        PushErc(ercRtxdTooMuchText);
        return fFalse;
    }

    if (!FSetUndo(cp, cp + ccpDel, ccpIns))
        return fFalse;

    if (!TXRD_PAR::FReplaceRgch(prgch, ccpIns, cp, ccpDel, fdocNil))
    {
        CancelUndo();
        return fFalse;
    }
    _AdjustMpe(cp, ccpIns, ccpDel);
    CommitUndo();
    AssertThis(fobjAssertFull);
    InvalAllDdg(cp, ccpIns, ccpDel, grfdoc);
    return fTrue;
}

/***************************************************************************
    Replace cp to cp + ccpDel with ccpIns characters from prgch, using
    the given chp and pap. If ccpIns is zero, prgch can be nil. pchp
    and/or ppap can be nil.
***************************************************************************/
bool TXRD::FReplaceRgch(void *prgch, int32_t ccpIns, int32_t cp, int32_t ccpDel, PCHP pchp, PPAP ppap, uint32_t grfdoc)
{
    AssertThis(0);
    AssertIn(ccpIns, 0, kcbMax);
    AssertPvCb(prgch, ccpIns * SIZEOF(achar));
    AssertIn(cp, 0, CpMac());
    AssertIn(ccpDel, 0, CpMac() - cp);
    AssertNilOrVarMem(pchp);
    AssertNilOrVarMem(ppap);

    return _FReplaceCore(prgch, pvNil, fFalse, pvNil, 0, ccpIns, cp, ccpDel, pchp, ppap, grfdoc);
}

/***************************************************************************
    Replace cp to cp + ccpDel with ccpIns characters from prgch, pflo or pbsf,
    using the given chp and pap. If ccpIns is zero, prgch can be nil. pchp
    and/or ppap can be nil.
***************************************************************************/
bool TXRD::_FReplaceCore(void *prgch, PFLO pflo, bool fCopy, PBSF pbsf, int32_t cpSrc, int32_t ccpIns, int32_t cp,
                         int32_t ccpDel, PCHP pchp, PPAP ppap, uint32_t grfdoc)
{
    AssertThis(fobjAssertFull);
    AssertIn(ccpIns, 0, kcbMax);
    AssertNilOrPvCb(prgch, ccpIns * SIZEOF(achar));
    AssertNilOrPo(pflo, 0);
    AssertNilOrPo(pbsf, 0);
    AssertIn(cp, 0, CpMac());
    AssertIn(ccpDel, 0, CpMac() - cp);
    AssertNilOrVarMem(pchp);
    AssertNilOrVarMem(ppap);
    SPVM rgspvm[sprmLimChp - sprmMinChp + sprmLimPap - sprmMinPap];
    int32_t cspvmChp, cspvmPap;
    int32_t cpMinUndo, cpLimUndo;

    if (CpMac() + ccpIns >= kcpMaxTxrd)
    {
        PushErc(ercRtxdTooMuchText);
        return fFalse;
    }

    cpMinUndo = cp;
    cpLimUndo = cp + ccpDel;

    cspvmChp = cspvmPap = 0;
    if (pvNil != pchp && ccpIns > 0 && !_FGetRgspvmFromChp(pchp, pvNil, rgspvm, &cspvmChp))
    {
        return fFalse;
    }

    if (pvNil != ppap && ccpIns > 0)
    {
        _GetParaBounds(&cpMinUndo, &cpLimUndo, fFalse);
        if (cpMinUndo < cpLimUndo && !_FGetRgspvmFromPap(ppap, pvNil, rgspvm + cspvmChp, &cspvmPap))
        {
            goto LFail;
        }
        cpMinUndo = LwMin(cp, cpMinUndo);
        cpLimUndo = LwMax(cp + ccpDel, cpLimUndo);
    }

    // now make sure that _pglmpe has enough room - we need at most 2
    // entries per sprm.
    if (!_pglmpe->FEnsureSpace(2 * (cspvmChp + cspvmPap)))
        goto LFail;

    if (!FSetUndo(cpMinUndo, cpLimUndo, cpLimUndo - cpMinUndo - ccpDel + ccpIns))
        goto LFail;

    if (pvNil != pflo)
    {
        Assert(cpSrc == 0 && ccpIns * SIZEOF(achar) == pflo->cb, "bad flo");
        if (!FReplaceFlo(pflo, fCopy, cp, ccpDel, koskCur, fdocNil))
            goto LCancel;
    }
    else if (pvNil != pbsf)
    {
        if (!FReplaceBsf(pbsf, cpSrc, ccpIns, cp, ccpDel, fdocNil))
            goto LCancel;
    }
    else
    {
        Assert(cpSrc == 0, "bad cpSrc");
        AssertPvCb(prgch, ccpIns * SIZEOF(achar));
        if (!FReplaceRgch(prgch, ccpIns, cp, ccpDel, fdocNil))
        {
        LCancel:
            CancelUndo();
        LFail:
            _ReleaseRgspvm(rgspvm, cspvmChp + cspvmPap);
            return fFalse;
        }
    }

    if (cspvmChp > 0)
        _ApplyRgspvm(cp, ccpIns, rgspvm, cspvmChp);
    if (cspvmPap > 0)
    {
        int32_t cpMin = cp;
        int32_t cpLim = cp + ccpIns;

        _GetParaBounds(&cpMin, &cpLim, fFalse);
        Assert(cpMin >= cpMinUndo, "new para before old one");
        Assert(cpLim <= cpLimUndo - ccpDel + ccpIns, "new para after old one");
        _ApplyRgspvm(cpMin, cpLim - cpMin, rgspvm + cspvmChp, cspvmPap);
    }
    CommitUndo();

    AssertThis(fobjAssertFull);
    InvalAllDdg(cpMinUndo, cpLimUndo - cpMinUndo - ccpDel + ccpIns, cpLimUndo - cpMinUndo, grfdoc);
    return fTrue;
}

/***************************************************************************
    Replace cp to cp + ccpDel with the characters in the given FLO.
***************************************************************************/
bool TXRD::FReplaceFlo(PFLO pflo, bool fCopy, int32_t cp, int32_t ccpDel, int16_t osk, uint32_t grfdoc)
{
    AssertThis(fobjAssertFull);
    AssertPo(pflo, 0);
    AssertIn(cp, 0, CpMac());
    AssertIn(ccpDel, 0, CpMac() - cp);
    int32_t ccpIns;
    FLO flo = *pflo;

    flo.pfil->AddRef();
    if (!flo.FTranslate(osk))
        goto LFail;

    ccpIns = flo.cb / SIZEOF(achar);
    if (CpMac() + ccpIns >= kcpMaxTxrd)
    {
        PushErc(ercRtxdTooMuchText);
        goto LFail;
    }

    if (!FSetUndo(cp, cp + ccpDel, ccpIns))
        goto LFail;

    if (!TXRD_PAR::FReplaceFlo(&flo, fCopy, cp, ccpDel, koskCur, fdocNil))
    {
        CancelUndo();
    LFail:
        ReleasePpo(&flo.pfil);
        return fFalse;
    }
    ReleasePpo(&flo.pfil);

    _AdjustMpe(cp, ccpIns, ccpDel);
    CommitUndo();
    AssertThis(fobjAssertFull);
    InvalAllDdg(cp, ccpIns, ccpDel, grfdoc);
    return fTrue;
}

/***************************************************************************
    Replace cp to cp + ccpDel with the characters in the given FLO, using
    the given chp and pap. pchp and/or ppap can be nil.
***************************************************************************/
bool TXRD::FReplaceFlo(PFLO pflo, bool fCopy, int32_t cp, int32_t ccpDel, PCHP pchp, PPAP ppap, int16_t osk,
                       uint32_t grfdoc)
{
    AssertThis(0);
    AssertPo(pflo, 0);
    AssertIn(cp, 0, CpMac());
    AssertIn(ccpDel, 0, CpMac() - cp);
    AssertNilOrVarMem(pchp);
    AssertNilOrVarMem(ppap);
    bool fRet;
    FLO flo = *pflo;

    flo.pfil->AddRef();
    fRet = flo.FTranslate(osk) && _FReplaceCore(pvNil, &flo, fCopy && pflo->pfil == flo.pfil, pvNil, 0, pflo->cb, cp,
                                                ccpDel, pchp, ppap, grfdoc);
    ReleasePpo(&flo.pfil);
    return fRet;
}

/***************************************************************************
    Replace cp to cpDst + ccpDel with ccpSrc characters from pbsfSrc starting
    at cpSrc.
***************************************************************************/
bool TXRD::FReplaceBsf(PBSF pbsfSrc, int32_t cpSrc, int32_t ccpSrc, int32_t cpDst, int32_t ccpDel, uint32_t grfdoc)
{
    AssertThis(fobjAssertFull);
    AssertPo(pbsfSrc, 0);
    AssertIn(cpSrc, 0, pbsfSrc->IbMac() / SIZEOF(achar) + 1);
    AssertIn(ccpSrc, 0, pbsfSrc->IbMac() / SIZEOF(achar) + 1 - cpSrc);
    AssertIn(cpDst, 0, CpMac());
    AssertIn(ccpDel, 0, CpMac() - cpDst);

    if (CpMac() + ccpSrc >= kcpMaxTxrd)
    {
        PushErc(ercRtxdTooMuchText);
        return fFalse;
    }

    if (!FSetUndo(cpDst, cpDst + ccpDel, ccpSrc))
        return fFalse;

    if (!TXRD_PAR::FReplaceBsf(pbsfSrc, cpSrc, ccpSrc, cpDst, ccpDel, fdocNil))
    {
        CancelUndo();
        return fFalse;
    }

    _AdjustMpe(cpDst, ccpSrc, ccpDel);
    CommitUndo();
    AssertThis(fobjAssertFull);
    InvalAllDdg(cpDst, ccpSrc, ccpDel, grfdoc);
    return fTrue;
}

/***************************************************************************
    Replace cp to cpDst + ccpDel with ccpSrc characters from pbsfSrc starting
    at cpSrc, using the given chp and pap. pchp and/or ppap can be nil.
***************************************************************************/
bool TXRD::FReplaceBsf(PBSF pbsfSrc, int32_t cpSrc, int32_t ccpSrc, int32_t cpDst, int32_t ccpDel, PCHP pchp, PPAP ppap,
                       uint32_t grfdoc)
{
    AssertThis(0);
    AssertPo(pbsfSrc, 0);
    AssertIn(cpSrc, 0, pbsfSrc->IbMac() + 1);
    AssertIn(ccpSrc, 0, pbsfSrc->IbMac() + 1 - cpSrc);
    AssertIn(cpDst, 0, CpMac());
    AssertIn(ccpDel, 0, CpMac() - cpDst);
    AssertNilOrVarMem(pchp);
    AssertNilOrVarMem(ppap);

    return _FReplaceCore(pvNil, pvNil, fFalse, pbsfSrc, cpSrc, ccpSrc, cpDst, ccpDel, pchp, ppap, grfdoc);
}

/***************************************************************************
    Replace cp to cpDst + ccpDel with ccpSrc characters from pbsfSrc starting
    at cpSrc, using the given chp and pap. pchp and/or ppap can be nil.
***************************************************************************/
bool TXRD::FReplaceTxtb(PTXTB ptxtbSrc, int32_t cpSrc, int32_t ccpSrc, int32_t cpDst, int32_t ccpDel, uint32_t grfdoc)
{
    AssertThis(0);
    AssertPo(ptxtbSrc, 0);
    AssertIn(cpSrc, 0, ptxtbSrc->CpMac() + 1);
    AssertIn(ccpSrc, 0, ptxtbSrc->CpMac() + 1 - cpSrc);
    AssertIn(cpDst, 0, CpMac());
    AssertIn(ccpDel, 0, CpMac() - cpDst);

    // NOTE: this cast to PTXRD is just a rude hack to get around
    // an oddity of C++. TXRD cannot access _pbsf in a TXTB, only in a TXRD.
    // ptxtbSrc is probably not a TXRD, but the cast still works.
    return FReplaceBsf(((PTXRD)ptxtbSrc)->_pbsf, cpSrc, ccpSrc, cpDst, ccpDel, grfdoc);
}

/***************************************************************************
    Replace cp to cpDst + ccpDel with ccpSrc characters from pbsfSrc starting
    at cpSrc, using the given chp and pap. pchp and/or ppap can be nil.
***************************************************************************/
bool TXRD::FReplaceTxtb(PTXTB ptxtbSrc, int32_t cpSrc, int32_t ccpSrc, int32_t cpDst, int32_t ccpDel, PCHP pchp,
                        PPAP ppap, uint32_t grfdoc)
{
    AssertThis(0);
    AssertPo(ptxtbSrc, 0);
    AssertIn(cpSrc, 0, ptxtbSrc->CpMac() + 1);
    AssertIn(ccpSrc, 0, ptxtbSrc->CpMac() + 1 - cpSrc);
    AssertIn(cpDst, 0, CpMac());
    AssertIn(ccpDel, 0, CpMac() - cpDst);
    AssertNilOrVarMem(pchp);
    AssertNilOrVarMem(ppap);

    // NOTE: this cast to PTXRD is just a rude hack to get around
    // an oddity of C++. TXRD cannot access _pbsf in a TXTB, only in a TXRD.
    // ptxtbSrc is probably not a TXRD, but the cast still works.
    return _FReplaceCore(pvNil, pvNil, fFalse, ((PTXRD)ptxtbSrc)->_pbsf, cpSrc, ccpSrc, cpDst, ccpDel, pchp, ppap,
                         grfdoc);
}

/***************************************************************************
    Replace stuff in this document with stuff in the given document.
    REVIEW shonk: this doesn't preserve font size if the two docs have
    different default font sizes!
***************************************************************************/
bool TXRD::FReplaceTxrd(PTXRD ptxrd, int32_t cpSrc, int32_t ccpSrc, int32_t cpDst, int32_t ccpDel, uint32_t grfdoc)
{
    AssertThis(fobjAssertFull);
    AssertPo(ptxrd, 0);
    AssertIn(cpSrc, 0, ptxrd->CpMac());
    AssertIn(ccpSrc, 0, ptxrd->CpMac() + 1 - cpSrc);
    AssertIn(cpDst, 0, CpMac());
    AssertIn(ccpDel, 0, CpMac() + 1 - cpDst);
    Assert(ptxrd != this, "can't copy from a rich text doc to itself!");
    int32_t dcp = 0;

    // REVIEW shonk: is there an easy way to make this atomic?

    if (CpMac() + ccpSrc >= kcpMaxTxrd)
    {
        PushErc(ercRtxdTooMuchText);
        return fFalse;
    }

    if (!FSetUndo(cpDst, cpDst + ccpDel, ccpSrc))
        return fFalse;

    // protect the trailing EOP
    if (cpDst + ccpDel == CpMac())
    {
#ifdef DEBUG
        achar ch;
        ptxrd->FetchRgch(cpSrc + ccpSrc - 1, 1, &ch);
        Assert(ch == kchReturn, "trying to replace trailing EOP");
#endif // DEBUG
        Assert(ccpDel > 0 && ccpSrc > 0, "bad parameters to FReplaceTxrd");
        ccpDel--;
        ccpSrc--;
        dcp = 1;
    }

    // insert the text
    if ((ccpSrc > 0 || ccpDel > 0) && !FReplaceBsf(ptxrd->_pbsf, cpSrc, ccpSrc, cpDst, ccpDel, fdocNil))
    {
        CancelUndo();
        return fFalse;
    }

    if (ccpSrc > 0)
        _CopyProps(ptxrd, cpSrc, cpDst, ccpSrc, ccpSrc, 0, sprmMinPap);

    if (ccpSrc + dcp > 0)
    {
        int32_t ccpSrcPap;
        int32_t cpMinPap = cpDst;
        int32_t cpLimPap = cpDst + ccpSrc;

        _GetParaBounds(&cpMinPap, &cpLimPap, fFalse);
        ccpSrcPap = ccpSrc - cpMinPap + cpDst;
        if (ccpSrcPap > 0 && cpMinPap < cpLimPap)
        {
            _CopyProps(ptxrd, cpSrc + cpMinPap - cpDst, cpMinPap, ccpSrcPap, cpLimPap - cpMinPap, sprmMinPap,
                       sprmMinObj);
        }
    }

    if (ccpSrc > 0)
    {
        // object properties
        int32_t cp;
        uint8_t sprm;
        int32_t impe, impeNew;
        MPE mpe, mpeNew;
        void *pv;
        bool fRet;

        ptxrd->_FFindMpe(_SpcpFromSprmCp(sprmMinObj, cpSrc), pvNil, pvNil, &impe);
        while (impe < ptxrd->_pglmpe->IvMac())
        {
            ptxrd->_pglmpe->Get(impe++, &mpe);
            cp = _CpFromSpcp(mpe.spcp);
            if (!FIn(cp, cpSrc, cpSrc + ccpSrc))
                continue;
            sprm = _SprmFromSpcp(mpe.spcp);
            mpeNew.spcp = _SpcpFromSprmCp(sprm, cpDst + cp - cpSrc);
            pv = ptxrd->_pagcact->PvLock(mpe.lw - 1);
            fRet = _FEnsureInAg(sprm, pv, ptxrd->_pagcact->Cb(mpe.lw - 1), &mpeNew.lw);
            ptxrd->_pagcact->Unlock();
            if (!fRet)
                continue;

            // insert the object sprm
            if (_FFindMpe(mpeNew.spcp, pvNil, pvNil, &impeNew))
                impeNew++;
            if (!_pglmpe->FInsert(impeNew, &mpeNew))
                _ReleaseInAg(mpeNew.lw);
        }
    }

    CommitUndo();
    AssertThis(fobjAssertFull);
    InvalAllDdg(cpDst, ccpSrc + dcp, ccpDel + dcp, grfdoc);
    return fTrue;
}

/***************************************************************************
    Copy properties from a TXRD to this one. Properties from sprmMin to
    sprmLim are copied.
***************************************************************************/
void TXRD::_CopyProps(PTXRD ptxrd, int32_t cpSrc, int32_t cpDst, int32_t ccpSrc, int32_t ccpDst, uint8_t sprmMin,
                      uint8_t sprmLim)
{
    AssertThis(0);
    AssertPo(ptxrd, 0);
    AssertIn(cpSrc, 0, ptxrd->CpMac());
    AssertIn(cpDst, 0, CpMac());
    AssertIn(ccpSrc, 1, ptxrd->CpMac() + 1 - cpSrc);
    AssertIn(ccpDst, 1, CpMac() + 1 - cpDst);
    SPVM spvm;
    uint8_t sprm;
    int32_t impe;
    int32_t cpMin, cpLim;
    bool fRet;
    int32_t lw;
    int32_t cb;
    void *pv;

    // zero the character properties over the inserted text
    spvm.sprm = sprmNil;
    spvm.lw = 0;
    spvm.lwMask = -1;
    _FFindMpe(_SpcpFromSprmCp(sprmMin, 0), pvNil, pvNil, &impe);
    while (_FFetchProp(impe++, &sprm) && sprm < sprmLim)
    {
        if (spvm.sprm == sprm)
            continue;
        spvm.sprm = sprm;
        if (_pglmpe->FEnsureSpace(2))
            _ApplyRgspvm(cpDst, ccpDst, &spvm, 1);
        else
            Warn("growing _pglmpe failed");
    }

    // apply the source properties
    ptxrd->_FFindMpe(_SpcpFromSprmCp(sprmMin, 0), pvNil, pvNil, &impe);
    while (ptxrd->_FFetchProp(impe++, &sprm, &lw, &cpMin, &cpLim) && sprm < sprmLim)
    {
        // if this MPE doesn't overlap our source range, ignore it.
        if (cpLim <= cpSrc || cpMin >= cpSrc + ccpSrc)
            continue;

        // if this MPE goes to the end of the source range, also carry it
        // to the end of the destination
        if (cpLim >= cpSrc + ccpSrc)
            cpLim = cpSrc + ccpDst;

        // adjust the min and lim to destination cp values and restrict
        // them to [cpDst, cpDst + ccpDst).
        if ((cpMin += cpDst - cpSrc) < cpDst)
            cpMin = cpDst;
        if ((cpLim += cpDst - cpSrc) > cpDst + ccpDst)
            cpLim = cpDst + ccpDst;

        // if the range is empty, ignore it
        if (cpMin >= cpLim)
            continue;

        spvm.sprm = sprm;
        if (_FSprmInAg(sprm) && lw > 0)
        {
            cb = ptxrd->_pagcact->Cb(lw - 1);
            pv = ptxrd->_pagcact->PvLock(lw - 1);
            fRet = _FEnsureInAg(sprm, pv, cb, &spvm.lw);
            ptxrd->_pagcact->Unlock();
            if (!fRet)
            {
                Warn("ensure in ag failed");
                continue;
            }
        }
        else
            spvm.lw = lw;

        if (_pglmpe->FEnsureSpace(2))
            _ApplyRgspvm(cpMin, cpLim - cpMin, &spvm, 1);
        else
        {
            Warn("growing _pglmpe failed");
            _ReleaseRgspvm(&spvm, 1);
        }
    }
}

/***************************************************************************
    Find the first object at or after cpMin. Put its location in *pcp and
    allocate a buffer to hold its extra data and put it in *ppv (if ppv is
    not nil). If an object was found, but the buffer couldn't be allocated,
    *ppv is set to nil.
***************************************************************************/
bool TXRD::FFetchObject(int32_t cpMin, int32_t *pcp, void **ppv, int32_t *pcb)
{
    AssertThis(0);
    AssertIn(cpMin, 0, CpMac());
    AssertVarMem(pcp);
    AssertNilOrVarMem(ppv);
    AssertNilOrVarMem(pcb);

    MPE mpe;
    int32_t impe;
    int32_t cb;
    bool fRet;

    fRet = _FFindMpe(_SpcpFromSprmCp(sprmObject, cpMin), &mpe, pcp, &impe);
    if (fRet && cpMin == _CpFromSpcp(mpe.spcp))
        *pcp = cpMin;
    else if (*pcp >= CpMac())
        return fFalse;
    else
    {
        if (fRet)
            impe++;
        Assert(impe < _pglmpe->IvMac(), "bad cpLim from _FFindMpe");
        _pglmpe->Get(impe, &mpe);
        Assert(_CpFromSpcp(mpe.spcp) == *pcp, "_FFindMpe messed up");
    }

    if (pvNil != pcb)
        *pcb = _pagcact->Cb(mpe.lw - 1);
    if (pvNil != ppv)
    {
        mpe.lw--;
        AssertIn(mpe.lw, 0, _pagcact->IvMac());
        cb = _pagcact->Cb(mpe.lw);
        if (FAllocPv(ppv, cb, fmemNil, mprNormal))
            CopyPb((uint8_t *)_pagcact->QvGet(mpe.lw), *ppv, cb);
    }

    return fTrue;
}

/***************************************************************************
    Insert a picture into the rich text document.
***************************************************************************/
bool TXRD::FInsertObject(void *pv, int32_t cb, int32_t cp, int32_t ccpDel, PCHP pchp, uint32_t grfdoc)
{
    AssertThis(fobjAssertFull);
    AssertPvCb(pv, cb);
    AssertIn(cp, 0, CpMac());
    AssertIn(ccpDel, 0, CpMac() - cp);
    AssertNilOrVarMem(pchp);
    SPVM rgspvm[sprmLimChp - sprmMinChp];
    MPE mpe;
    int32_t impe;
    achar ch = kchObject;
    int32_t cspvmChp = 0;

    BumpCombineUndo();
    if (!FSetUndo(cp, cp + ccpDel, 1))
        return fFalse;

    mpe.spcp = _SpcpFromSprmCp(sprmObject, cp);
    if (!_FEnsureInAg(sprmObject, pv, cb, &mpe.lw))
        goto LFail;

    if (pvNil != pchp && !_FGetRgspvmFromChp(pchp, pvNil, rgspvm, &cspvmChp))
        goto LFail;

    // now make sure that _pglmpe has enough room - we need at most 2
    // entries per sprm, plus one for the object
    if (!_pglmpe->FEnsureSpace(2 * cspvmChp + 1))
        goto LFail;

    if (!FReplaceRgch(&ch, 1, cp, ccpDel, fdocNil))
    {
        _ReleaseRgspvm(rgspvm, cspvmChp);
    LFail:
        _ReleaseSprmLw(sprmObject, mpe.lw);
        CancelUndo();
        return fFalse;
    }

    if (cspvmChp > 0)
        _ApplyRgspvm(cp, 1, rgspvm, cspvmChp);

    // insert the object sprm
    if (_FFindMpe(mpe.spcp, pvNil, pvNil, &impe))
        impe++;
    AssertDo(_pglmpe->FInsert(impe, &mpe), "should have been ensured");
    CommitUndo();
    BumpCombineUndo();

    AssertThis(fobjAssertFull);
    InvalAllDdg(cp, 1, ccpDel, grfdoc);
    return fTrue;
}

/***************************************************************************
    Insert a picture into the rich text document.
***************************************************************************/
bool TXRD::FApplyObjectProps(void *pv, int32_t cb, int32_t cp, uint32_t grfdoc)
{
    AssertThis(fobjAssertFull);
    AssertPvCb(pv, cb);
    AssertIn(cp, 0, CpMac());
    MPE mpe;
    int32_t impe;
    int32_t lwOld;

    if (!_FFindMpe(_SpcpFromSprmCp(sprmObject, cp), &mpe, pvNil, &impe) || cp != _CpFromSpcp(mpe.spcp))
    {
        Bug("cp not an object");
        return fFalse;
    }

    BumpCombineUndo();
    if (!FSetUndo(cp, cp + 1, 1))
        return fFalse;

    lwOld = mpe.lw;
    if (!_FEnsureInAg(sprmObject, pv, cb, &mpe.lw))
    {
        CancelUndo();
        return fFalse;
    }

    _pglmpe->Put(impe, &mpe);
    _ReleaseSprmLw(sprmObject, lwOld);
    CommitUndo();
    BumpCombineUndo();

    AssertThis(fobjAssertFull);
    InvalAllDdg(cp, 1, 1, grfdoc);
    return fTrue;
}

/***************************************************************************
    Get the bounds of an object.
***************************************************************************/
bool TXRD::FGetObjectRc(int32_t cp, PGNV pgnv, PCHP pchp, RC *prc)
{
    AssertThis(0);
    AssertIn(cp, 0, CpMac());
    AssertPo(pgnv, 0);
    AssertVarMem(pchp);
    AssertVarMem(prc);
    MPE mpe;
    uint32_t spcp = _SpcpFromSprmCp(sprmObject, cp);

    if (!_FFindMpe(spcp, &mpe) || _CpFromSpcp(mpe.spcp) != cp)
        return fFalse;

    return _FGetObjectRc(mpe.lw - 1, _SprmFromSpcp(mpe.spcp), pgnv, pchp, prc);
}

/***************************************************************************
    Get the object bounds from the AG entry.
***************************************************************************/
bool TXRD::_FGetObjectRc(int32_t icact, uint8_t sprm, PGNV pgnv, PCHP pchp, RC *prc)
{
    AssertIn(icact, 0, _pagcact->IvMac());
    Assert(sprm >= sprmObject, 0);
    AssertPo(pgnv, 0);
    AssertVarMem(pchp);
    AssertVarMem(prc);

    // TXRD has no acceptable object types
    TrashVar(prc);
    return fFalse;
}

/***************************************************************************
    Draw an object.
***************************************************************************/
bool TXRD::FDrawObject(int32_t cp, PGNV pgnv, int32_t *pxp, int32_t yp, PCHP pchp, RC *prcClip)
{
    AssertThis(0);
    AssertIn(cp, 0, CpMac());
    AssertPo(pgnv, 0);
    AssertVarMem(pxp);
    AssertVarMem(pchp);
    AssertVarMem(prcClip);
    MPE mpe;
    uint32_t spcp = _SpcpFromSprmCp(sprmObject, cp);

    if (!_FFindMpe(spcp, &mpe) || _CpFromSpcp(mpe.spcp) != cp)
        return fFalse;

    return _FDrawObject(mpe.lw - 1, _SprmFromSpcp(mpe.spcp), pgnv, pxp, yp, pchp, prcClip);
}

/***************************************************************************
    Draw the object.
***************************************************************************/
bool TXRD::_FDrawObject(int32_t icact, uint8_t sprm, PGNV pgnv, int32_t *pxp, int32_t yp, PCHP pchp, RC *prcClip)
{
    AssertIn(icact, 0, _pagcact->IvMac());
    Assert(sprm >= sprmObject, 0);
    AssertPo(pgnv, 0);
    AssertVarMem(pxp);
    AssertVarMem(pchp);
    AssertVarMem(prcClip);

    // TXRD has no acceptable object types
    return fFalse;
}

/***************************************************************************
    Create a new rich text undo object for the given rich text document.
***************************************************************************/
PRTUN RTUN::PrtunNew(int32_t cactCombine, PTXRD ptxrd, int32_t cp1, int32_t cp2, int32_t ccpIns)
{
    AssertPo(ptxrd, 0);
    AssertIn(cp1, 0, ptxrd->CpMac() + 1);
    AssertIn(cp2, 0, ptxrd->CpMac() + 1);
    AssertIn(ccpIns, 0, kcbMax);
    PRTUN prtun;

    SortLw(&cp1, &cp2);
    AssertIn(cp1, 0, ptxrd->CpMac());
    if (pvNil == (prtun = NewObj RTUN))
        return pvNil;

    prtun->_cactCombine = cactCombine;
    prtun->_cpMin = cp1;
    prtun->_ccpIns = ccpIns;
    if (cp1 < cp2)
    {
        // copy the piece of the txrd.
        if (pvNil == (prtun->_ptxrd = TXRD::PtxrdNew(pvNil)))
            goto LFail;
        prtun->_ptxrd->SetInternal();
        prtun->_ptxrd->SuspendUndo();
        if (!prtun->_ptxrd->FReplaceTxrd(ptxrd, cp1, cp2 - cp1, 0, 0))
        {
        LFail:
            ReleasePpo(&prtun);
        }
    }

    AssertNilOrPo(prtun, 0);
    return prtun;
}

/***************************************************************************
    Destructor for a rich text undo object.
***************************************************************************/
RTUN::~RTUN(void)
{
    ReleasePpo(&_ptxrd);
}

/***************************************************************************
    Undo this rich text undo object on the given document.
***************************************************************************/
bool RTUN::FUndo(PDOCB pdocb)
{
    AssertThis(0);
    AssertPo(pdocb, 0);
    PTXRD ptxrd;
    int32_t ccpIns;
    PTXRD ptxrdNew = pvNil;

    if (!pdocb->FIs(kclsTXRD))
        goto LAssert;
    ptxrd = (PTXRD)pdocb;
    AssertPo(ptxrd, 0);

    if (!FIn(_cpMin, 0, ptxrd->CpMac()) || !FIn(_ccpIns, 0, ptxrd->CpMac() + 1 - _cpMin))
    {
    LAssert:
        Bug("This rich text undo object cannot be applied to this document");
        return fFalse;
    }

    if (_ccpIns > 0)
    {
        // copy the piece of the txrd.
        if (pvNil == (ptxrdNew = TXRD::PtxrdNew(pvNil)))
            return fFalse;
        ptxrdNew->SetInternal();
        ptxrdNew->SuspendUndo();
        if (!ptxrdNew->FReplaceTxrd(ptxrd, _cpMin, _ccpIns, 0, 0))
            goto LFail;
    }

    ptxrd->SuspendUndo();
    ptxrd->HideSel();
    if (pvNil == _ptxrd)
    {
        ccpIns = 0;
        if (!ptxrd->FReplaceRgch(pvNil, 0, _cpMin, _ccpIns))
            goto LFail;
    }
    else
    {
        ccpIns = _ptxrd->CpMac() - 1;
        if (!ptxrd->FReplaceTxrd(_ptxrd, 0, ccpIns, _cpMin, _ccpIns))
        {
        LFail:
            ReleasePpo(&ptxrdNew);
            ptxrd->ResumeUndo();
            return fFalse;
        }
    }
    ptxrd->ResumeUndo();
    ptxrd->SetSel(_cpMin, _cpMin + ccpIns, ginNil);
    ptxrd->ShowSel();
    ptxrd->SetSel(_cpMin + ccpIns, _cpMin + ccpIns);

    ReleasePpo(&_ptxrd);
    _ptxrd = ptxrdNew;
    _ccpIns = ccpIns;
    return fTrue;
}

/***************************************************************************
    Redo this rich text undo object on the given document.
***************************************************************************/
bool RTUN::FDo(PDOCB pdocb)
{
    AssertThis(0);
    return FUndo(pdocb);
}

/***************************************************************************
    If possible, combine the given rtun with this one. Returns success.
***************************************************************************/
bool RTUN::FCombine(PRTUN prtun)
{
    AssertThis(0);
    AssertPo(prtun, 0);
    int32_t ccp;

    // if the _cactCombine numbers are different, they can't be combined
    if (prtun->_cactCombine != _cactCombine)
        return fFalse;

    // if the new record doesn't delete anything and the new text is
    // at the end of the old text, just adjust _ccpIns.
    if (prtun->_ptxrd == pvNil && _cpMin + _ccpIns == prtun->_cpMin)
    {
        _ccpIns += prtun->_ccpIns;
        return fTrue;
    }

    // if either of the new records inserts anything, we can't combine the two
    if (prtun->_ccpIns != 0 || _ccpIns != 0)
        return fFalse;

    // handle repeated delete keys
    AssertPo(_ptxrd, 0);
    AssertPo(prtun->_ptxrd, 0);
    ccp = prtun->_ptxrd->CpMac() - 1;
    if (prtun->_cpMin == _cpMin)
    {
        return _ptxrd->FReplaceTxrd(prtun->_ptxrd, 0, ccp, _ptxrd->CpMac() - 1, 0);
    }

    // handle repeated backspace keys
    if (prtun->_cpMin + ccp == _cpMin)
    {
        if (!_ptxrd->FReplaceTxrd(prtun->_ptxrd, 0, ccp, 0, 0))
            return fFalse;
        _cpMin = prtun->_cpMin;
        return fTrue;
    }

    return fFalse;
}

#ifdef DEBUG
/***************************************************************************
    Assert the validity of a RTUN.
***************************************************************************/
void RTUN::AssertValid(uint32_t grf)
{
    RTUN_PAR::AssertValid(grf);
    AssertNilOrPo(_ptxrd, 0);
    AssertIn(_cpMin, 0, kcbMax);
    AssertIn(_ccpIns, 0, kcbMax);
    Assert(_ccpIns > 0 || _ptxrd != pvNil, "empty RTUN");
}

/***************************************************************************
    Mark memory for the RTUN.
***************************************************************************/
void RTUN::MarkMem(void)
{
    AssertThis(fobjAssertFull);
    RTUN_PAR::MarkMem();
    MarkMemObj(_ptxrd);
}
#endif // DEBUG
