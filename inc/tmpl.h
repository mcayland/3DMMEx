/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/*************************************************************************

    tmpl.h: Actor template class

    Primary Author: ******
    Review Status: REVIEWED - any changes to this file must be reviewed!

    BASE ---> BACO ---> ACTN
    BASE ---> BACO ---> TMPL

    A TMPL encapsulates all the data that distinguishes one actor
    "species" from another, including the species' models, actions,
    and custom texture maps.  One or more BODY classes are created based
    on a TMPL, and the TMPL attaches models and materials to the body
    based on more abstract concepts like actions and costumes.

*************************************************************************/
#ifndef TMPL_H
#define TMPL_H

/****************************************
    Cel part spec: tells what model and
    xfrm to apply to a body part for
    one cel
****************************************/
struct CPS
{
    int16_t chidModl; // CHID (under TMPL chunk) of model for this body part
    int16_t imat34;   // index into ACTN's GL of transforms
};
VERIFY_STRUCT_SIZE(CPS, 4);
const BOM kbomCps = 0x50000000;

/****************************************
    Cel: tells what CPS's to apply to an
    actor for one cel.  It also tells
    what sound to play (if any), and how
    far the actor should move from the
    previous cel (dwr).
****************************************/
struct CEL
{
    CHID chidSnd; // sound to play at this cel (CHID under ACTN chunk)
    BRS dwr;      // distance from previous cel
                  //	CPS rgcps[];	// list of cel part specs (variable part of pggcel)
};
VERIFY_STRUCT_SIZE(CEL, 8);
const BOM kbomCel = 0xf0000000;

// template on file
struct TMPLF
{
    int16_t bo;
    int16_t osk;
    BRA xaRest; // reminder: BRAs are shorts
    BRA yaRest;
    BRA zaRest;
    int16_t swPad; // so grftmpl (and the whole TMPLF) is long-aligned
    uint32_t grftmpl;
};
VERIFY_STRUCT_SIZE(TMPLF, 16);
#define kbomTmplf 0x554c0000

// action chunk on file
struct ACTNF
{
    int16_t bo;
    int16_t osk;
    int32_t grfactn;
};
VERIFY_STRUCT_SIZE(ACTNF, 8);
const uint32_t kbomActnf = 0x5c000000;

// grfactn flags
enum
{
    factnRotateX = 1, // Tells whether actor should rotate around this
    factnRotateY = 2, //   axis when following a path
    factnRotateZ = 4,
    factnStatic = 8, // Tells whether this is a stationary action
};

/****************************************
    ACTN (action): all the information
    for an action like 'rest' or 'walk'.
****************************************/
typedef class ACTN *PACTN;
#define ACTN_PAR BACO
#define kclsACTN KLCONST4('A', 'C', 'T', 'N')
class ACTN : public ACTN_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PGG _pggcel;       // GG of CELs; variable part is a rgcps[]
    PGL _pglbmat34;    // GL of transformation matrices used in this action
    PGL _pgltagSnd;    // GL of motion-match sounds for this action
    uint32_t _grfactn; // various flags for this action

  protected:
    ACTN(void)
    {
    } // can't instantiate directly; must use FReadActn
    bool _FInit(PCFL pcfl, CTG ctg, CNO cno);

  public:
    static PACTN PactnNew(PGG pggcel, PGL pglbmat34, uint32_t grfactn);
    static bool FReadActn(PCRF pcrf, CTG ctg, CNO cno, PBLCK pblck, PBACO *ppbaco, int32_t *pcb);
    ~ACTN(void);

    uint32_t Grfactn(void)
    {
        return _grfactn;
    }

    int32_t Ccel(void)
    {
        return _pggcel->IvMac();
    }
    void GetCel(int32_t icel, CEL *pcel);
    void GetCps(int32_t icel, int32_t icps, CPS *pcps);
    void GetMatrix(int32_t imat34, BMAT34 *pbmat34);
    void GetSnd(int32_t icel, PTAG ptagSnd);
};

// grftmpl flags
enum
{
    ftmplOnlyCustomCostumes = 1, // fTrue means don't apply generic MTRLs
    ftmplTdt = 2,                // fTrue means this is a 3-D Text object
    ftmplProp = 4,               // fTrue means this is a "prop" actor
};

/****************************************
    TMPL: The template class.
    anid is an action ID.
    cmid is a costume ID.
    celn is a cel number.
****************************************/
typedef class TMPL *PTMPL;
#define TMPL_PAR BACO
#define kclsTMPL KLCONST4('T', 'M', 'P', 'L')
class TMPL : public TMPL_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    BRA _xaRest; // Rest orientation
    BRA _yaRest;
    BRA _zaRest;
    uint32_t _grftmpl;
    PGL _pglibactPar; // GL of parent IDs (shorts) to build BODY
    PGL _pglibset;    // GL of body-part-set IDs to build BODY
    PGG _pggcmid;     // List of costumes for each body part set
    int32_t _ccmid;   // Count of custom costumes
    int32_t _cbset;   // Count of body part sets
    int32_t _cactn;   // Count of actions
    STN _stn;         // Template name

  protected:
    TMPL(void)
    {
    } // can't instantiate directly; must use FReadTmpl
    bool _FReadTmplf(PCFL pcfl, CTG ctg, CNO cno);
    virtual bool _FInit(PCFL pcfl, CTG ctgTmpl, CNO cnoTmpl);
    virtual PACTN _PactnFetch(int32_t anid);
    virtual PMODL _PmodlFetch(CHID chidModl);
    bool _FWriteTmplf(PCFL pcfl, CTG ctg, CNO *pcno);

  public:
    static bool FReadTmpl(PCRF pcrf, CTG ctg, CNO cno, PBLCK pblck, PBACO *ppbaco, int32_t *pcb);
    ~TMPL(void);
    static PGL PgltagFetch(PCFL pcfl, CTG ctg, CNO cno, bool *pfError);

    // TMPL / BODY stuff
    void GetName(PSTN pstn); // default name of actor or text of the TDT
    PBODY PbodyCreate(void); // Creates a body based on this TMPL
    void GetRestOrien(BRA *pxa, BRA *pya, BRA *pza);
    bool FIsTdt(void)
    {
        return FPure(_grftmpl & ftmplTdt);
    }
    bool FIsProp(void)
    {
        return FPure(_grftmpl & ftmplProp);
    }

    // Action stuff
    int32_t Cactn(void)
    {
        return _cactn;
    } // count of actions
    virtual bool FGetActnName(int32_t anid, PSTN pstn);
    bool FSetActnCel(PBODY pbody, int32_t anid, int32_t celn, BRS *pdwr = pvNil);
    bool FGetGrfactn(int32_t anid, uint32_t *pgrfactn);
    bool FGetDwrActnCel(int32_t anid, int32_t celn, BRS *pdwr);
    bool FGetCcelActn(int32_t anid, int32_t *pccel);
    bool FGetSndActnCel(int32_t anid, int32_t celn, bool *pfSoundExists, PTAG ptag);

    // Costume stuff
    virtual bool FSetDefaultCost(PBODY pbody); // applies default costume
    virtual PCMTL PcmtlFetch(int32_t cmid);
    int32_t CcmidOfBset(int32_t ibset);
    int32_t CmidOfBset(int32_t ibset, int32_t icmid);
    bool FBsetIsAccessory(int32_t ibset); // whether ibset holds accessories
    bool FIbsetAccOfIbset(int32_t ibset, int32_t *pibsetAcc);
    bool FSameAccCmids(int32_t cmid1, int32_t cmid2);
};

#endif // TMPL_H
