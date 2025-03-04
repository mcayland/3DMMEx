/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

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
#ifndef KIDWORLD_H
#define KIDWORLD_H

/***************************************************************************
    Base GOK descriptor.
***************************************************************************/
// location from parent map structure
struct LOP
{
    int32_t hidPar;
    int32_t xp;
    int32_t yp;
    int32_t zp; // the z-plane number used for placing the GOK in the GOB tree
};
VERIFY_STRUCT_SIZE(LOP, 16);

// cursor map entry
struct CUME
{
    uint32_t grfcustMask; // what cursor states this CUME is good for
    uint32_t grfcust;
    uint32_t grfbitSno; // what button states this CUME is good for
    CNO cnoCurs;        // the cursor to use
    CHID chidScript;    // execution script (absolute)
    int32_t cidDefault; // default command
    CNO cnoTopic;       // tool tip topic
};
VERIFY_STRUCT_SIZE(CUME, 28);

typedef class GOKD *PGOKD;
#define GOKD_PAR BACO
#define kclsGOKD KLCONST4('G', 'O', 'K', 'D')
class GOKD : public GOKD_PAR
{
    RTCLASS_DEC

  protected:
    GOKD(void)
    {
    }

  public:
    virtual int32_t Gokk(void) = 0;
    virtual bool FGetCume(uint32_t grfcust, int32_t sno, CUME *pcume) = 0;
    virtual void GetLop(int32_t hidPar, LOP *plop) = 0;
};

/***************************************************************************
    Standard GOK descriptor. Contains location information and cursor
    map stuff.
***************************************************************************/
// GOK construction descriptor on file - these are stored in chunky resource files
struct GOKDF
{
    int16_t bo;
    int16_t osk;
    int32_t gokk;
    // LOP rglop[];		ends with a default entry (hidPar == hidNil)
    // CUME rgcume[];	the cursor map
};
VERIFY_STRUCT_SIZE(GOKDF, 8);
const BOM kbomGokdf = 0x0C000000;

typedef class GKDS *PGKDS;
#define GKDS_PAR GOKD
#define kclsGKDS KLCONST4('G', 'K', 'D', 'S')
class GKDS : public GKDS_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    HQ _hqData;
    int32_t _gokk;
    int32_t _clop;
    int32_t _ccume;

    GKDS(void)
    {
    }

  public:
    // An object reader for a GOKD.
    static bool FReadGkds(PCRF pcrf, CTG ctg, CNO cno, BLCK *pblck, PBACO *ppbaco, int32_t *pcb);
    ~GKDS(void);

    virtual int32_t Gokk(void);
    virtual bool FGetCume(uint32_t grfcust, int32_t sno, CUME *pcume);
    virtual void GetLop(int32_t hidPar, LOP *plop);
};

/***************************************************************************
    World of Kidspace class.
***************************************************************************/
typedef class WOKS *PWOKS;
#define WOKS_PAR GOB
#define kclsWOKS KLCONST4('W', 'O', 'K', 'S')
class WOKS : public WOKS_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PSTRG _pstrg;
    STRG _strg;
    uint32_t _grfcust;

    CLOK _clokAnim;
    CLOK _clokNoSlip;
    CLOK _clokGen;
    CLOK _clokReset;

  public:
    WOKS(GCB *pgcb, PSTRG pstrg = pvNil);
    ~WOKS(void);

    PSTRG Pstrg(void)
    {
        return _pstrg;
    }

    virtual bool FGobIn(PGOB pgob);
    virtual PGOKD PgokdFetch(CTG ctg, CNO cno, PRCA prca);
    virtual PGOK PgokNew(PGOB pgobPar, int32_t hid, CNO cno, PRCA prca);
    virtual PSCEG PscegNew(PRCA prca, PGOB pgob);
    virtual PHBAL PhbalNew(PGOB pgobPar, PRCA prca, CNO cnoTopic, PHTOP phtop = pvNil);
    virtual PCMH PcmhFromHid(int32_t hid);
    virtual PGOB PgobParGob(PGOB pgob);
    virtual bool FFindFile(PSTN pstnSrc, PFNI pfni);
    virtual tribool TGiveAlert(PSTN pstn, int32_t bk, int32_t cok);
    virtual void Print(PSTN pstn);

    virtual uint32_t GrfcustCur(bool fAsynch = fFalse);
    virtual void ModifyGrfcust(uint32_t grfcustOr, uint32_t grfcustXor);
    virtual uint32_t GrfcustAdjust(uint32_t grfcust);

    virtual bool FModalTopic(PRCA prca, CNO cnoTopic, int32_t *plwRet);
    virtual PCLOK PclokAnim(void)
    {
        return &_clokAnim;
    }
    virtual PCLOK PclokNoSlip(void)
    {
        return &_clokNoSlip;
    }
    virtual PCLOK PclokGen(void)
    {
        return &_clokGen;
    }
    virtual PCLOK PclokReset(void)
    {
        return &_clokReset;
    }
    virtual PCLOK PclokFromHid(int32_t hid);
};

#endif //! KIDWORLD_H
