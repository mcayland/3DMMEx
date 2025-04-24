/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/*******************************************************************

 Actor class

 Primary Author : *****
 Review Status: Reviewed

 BASE -------> ACTR

 Each actor has a unique route which is defined by one or more nodes,
 and which may be a concatenation of one or more subroutes.
 Each actor also has an event list describing events which happen
 at a specified time and point along the route.  A point may coincide
 with a node or lie between nodes.

 *******************************************************************/
#ifndef ACTOR_H
#define ACTOR_H

//
//	XYZ : A point in x,y,z along an actor's RouTE.
//
struct XYZ
{
    BRS dxr;
    BRS dyr;
    BRS dzr;

    bool operator==(XYZ &xyz)
    {
        return ((dxr == xyz.dxr) && (dyr == xyz.dyr) && (dzr == xyz.dzr));
    }
    bool operator!=(XYZ &xyz)
    {
        return ((dxr != xyz.dxr) || (dyr != xyz.dyr) || (dzr != xyz.dzr));
    }
};
VERIFY_STRUCT_SIZE(XYZ, 12);
typedef XYZ *PXYZ;

const BOM kbomXyz = 0xfc000000;

//
// 	A RouTE is a general list (GL) of Route PoinTs(RPT)
//  A Subroute is a contiguous section of a route.
//
struct RPT
{
    XYZ xyz;
    BRS dwr; // Distance from this node to the next node on the route
};
VERIFY_STRUCT_SIZE(RPT, 16);
const BOM kbomRpt = 0xff000000;

const int32_t knfrmInvalid = klwMax;                                  // invalid frame state.  Regenerate correct state
const int32_t kcrptGrow = 32;                                         // quantum growth for rpt
const int32_t kcsmmGrow = 2;                                          // quantum growth for smm
const int32_t kctagSndGrow = 2;                                       // quantum growth for tagSnd
const int32_t smmNil = -1;                                            // Not motion match sound
const BRS kdwrNil = BR_SCALAR(-1.0);                                  // flags use of template cel stepsize
const BRS kzrDefault = BR_SCALAR(-25.0);                              // initial default z position
const BRS kdwrMax = BR_SCALAR(32767.0);                               // large BRS value
const uint32_t kdtsThreshRte = (kdtsSecond >> 2) + (kdtsSecond >> 1); // time threshhold before record in place
const BRS kdwrThreshRte = BR_SCALAR(2.0);                             // distance threshhold before entering record mode
const int32_t kcaevInit = 10;
const BRS kdwrFast = BR_SCALAR(3.0);       // delta world coord change for fast mouse move
const BRS krOriWeightMin = BR_SCALAR(0.1); // orientation weighting for slow mouse move
const BRS krAngleMin = BR_SCALAR(0.1);     // Min angle impacting amount of forward movement on placement
const BRS krAngleMinRcp = BR_RCP(krAngleMin);
const BRS krAngleMax = BR_SCALAR(0.4);         // Max angle impacting amount of forward movement on placement
const BRS krScaleMinFactor = (BR_SCALAR(0.1)); // Min scaling between Brender update
const auto krScaleMaxFactor = BrsDiv(rOne, krScaleMinFactor); // Max scaling between Brender update
const auto krScaleMin = (BR_SCALAR(0.25));                    // Min scaling
const auto krScaleMax = (BR_SCALAR(10.0));                    // Max scaling
const auto krPullMin = krScaleMin;
const auto krPullMax = BrsDiv(rOne, krPullMin);

#define aridNil ivNil

// Angle valid flags
enum
{
    fbraRotateX = factnRotateX,
    fbraRotateY = factnRotateY,
    fbraRotateZ = factnRotateZ
};

// Normalize flags
enum
{
    fnormSize = 1,
    fnormRotate = 2
};

