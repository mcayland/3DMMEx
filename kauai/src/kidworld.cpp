/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Copyright (c) Microsoft Corporation

    This is a class that knows how to create GOKs, Help Balloons and
    Kidspace script interpreters. It exists so an app can customize
    default behavior.

***************************************************************************/
#include "kidframe.h"
ASSERTNAME

RTCLASS(GOKD)
RTCLASS(GKDS)
RTCLASS(WOKS)

/***************************************************************************
    Static method to read a GKDS from the CRF. This is a CRF object reader.
***************************************************************************/
bool GKDS::FReadGkds(PCRF pcrf, CTG ctg, CNO cno, PBLCK pblck, PBACO *ppbaco, int32_t *pcb)
{
    PGKDS pgkds;
    GOKDF gokdf;
    HQ hq;
    LOP *qlop;
    int32_t cb;

    *pcb = pblck->Cb(fTrue);
    if (pvNil == ppbaco)
        return fTrue;

    if (!pblck->FUnpackData())
        return fFalse;
    *pcb = pblck->Cb();

    if (*pcb < SIZEOF(GOKDF) + SIZEOF(LOP) || CbRoundToLong(*pcb) != *pcb)
    {
        Bug("Bad GOKD");
        return fFalse;
    }

    if (!pblck->FReadRgb(&gokdf, SIZEOF(GOKDF), 0))
        return fFalse;
    if (gokdf.bo == kboOther)
        SwapBytesBom(&gokdf, kbomGokdf);
    else if (gokdf.bo != kboCur)
    {
        Bug("Bad GOKD 2");
        return fFalse;
    }

    if (!pblck->FMoveMin(SIZEOF(gokdf)))
        return fFalse;
    hq = pblck->HqFree();
    if (hqNil == hq)
        return fFalse;
    cb = CbOfHq(hq);

    if (pvNil == (pgkds = NewObj GKDS))
    {
        FreePhq(&hq);
        return fFalse;
    }
    pgkds->_hqData = hq;
    pgkds->_gokk = gokdf.gokk;
    if (gokdf.bo == kboOther)
        SwapBytesRglw(QvFromHq(hq), cb / SIZEOF(int32_t));

    qlop = (LOP *)QvFromHq(hq);
    for (pgkds->_clop = 0;; qlop++)
    {
        if (cb < SIZEOF(LOP))
        {
            Bug("Bad LOP list in GOKD");
            ReleasePpo(&pgkds);
            return fFalse;
        }
        pgkds->_clop++;
        cb -= SIZEOF(LOP);
        if (hidNil == qlop->hidPar)
            break;
    }
    if ((cb % SIZEOF(CUME)) != 0)
    {
        Bug("Bad CUME list in GOKD");
        ReleasePpo(&pgkds);
        return fFalse;
    }
    pgkds->_ccume = cb / SIZEOF(CUME);
    *ppbaco = pgkds;
    return fTrue;
}

/***************************************************************************
    Destructor for a GKDS object.
***************************************************************************/
GKDS::~GKDS(void)
{
    FreePhq(&_hqData);
}

#ifdef DEBUG
/***************************************************************************
    Assert the validity of a GKDS.
***************************************************************************/
void GKDS::AssertValid(uint32_t grf)
{
    LOP *qrglop;

    GKDS_PAR::AssertValid(0);
    AssertHq(_hqData);
    AssertIn(_clop, 0, kcbMax);
    AssertIn(_ccume, 0, kcbMax);
    Assert(LwMul(_clop, SIZEOF(LOP)) + LwMul(_ccume, SIZEOF(CUME)) == CbOfHq(_hqData), "GKDS _hqData wrong size");
    qrglop = (LOP *)QvFromHq(_hqData);
    Assert(qrglop[_clop - 1].hidPar == hidNil, "bad rglop in GKDS");
}

/***************************************************************************
    Mark memory for the GKDS.
***************************************************************************/
void GKDS::MarkMem(void)
{
    AssertValid(0);
    GKDS_PAR::MarkMem();
    MarkHq(_hqData);
}
#endif // DEBUG

/***************************************************************************
    Return the GOK kind id.
***************************************************************************/
int32_t GKDS::Gokk(void)
{
    AssertThis(0);
    return _gokk;
}

/***************************************************************************
    Look for a cursor map entry in this GKDS.
***************************************************************************/
bool GKDS::FGetCume(uint32_t grfcust, int32_t sno, CUME *pcume)
{
    AssertThis(0);
    AssertVarMem(pcume);
    CUME *qcume;
    int32_t ccume;
    uint32_t fbitSno = (1L << (sno & 0x1F));

    if (0 == _ccume)
        return fFalse;

    qcume = (CUME *)PvAddBv(QvFromHq(_hqData), LwMul(_clop, SIZEOF(LOP)));
    for (ccume = _ccume; ccume > 0; ccume--, qcume++)
    {
        if ((qcume->grfbitSno & fbitSno) && (qcume->grfcustMask & grfcust) == qcume->grfcust)
        {
            *pcume = *qcume;
            return fTrue;
        }
    }
    TrashVar(pcume);
    return fFalse;
}