// Mouse Actor flags
enum
{
    fmafNil = 0,
    fmafFreeze = 0x01,      // freeze cels
    fmafGround = 0x02,      // respect ground level
    fmafOrient = 0x04,      // orient actor during move
    fmafEntireScene = 0x08, // position over entire scene, vs subroute only
    fmafEntireSubrte = 0x10 // position over entire subroute
};

struct RTEL // RouTE Location - a function of space and time
{
    int irpt;      // The preceding node for the given point
    BRS dwrOffset; // Absolute linear distance beyond node irpt
    int32_t dnfrm; // Delta frame number (ie, time) at this point

    bool operator==(RTEL &rtel)
    {
        return (irpt == rtel.irpt && dwrOffset == rtel.dwrOffset && dnfrm == rtel.dnfrm);
    }

    bool operator!=(RTEL &rtel)
    {
        return (irpt != rtel.irpt || dwrOffset != rtel.dwrOffset || dnfrm != rtel.dnfrm);
    }

    bool operator<=(RTEL &rtel)
    {
        return (irpt < rtel.irpt || (irpt == rtel.irpt && (dwrOffset < rtel.dwrOffset ||
                                                           (dwrOffset == rtel.dwrOffset && dnfrm <= rtel.dnfrm))));
    }

    bool operator>=(RTEL &rtel)
    {
        return (irpt > rtel.irpt || (irpt == rtel.irpt && (dwrOffset > rtel.dwrOffset ||
                                                           (dwrOffset == rtel.dwrOffset && dnfrm >= rtel.dnfrm))));
    }

    bool operator<(RTEL &rtel)
    {
        return (irpt < rtel.irpt || (irpt == rtel.irpt && (dwrOffset < rtel.dwrOffset ||
                                                           (dwrOffset == rtel.dwrOffset && dnfrm < rtel.dnfrm))));
    }

    bool operator>(RTEL &rtel)
    {
        return (irpt > rtel.irpt || (irpt == rtel.irpt && (dwrOffset > rtel.dwrOffset ||
                                                           (dwrOffset == rtel.dwrOffset && dnfrm > rtel.dnfrm))));
    }
};

// Actor EVents are stored in a GG (general group)
// Fixed part of the GG:
struct AEV
{
    int32_t aet;  // Actor Event Type
    int32_t nfrm; // Absolute frame number (* Only valid < current event)
    RTEL rtel;    // RouTE Location for this event
};                // Additional event parameters (in the GG)
VERIFY_STRUCT_SIZE(AEV, 20);
typedef AEV *PAEV;

//
//	Actor level Event Types which live in a GG.
//	The fixed part of a GG entry is an actor event (aev)
//	The variable part is documented in the following comments
//
enum AET
{
    aetAdd,    // Add Actor onstage: aevadd - internal (no api)
    aetActn,   // Animate Actor : aevactn
    aetCost,   // Set Costume : aevcost
    aetRotF,   // Transform Actor rotate: BMAT34
    aetPull,   // Transform Actor Pull : aevpull
    aetSize,   // Transform Actor size uniformly : BRS
    aetSnd,    // Play a sound : aevsnd
    aetMove,   // Translate the path at this point : XYZ
    aetFreeze, // Freeze (or Un) Actor : long
    aetTweak,  // Path tweak : XYZ
    aetStep,   // Force step size (eg, float, wait) : BRS
    aetRem,    // Remove an actor from the stage : nil
    aetRotH,   // Single frame rotation : BMAT34
    aetLim
};
VERIFY_STRUCT_SIZE(AET, 4);

const BOM kbomAet = 0xc0000000;
const BOM kbomAev = 0xff000000;

//
//	Variable part of the Actor EVent GG:
//
struct AEVPULL // Squash/stretch
{
    BRS rScaleX;
    BRS rScaleY;
    BRS rScaleZ;
};
VERIFY_STRUCT_SIZE(AEVPULL, 12);
const BOM kbomAevpull = 0xfc000000;

// Every subroute is normalized.  The normalization translation is
// stored in the Add Event
// ** nfrmPrev valid for nfrmSub <= _nfrmCur only (optimization)
struct AEVADD
{
    BRS dxr; // Translation in x for this subroute
    BRS dyr; // Translation in y for this subroute
    BRS dzr; // Translation in z for this subroute
    BRA xa;  // Single point orientation
    BRA ya;  // Single point orientation
    BRA za;  // Single point orientation
};
VERIFY_STRUCT_SIZE(AEVADD, 20);
const BOM kbomAevadd = 0xffc00000 | kbomBmat34 >> 10;

struct AEVACTN
{
    int32_t anid;
    int32_t celn; // starting cel of action
};
VERIFY_STRUCT_SIZE(AEVACTN, 8);
const BOM kbomAevactn = 0xf0000000;

// On-disk representation of AEVCOST
struct AEVCOSTF
{
    int32_t ibset; // body part set
    int32_t cmid;  // costume ID (for custom costumes)
    tribool fCmtl; // vs fMtrl
    TAGF tag;
};
VERIFY_STRUCT_SIZE(AEVCOSTF, 28);

struct AEVCOST
{
    int32_t ibset; // body part set
    int32_t cmid;  // costume ID (for custom costumes)
    tribool fCmtl; // vs fMtrl
    TAG tag;
};
const BOM kbomAevcost = 0xfc000000 | (kbomTag >> 6);

// On-disk representation of AEVSND
struct AEVSNDF
{
    tribool fLoop;    // loop count
    tribool fQueue;   // queued sound
    int32_t vlm;      // volume
    int32_t celn;     // motion match	: ivNil if not
    int32_t sty;      // sound type
    tribool fNoSound; // no sound
    CHID chid;        // user sound requires chid
    TAGF tag;
};
VERIFY_STRUCT_SIZE(AEVSNDF, 44);

struct AEVSND
{
    tribool fLoop;    // loop count
    tribool fQueue;   // queued sound
    int32_t vlm;      // volume
    int32_t celn;     // motion match	: ivNil if not
    int32_t sty;      // sound type
    tribool fNoSound; // no sound
    CHID chid;        // user sound requires chid
    TAG tag;
};
const BOM kbomAevsnd = 0xfff00000 | (kbomTag >> 12);

const BOM kbomAevsize = 0xc0000000;
const BOM kbomAevfreeze = 0xc0000000;
const BOM kbomAevstep = 0xc0000000;
const BOM kbomAevmove = kbomXyz;
const BOM kbomAevtweak = kbomXyz;
const BOM kbomAevrot = kbomBmat34;

// Separate ggaev variable portion sizes
#define kcbVarAdd (SIZEOF(AEVADD))
#define kcbVarActn (SIZEOF(AEVACTN))
#define kcbVarCost (SIZEOF(AEVCOST))
#define kcbVarRot (SIZEOF(BMAT34))
#define kcbVarSize (SIZEOF(BRS))
#define kcbVarPull (SIZEOF(AEVPULL))
#define kcbVarSnd (SIZEOF(AEVSND))
#define kcbVarFreeze (SIZEOF(int32_t))
#define kcbVarMove (SIZEOF(XYZ))
#define kcbVarTweak (SIZEOF(XYZ))
#define kcbVarStep (SIZEOF(BRS))
#define kcbVarZero (0)

// Actor Event Flags
enum
{
    faetNil = 0,
    faetAdd = 1 << aetAdd,
    faetActn = 1 << aetActn,
    faetCost = 1 << aetCost,
    faetRotF = 1 << aetRotF,
    faetPull = 1 << aetPull,
    faetSize = 1 << aetSize,
    faetFreeze = 1 << aetFreeze,
    faetTweak = 1 << aetTweak,
    faetStep = 1 << aetStep,
    faetRem = 1 << aetRem,
    faetMove = 1 << aetMove,
    faetRotH = 1 << aetRotH
};