/***************************************************************************
    Get the location map entry from the parent id.
***************************************************************************/
void GKDS::GetLop(int32_t hidPar, LOP *plop)
{
    AssertThis(0);
    AssertVarMem(plop);
    LOP *qlop;

    for (qlop = (LOP *)QvFromHq(_hqData);; qlop++)
    {
        if (hidNil == qlop->hidPar || hidPar == qlop->hidPar)
            break;
    }
    *plop = *qlop;
}

/***************************************************************************
    Constructor for a World of Kidspace GOB.
***************************************************************************/
WOKS::WOKS(GCB *pgcb, PSTRG pstrg)
    : WOKS_PAR(pgcb), _clokAnim(CMH::HidUnique()), _clokNoSlip(CMH::HidUnique(), fclokNoSlip),
      _clokGen(CMH::HidUnique()), _clokReset(CMH::HidUnique(), fclokReset)
{
    AssertThis(0);
    AssertNilOrPo(pstrg, 0);

    if (pvNil == pstrg)
        pstrg = &_strg;
    _pstrg = pstrg;
    _pstrg->AddRef();

    _clokAnim.Start(0);
    _clokNoSlip.Start(0);
    _clokGen.Start(0);
    _clokReset.Start(0);
}

/***************************************************************************
    Destructor for a kidspace world.
***************************************************************************/
WOKS::~WOKS(void)
{
    ReleasePpo(&_pstrg);
}

#ifdef DEBUG
/***************************************************************************
    Assert the validity of a WOKS.
***************************************************************************/
void WOKS::AssertValid(uint32_t grf)
{
    WOKS_PAR::AssertValid(0);
    AssertPo(&_strg, 0);
}

/***************************************************************************
    Mark memory for the WOKS.
***************************************************************************/
void WOKS::MarkMem(void)
{
    AssertValid(0);
    WOKS_PAR::MarkMem();
    MarkMemObj(&_strg);
}
#endif // DEBUG

/***************************************************************************
    Return whether the GOB is in this kidspace world.
***************************************************************************/
bool WOKS::FGobIn(PGOB pgob)
{
    AssertThis(0);
    AssertPo(pgob, 0);

    for (; pgob != this; pgob = pgob->PgobPar())
    {
        if (pgob == pvNil)
            return fFalse;
    }

    return fTrue;
}

/***************************************************************************
    Get a GOKD from the given chunk.
***************************************************************************/
PGOKD WOKS::PgokdFetch(CTG ctg, CNO cno, PRCA prca)
{
    AssertThis(0);
    AssertPo(prca, 0);

    return (PGOKD)prca->PbacoFetch(ctg, cno, GKDS::FReadGkds);
}

/***************************************************************************
    Create a new gob in this kidspace world.
***************************************************************************/
PGOK WOKS::PgokNew(PGOB pgobPar, int32_t hid, CNO cnoGokd, PRCA prca)
{
    AssertThis(0);
    AssertNilOrPo(pgobPar, 0);
    PGOKD pgokd;
    PGOK pgok;

    if (pgobPar == pvNil)
        pgobPar = this;
    else if (!FGobIn(pgobPar))
    {
        // parent isn't in this kidspace world
        Bug("Parent is not in this kidspace world");
        return pvNil;
    }

    if (hidNil == hid)
        hid = CMH::HidUnique();
    else if (pvNil != PcmhFromHid(hid))
    {
        BugVar("command handler with this ID already exists", &hid);
        return pvNil;
    }

    if (pvNil == (pgokd = PgokdFetch(kctgLGokd, cnoGokd, prca)))
        return pvNil;

    pgok = GOK::PgokNew(this, pgobPar, hid, pgokd, prca);
    ReleasePpo(&pgokd);

    return pgok;
}

/***************************************************************************
    Create a new script interpreter for this kidspace world.
***************************************************************************/
PSCEG WOKS::PscegNew(PRCA prca, PGOB pgob)
{
    AssertThis(0);
    AssertPo(prca, 0);
    AssertPo(pgob, 0);

    return NewObj SCEG(this, prca, pgob);
}

/***************************************************************************
    Create a new help balloon.
***************************************************************************/
PHBAL WOKS::PhbalNew(PGOB pgobPar, PRCA prca, CNO cnoTopic, PHTOP phtop)
{
    AssertThis(0);
    AssertNilOrPo(pgobPar, 0);
    AssertPo(prca, 0);
    AssertNilOrVarMem(phtop);

    if (pgobPar == pvNil)
        pgobPar = this;
    else if (!FGobIn(pgobPar))
    {
        // parent isn't in this kidspace world
        Bug("Parent is not in this kidspace world");
        return pvNil;
    }

    return HBAL::PhbalCreate(this, pgobPar, prca, cnoTopic, phtop);
}