//
// Because rotations are non-abelian, cumulative rotations are stored
// in a BMAT34.  It would not be correct to multiply (xa,ya,za) values
// treating x,y,z independently.
//
// Bmat34Fwd does NOT include the path portion of the orientation.
// Bmat34Cur includes all current angular rotation - path rotation is NOT post
// applied to this.
//
// Careful: The task is complicated by the fact that users can apply single
// frame and frame forward orientations in the same frame and must not see the
// actor jump in angle when choosing between the two methods of editing.
//
struct XFRM
{
    BMAT34 bmat34Fwd; // Rotation	fwd	: path rotation post applied to this
    AEVPULL aevpull;  // Stretching (pulling) constants
    BRS rScaleStep;   // Uniform scaling to be applied to step size
    BMAT34 bmat34Cur; // <<Path independent>> Single frame & static segment rotation
    BRA xaPath;       // Path portion of the current frame's rotation
    BRA yaPath;       // Path portion of the current frame's rotation
    BRA zaPath;       // Path portion of the current frame's rotation
};

//
// Current action Motion Match Sounds
// Aev & Aevsnd grouped together form a gl
// Not to be saved with a movie
//
struct SMM
{
    AEV aev; // event for the sound
    AEVSND aevsnd;
};

//
// Default hilite colors
//
#define kiclrNormalHilite 108
#define kiclrTimeFreezeHilite 43

/***********************************************
   Actor Class
***********************************************/
typedef class ACTR *PACTR;
#define ACTR_PAR BASE
#define kclsACTR KLCONST4('A', 'C', 'T', 'R')
class ACTR : public ACTR_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    // Components of an actor
    // Note: In addition to these components, any complete actor must
    // have either fLifeDirty set or _nfrmLast current.
    // Note: _tagTmpl cannot be derived from _ptmpl
    PGG _pggaev;        // GG pointer to Actor EVent list
    PGL _pglrpt;        // GL pointer to actor's route
    TMPL *_ptmpl;       // Actor body & action list template
    BODY *_pbody;       // Actor's body
    TAG _tagTmpl;       // Note: The sid cannot be queried at save time
    TAG _tagSnd;        // Sound (played on entrance)
    SCEN *_pscen;       // Underlying scene
    XYZ _dxyzFullRte;   // Origin of the route
    int32_t _nfrmFirst; // klwMax -or- First frame : Set	when event created
    int32_t _arid;      // Unique id assigned to this actor.
    uint32_t _grfactn;  // Cached current grfactn

    // Frame Dependent State Information
    XYZ _dxyzRte;            //_dxyzFullRte + _dxyzSubRte : Set when Add processed
    XYZ _dxyzSubRte;         // Subpath translation : Set when Add processed
    bool _fOnStage : 1;      // Versus Brender hidden.  Set by Add, Rem only
    bool _fFrozen : 1;       // Path offset > 0 but not moving
    bool _fLifeDirty : 1;    // Set if _nfrmLast requires recomputation
    bool _fPrerendered : 1;  // Set if actor is prerendered
    bool _fUseBmat34Cur : 1; //_xfrm.bmat34Cur last used in orienting actor
    BRS _dwrStep;            // Current step size in use
    int32_t _anidCur;        // Current action in template
    int32_t _ccelCur;        // Cached cel count.  Retrieving this was hi profile.
    int32_t _celnCur;        // Current cell
    int32_t _nfrmCur;        // Current frame number : Set by FGotoFrame
    int32_t _nfrmLast;       // Latest known frame for this actor.
    int32_t _iaevCur;        // Current event in event list
    int32_t _iaevFrmMin;     // First event in current frame
    int32_t _iaevActnCur;    // Event defining current action
    int32_t _iaevAddCur;     // Most recent add (useful for Compose)
    RTEL _rtelCur;           // Current location on route	(excludes tweak info)
    XYZ _xyzCur;             // Last point displayed (may be tweak modified)
    XFRM _xfrm;              // Current transformation
    PGL _pglsmm;             // Current action motion match sounds

    // Path Recording State Information
    RTEL _rtelInsert;        // Joining information
    uint32_t _tsInsert;      // Starting time of route recording
    bool _fModeRecord : 1;   // Record a route mode
    bool _fRejoin : 1;       // Rerecord is extending a subpath from the end
    bool _fPathInserted : 1; // More path inserted
    bool _fTimeFrozen : 1;   // Is the actor frozen wrt time?
    int32_t _dnfrmGap;       // Frames between subroutes
    XYZ _dxyzRaw;            // Raw mouse movement from previous frame

    //
    //	Protected functions
    //
    ACTR(void);
    bool _FInit(TAG *ptmplTag);         // Constructor allocation & file I/O
    void _InitXfrmRot(BMAT34 *pbmat34); // Initialize rotation only
    void _InitXfrm(void);               // Initialize rotation & scaling
    bool _FCreateGroups(void);
    void _InitState(void);
    void _GetNewOrigin(BRS *pxr, BRS *pyr, BRS *pzr);
    void _SetStateRewound(void);

    bool _FQuickBackupToFrm(int32_t nfrm, bool *pfQuickMethodValid);
    bool _FGetRtelBack(RTEL *prtel, bool fUpdateStateVar);
    bool _FDoFrm(bool fPositionBody, bool *pfPositionDirty, bool *pfSoundInFrame = pvNil);
    bool _FGetStatic(int32_t anid, bool *pfStatic);
    bool _FIsDoneAevSub(int32_t iaev, RTEL rtel);
    bool _FIsAddNow(int32_t iaev);
    bool _FGetDwrPlay(BRS *pdwr);   // Step size if playing
    bool _FGetDwrRecord(BRS *pdwr); // Step size if recording
    bool _FDoAevCur(void);
    bool _FDoAevCore(int32_t iaev);
    bool _FDoAetVar(int32_t aet, void *pvVar, int32_t cbVar);

    bool _FEnqueueSnd(int32_t iaev);
    bool _FEnqueueSmmInMsq(void);
    bool _FInsertSmm(int32_t iaev);
    bool _FRemoveAevMm(int32_t anid);
    bool _FAddAevDefMm(int32_t anid);

    bool _FAddDoAev(int32_t aetNew, int32_t kcbNew, void *pvVar);
    void _MergeAev(int32_t iaevFirst, int32_t iaevLast, int32_t *piaevNew = pvNil);
    bool _FFreeze(void);   // insert freeze event
    bool _FUnfreeze(void); // insert unfreeze event
    void _Hide(void);
    bool _FInsertGgRpt(int32_t irpt, RPT *prpt, BRS dwrPrior = rZero);
    bool _FAddAevFromPrev(int32_t iaevLim, uint32_t grfaet);
    bool _FAddAevFromLater(void);
    bool _FFindNextAevAet(int32_t aet, int32_t iaevCur, int32_t *piaevAdd);
    bool _FFindPrevAevAet(int32_t aet, int32_t iaevCur, int32_t *piaevAdd);
    void _FindAevLastSub(int32_t iaevAdd, int32_t iaevLim, int32_t *piaevLast);
    void _DeleteFwdCore(bool fDeleteAll, bool *pfAlive = pvNil, int32_t iaevCur = ivNil);
    bool _FDeleteEntireSubrte(void);
    void _DelAddFrame(int32_t iaevAdd, int32_t iaevLim);

    void _UpdateXyzRte(void);
    bool _FInsertAev(int32_t iaev, int32_t cbNew, void *pvVar, void *paev, bool fUpdateState = fTrue);
    void _RemoveAev(int32_t iaev, bool fUpdateState = fTrue);
    void _PrepXfrmFill(int32_t aet, void *pvVar, int32_t cbVar, int32_t iaevMin, int32_t iaevCmp = ivNil,
                       uint32_t grfaet = faetNil);
    void _PrepActnFill(int32_t iaevMin, int32_t anidPrev, int32_t anidNew, uint32_t grfaet);
    void _PrepCostFill(int32_t iaevMin, AEVCOST *paevcost);
    void _AdjustAevForRteIns(int32_t irptAdjust, int32_t iaevMin);
    void _AdjustAevForRteDel(int32_t irptAdjust, int32_t iaevMin);
    bool _FInsertStop(void);
    void _CalcRteOrient(BMAT34 *pbmat34, BRA *pxa = pvNil, BRA *pya = pvNil, BRA *pza = pvNil,
                        uint32_t *pgrfbra = pvNil);
    void _ApplyRotFromVec(XYZ *pxyz, BMAT34 *pbmat34, BRA *pxa = pvNil, BRA *pya = pvNil, BRA *pza = pvNil,
                          uint32_t *grfbra = pvNil);
    void _SaveCurPathOrien(void);
    void _LoadAddOrien(AEVADD *paevadd, bool fNoReset = fFalse);
    BRA _BraAvgAngle(BRA a1, BRA a2, BRS rw);
    void _UpdateXyzTan(XYZ *pxyz, int32_t irptTan, int32_t rw);

    void _AdvanceRtel(BRS dwrStep, RTEL *prtel, int32_t iaevCur, int32_t nfrmCur, bool *pfEndRoute);
    void _GetXyzFromRtel(RTEL *prtel, PXYZ pxyz);
    void _GetXyzOnLine(PXYZ pxyzFirst, PXYZ pxyzSecond, BRS dwrOffset, PXYZ pxyz);
    void _PositionBody(PXYZ pxyz);
    void _MatrixRotUpdate(XYZ *pxyz, BMAT34 *pbmat34);
    void _TruncateSubRte(int32_t irptDelLim);
    bool _FComputeLifetime(int32_t *pnfrmLast = pvNil);
    bool _FIsStalled(int32_t iaevFirst, RTEL *prtel, int32_t *piaevLast = pvNil);

    void _RestoreFromUndo(PACTR pactrRestore);
    bool _FDupCopy(PACTR pactrSrc, PACTR pactrDest);

    bool _FWriteTmpl(PCFL pcfl, CNO cno);
    bool _FReadActor(PCFL pcfl, CNO cno);
    bool _FReadRoute(PCFL pcfl, CNO cno);
    bool _FReadEvents(PCFL pcfl, CNO cno);
    bool _FOpenTags(PCRF pcrf);
    static bool _FIsIaevTag(PGG pggaev, int32_t iaev, PTAG *pptag, PAEV *pqaev = pvNil);
    void _CloseTags(void);

  public:
    ~ACTR(void);
    static PACTR PactrNew(TAG *ptagTmpl);
    void SetPscen(SCEN *pscen);
    void SetArid(int32_t arid)
    {
        AssertBaseThis(0);
        _arid = arid;
    }
    void SetLifeDirty(void)
    {
        AssertBaseThis(0);
        _fLifeDirty = fTrue;
    }
    PSCEN Pscen(void)
    {
        AssertThis(0);
        return _pscen;
    }

    // ActrSave Routines
    static PACTR PactrRead(PCRF pcrf, CNO cno);    // Construct from a document
    bool FWrite(PCFL pcfl, CNO cno, CNO cnoScene); // Write to a document
    static PGL PgltagFetch(PCFL pcfl, CNO cno, bool *pfError);
    static bool FAdjustAridOnFile(PCFL pcfl, CNO cno, int32_t darid);

    // Visibility
    void Hilite(void);
    void Unhilite(void)
    {
        AssertBaseThis(0);
        _pbody->Unhilite();
    }
    void Hide(void)
    {
        AssertBaseThis(0);
        _pbody->Hide();
    }
    void Show(void)
    {
        AssertBaseThis(0);
        _pbody->Show();
    }
    bool FIsInView(void)
    {
        AssertBaseThis(0);
        return _pbody->FIsInView();
    }
    void GetCenter(int32_t *pxp, int32_t *pyp)
    {
        AssertBaseThis(0);
        _pbody->GetCenter(pxp, pyp);
    }
    void GetRcBounds(RC *prc)
    {
        AssertBaseThis(0);
        _pbody->GetRcBounds(prc);
    }
    void SetPrerendered(bool fPrerendered)
    {
        AssertBaseThis(0);
        _fPrerendered = fPrerendered;
    }
    bool FPrerendered(void)
    {
        AssertBaseThis(0);
        return _fPrerendered;
    }

    // Actor Information
    int32_t Arid(void)
    {
        AssertBaseThis(0);
        return (_arid);
    }
    bool FOnStage(void)
    {
        AssertBaseThis(0);
        return (_fOnStage);
    }
    bool FIsMyBody(BODY *pbody)
    {
        AssertBaseThis(0);
        return pbody == _pbody;
    }
    bool FIsMyTmpl(TMPL *ptmpl)
    {
        AssertBaseThis(0);
        return _ptmpl == ptmpl;
    }
    bool FIsModeRecord(void)
    {
        AssertBaseThis(0);
        return FPure(_fModeRecord);
    }
    bool FIsRecordValid(BRS dxr, BRS dyr, BRS dzr, uint32_t tsCurrent);
    PTMPL Ptmpl(void)
    {
        AssertBaseThis(0);
        return _ptmpl;
    }
    PBODY Pbody(void)
    {
        AssertBaseThis(0);
        return _pbody;
    }
    int32_t AnidCur(void)
    {
        AssertBaseThis(0);
        return _anidCur;
    }
    int32_t CelnCur(void)
    {
        AssertBaseThis(0);
        return _celnCur;
    }
    bool FFrozen(void)
    {
        AssertBaseThis(0);
        return _fFrozen;
    }
    bool FGetLifetime(int32_t *pnfrmFirst, int32_t *pnfrmLast); // allows nil ptrs
    bool FPtIn(int32_t xp, int32_t yp, int32_t *pibset);
    void GetTagTmpl(PTAG ptag)
    {
        AssertBaseThis(0);
        *ptag = _tagTmpl;
    }
    void GetName(PSTN pstn);
    bool FChangeTagTmpl(PTAG ptagTmplNew);
    bool FTimeFrozen(void)
    {
        AssertBaseThis(0);
        return _fTimeFrozen;
    }
    void SetTimeFreeze(bool fTimeFrozen)
    {
        AssertBaseThis(0);
        _fTimeFrozen = fTimeFrozen;
    }
    bool FIsPropBrws(void)
    {
        AssertThis(0);
        return FPure(_ptmpl->FIsProp() || _ptmpl->FIsTdt());
    }
    bool FIsTdt(void)
    {
        AssertThis(0);
        return FPure(_ptmpl->FIsTdt());
    }
    bool FMustRender(int32_t nfrmRenderLast);
    void GetXyzWorld(BRS *pxr, BRS *pyr, BRS *pzr);

    // Animation
    bool FGotoFrame(int32_t nfrm, bool *pfSoundInFrame = pvNil); // Prepare for display at frame nfrm
    bool FReplayFrame(int32_t grfscen);                          // Replay a frame.

    // Event Editing
    bool FAddOnStageCore(void);
    bool FSetActionCore(int32_t anid, int32_t celn, bool fFreeze);
    bool FRemFromStageCore(void);
    bool FSetCostumeCore(int32_t ibsetClicked, TAG *ptag, int32_t cmid, tribool fCustom);
    bool FSetStep(BRS dwrStep);
    bool FRotate(BRA xa, BRA ya, BRA za, bool fFromHereFwd);
    bool FNormalizeCore(uint32_t grfnorm);
    void SetAddOrient(BRA xa, BRA ya, BRA za, uint32_t grfbra, XYZ *pdxyz = pvNil);
    bool FScale(BRS rScaleStep);
    bool FPull(BRS rScaleX, BRS rScaleY, BRS rScaleZ);
    void DeleteFwdCore(bool fDeleteAll, bool *pfAlive = pvNil, int32_t iaevCur = ivNil);
    void DeleteBackCore(bool *pfAlive = pvNil);
    bool FSoonerLater(int32_t dnfrm);

    // ActrEdit Routines
    bool FDup(PACTR *ppactr, bool fReset = fFalse); // Duplicate everything
    void Restore(PACTR pactr);

    bool FCreateUndo(PACTR pactr, bool fSndUndo = fFalse, PSTN pstn = pvNil); // Create undo object
    void Reset(void);
    bool FAddOnStage(void); // add actor to the stage, w/Undo
    bool FSetAction(int32_t anid, int32_t celn, bool fFreeze, PACTR *ppactrDup = pvNil);
    bool FSetCostume(int32_t ibset, TAG *ptag, int32_t cmid, tribool fCustom);
    bool FRemFromStage(void);                                 // add event: rem actor from stage, w/Undo
    bool FCopy(PACTR *ppactr, bool fEntireScene = fFalse);    // Duplicate actor from this frame on
    bool FCopyRte(PACTR *ppactr, bool fEntireScene = fFalse); // Duplicate path from this frame on
    bool FPasteRte(PACTR pactr);                              // Paste from clipboard from this frame on
    bool FNormalize(uint32_t grfnorm);
    bool FPaste(int32_t nfrm, SCEN *pscen);
    bool FDelete(bool *pfAlive, bool fDeleteAll);

    // ActrSnd Routines
    bool FSetSnd(PTAG ptag, tribool fLoop, tribool fQueue, tribool fActnCel, int32_t vlm, int32_t sty);
    bool FSetSndCore(PTAG ptag, tribool fLoop, tribool fQueue, tribool fActnCel, int32_t vlm, int32_t sty);
    bool FSetVlmSnd(int32_t sty, bool fMotionMatch, int32_t vlm); // Set the volume of a sound
    bool FQuerySnd(int32_t sty, bool fMotionMatch, PGL *pglTagSnd, int32_t *pvlm, bool *pfLoop);
    bool FDeleteSndCore(int32_t sty, bool fMotionMatch);
    bool FSoundInFrm(void);
    bool FResolveAllSndTags(CNO cnoScen);

    // Route Definition
    void SetTsInsert(uint32_t tsCurrent)
    {
        AssertBaseThis(0);
        _tsInsert = tsCurrent;
    }
    bool FBeginRecord(uint32_t tsCurrent, bool fReplace, PACTR pactrRestore);
    bool FRecordMove(BRS dxr, BRS dyr, BRS dzr, uint32_t grfmaf, uint32_t tsCurrent, bool *pfLonger, bool *pfStep,
                     PACTR pactrRestore);
    bool FEndRecord(bool fReplace, PACTR pactrRestore);
    bool FTweakRoute(BRS dxr, BRS dyr, BRS dzr, uint32_t grfmaf = fmafNil);
    bool FMoveRoute(BRS dxr, BRS dyr, BRS dzr, bool *pfMoved = pvNil, uint32_t grfmaf = fmafNil);
};

//
// Actor document for clipping
//
typedef class ACLP *PACLP;
#define ACLP_PAR DOCB
#define kclsACLP KLCONST4('A', 'C', 'L', 'P')
class ACLP : public ACLP_PAR
{
    RTCLASS_DEC
    MARKMEM
    ASSERT

  protected:
    PACTR _pactr;
    bool _fRteOnly;
    STN _stnName;

    ACLP(void)
    {
    }

  public:
    //
    // Constructors and destructors
    //
    static PACLP PaclpNew(PACTR pactr, bool fRteOnly, bool fEndScene = fFalse);
    ~ACLP(void);

    //
    // Pasting function
    //
    bool FPaste(PMVIE pmvie);

    bool FRouteOnly(void)
    {
        return _fRteOnly;
    }
};

#endif //! ACTOR_H