/***************************************************************************
    Get the command handler for this hid.
***************************************************************************/
PCMH WOKS::PcmhFromHid(int32_t hid)
{
    AssertThis(0);
    PCMH pcmh;

    switch (hid)
    {
    case hidNil:
        return pvNil;

    case khidApp:
        return vpappb;
    }

    if (pvNil != (pcmh = PclokFromHid(hid)))
        return pcmh;
    return PgobFromHid(hid);
}

/***************************************************************************
    Get the clock having the given hid.
***************************************************************************/
PCLOK WOKS::PclokFromHid(int32_t hid)
{
    AssertThis(0);

    if (khidClokGokGen == hid)
        return &_clokGen;
    if (khidClokGokReset == hid)
        return &_clokReset;

    return CLOK::PclokFromHid(hid);
}

/***************************************************************************
    Get the parent gob of the given gob. This is here so a kidspace world
    can limit what scripts can get to.
***************************************************************************/
PGOB WOKS::PgobParGob(PGOB pgob)
{
    AssertThis(0);
    AssertPo(pgob, 0);

    pgob = pgob->PgobPar();
    if (pvNil == pgob || !FGobIn(pgob))
        return pvNil;
    return pgob;
}

/***************************************************************************
    Find a file given a string.
***************************************************************************/
bool WOKS::FFindFile(PSTN pstnSrc, PFNI pfni)
{
    AssertThis(0);
    AssertPo(pstnSrc, 0);
    AssertPo(pfni, 0);

    return pfni->FBuildFromPath(pstnSrc);
}

/***************************************************************************
    Put up an alert (and don't return until it is dismissed).
***************************************************************************/
tribool WOKS::TGiveAlert(PSTN pstn, int32_t bk, int32_t cok)
{
    AssertThis(0);

    return vpappb->TGiveAlertSz(pstn->Psz(), bk, cok);
}

/***************************************************************************
    Put up an alert (and don't return until it is dismissed).
***************************************************************************/
void WOKS::Print(PSTN pstn)
{
    AssertThis(0);
    AssertPo(pstn, 0);

    // REVIEW shonk: implement WOKS::Print better
#ifdef WIN
    OutputDebugString(pstn->Psz());
    OutputDebugString(PszLit("\n"));
#endif // WIN
}

/***************************************************************************
    Return the current cursor state. This takes the frame cursor state from
    vpappb and the rest from this kidspace world.
***************************************************************************/
uint32_t WOKS::GrfcustCur(bool fAsynch)
{
    AssertThis(0);

    return GrfcustAdjust(vpappb->GrfcustCur(fAsynch));
}

/***************************************************************************
    Modify the current cursor state. This sets the frame values in vpappb
    and the rest in this kidspace world.
***************************************************************************/
void WOKS::ModifyGrfcust(uint32_t grfcustOr, uint32_t grfcustXor)
{
    AssertThis(0);

    // write all the bits to the app. Also adjust our internal _grfcust
    vpappb->ModifyGrfcust(grfcustOr, grfcustXor);

    _grfcust |= grfcustOr;
    _grfcust ^= grfcustXor;
}

/***************************************************************************
    Adjust the given grfcust (take the Frame bits from it and combine with
    our other bits).
***************************************************************************/
uint32_t WOKS::GrfcustAdjust(uint32_t grfcust)
{
    AssertThis(0);

    grfcust &= kgrfcustFrame;
    grfcust |= _grfcust & (kgrfcustKid | kgrfcustApp);
    return grfcust;
}

/***************************************************************************
    Do a modal help topic.
***************************************************************************/
bool WOKS::FModalTopic(PRCA prca, CNO cnoTopic, int32_t *plwRet)
{
    AssertThis(0);
    AssertPo(prca, 0);
    AssertVarMem(plwRet);

    GCB gcb;
    PWOKS pwoksModal;
    GTE gte;
    PGOB pgob;
    uint32_t grfgte;
    bool fRet = fFalse;

    gte.Init(this, fgteNil);
    while (gte.FNextGob(&pgob, &grfgte, fgteNil))
    {
        if (!(grfgte & fgtePre) || !pgob->FIs(kclsGOK))
            continue;

        ((PGOK)pgob)->Suspend();
    }

    if (vpappb->FPushModal())
    {
        gcb.Set(CMH::HidUnique(), this, fgobNil, kginMark);
        gcb._rcRel.Set(0, 0, krelOne, krelOne);

        if (pvNil != (pwoksModal = NewObj WOKS(&gcb, _pstrg)))
        {
            vpsndm->PauseAll();
            vpcex->SetModalGob(pwoksModal);
            if (pvNil != pwoksModal->PhbalNew(pwoksModal, prca, cnoTopic))
                fRet = FPure(vpappb->FModalLoop(plwRet));
            vpcex->SetModalGob(pvNil);
            vpsndm->ResumeAll();
        }

        ReleasePpo(&pwoksModal);
        vpappb->PopModal();
    }

    gte.Init(this, fgteNil);
    while (gte.FNextGob(&pgob, &grfgte, fgteNil))
    {
        if (!(grfgte & fgtePre) || !pgob->FIs(kclsGOK))
            continue;

        ((PGOK)pgob)->Resume();
    }

    return fRet;
}
