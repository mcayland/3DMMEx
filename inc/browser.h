/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/**************************************************************

   Browser Class

    Author : *****
    Review Status: Reviewed

    Studio Independent Browsers:
    BASE --> CMH --> GOK	-->	BRWD  (Browser display class)
    BRWD --> BRWL  (Browser list class; chunky based)
    BRWD --> BRWT  (Browser text class)
    BRWD --> BRWL --> BRWN  (Browser named list class)

    Studio Dependent Browsers:
    BRWD --> BRWR  (Roll call class)
    BRWD --> BRWT --> BRWA  (Browser action class)
    BRWD --> BRWL --> BRWP	(Browser prop/actor class)
    BRWD --> BRWL --> BRWB	(Browser background class)
    BRWD --> BRWL --> BRWC	(Browser camera class)
    BRWD --> BRWL --> BRWN --> BRWM (Browser music class)
    BRWD --> BRWL --> BRWN --> BRWM --> BRWI (Browser import sound class)

    Note: An "frm" refers to the displayed frames on any page.
    A "thum" is a generic Browser Thumbnail, which may be a
    chid, cno, cnoPar, gob, stn, etc.	A browser will display,
    over multiple pages, as many browser entities as there are
    thum's.

***************************************************************/

#ifndef BRWD_H
#define BRWD_H

const int32_t kcmhlBrowser = 0x11000; // nice medium level for the Browser
const int32_t kcbMaxCrm = 300000;
const int32_t kdwTotalPhysLim = 10240000; // 10MB	heuristic
const int32_t kdwAvailPhysLim = 1024000;  // 1MB heuristic
const auto kBrwsScript = (kstDefault << 16) | kchidBrowserDismiss;

/************************************

    Browser Context	CLass
    Optional context to carry over
    between successive instantiations
    of the same browser

*************************************/
#define BRCN_PAR BASE
#define kclsBRCN 'BRCN'
typedef class BRCN *PBRCN;
class BRCN : public BRCN_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    BRCN(void){};

  public:
    int32_t brwdid;
    int32_t ithumPageFirst;
};

/************************************

   Browser ThumbFile Cki Struct

*************************************/
struct TFC
{
    int16_t bo;
    int16_t osk;
    union {
        struct
        {
            CTG ctg;
            CNO cno;
        };
        struct
        {
            uint32_t grfontMask;
            uint32_t grfont;
        };
        struct
        {
            CTG _ctg;
            CHID chid;
        };
    };
};
VERIFY_STRUCT_SIZE(TFC, 12);
const BOM kbomTfc = 0x5f000000;

/************************************

   Browser Display Class

*************************************/
#define BRWD_PAR GOK
#define kclsBRWD 'BRWD'
#define brwdidNil ivNil
typedef class BRWD *PBRWD;
class BRWD : public BRWD_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM
    CMD_MAP_DEC(BRWD)

  protected:
    int32_t _kidFrmFirst;     // kid of first frame
    int32_t _kidControlFirst; // kid of first control button
    int32_t _dxpFrmOffset;    // x inset of thumb in frame
    int32_t _dypFrmOffset;    // y inset of thumb in frame
    int32_t _sidDefault;      // default sid
    int32_t _thumDefault;     // default thum
    PBRCN _pbrcn;             // context carryover
    int32_t _idsFont;         // string id of Font
    int32_t _kidThumOverride; // projects may override one thum gobid
    int32_t _ithumOverride;   // projects may override one thum gobid
    PTGOB _ptgobPage;         // for page numbers
    PSTDIO _pstdio;

    // Display State variables
    int32_t _cthumCD;        // Non-user content
    int32_t _ithumSelect;    // Hilited frame
    int32_t _ithumPageFirst; // Index to thd of first frame on current page
    int32_t _cfrmPageCur;    // Number of visible thumbnails per current page
    int32_t _cfrm;           // Total frames possible per page
    int32_t _cthumScroll;    // #items to scroll on fwd/back.  default ivNil -> page scrolling
    bool _fWrapScroll;       // Wrap around.  Default = fTrue;
    bool _fNoRepositionSel;  // Don't reposition selection : default = fFalse;

  protected:
    void _SetScrollState(void);
    int32_t _CfrmCalc(void);
    static bool _FBuildGcb(GCB *pgcb, int32_t kidPar, int32_t kidBrws);
    bool _FInitGok(PRCA prca, int32_t kidGlass);
    void _SetVarForOverride(void);

    virtual int32_t _Cthum(void)
    {
        AssertThis(0);
        return 0;
    }
    virtual bool _FSetThumFrame(int32_t ithum, PGOB pgobPar)
    {
        AssertThis(0);
        return fFalse;
    }
    virtual bool _FClearHelp(int32_t ifrm)
    {
        return fTrue;
    }
    virtual void _ReleaseThumFrame(int32_t ifrm)
    {
    }
    virtual int32_t _IthumFromThum(int32_t thum, int32_t sid)
    {
        return thum;
    }
    virtual void _GetThumFromIthum(int32_t ithum, void *pThumSelect, int32_t *psid);
    virtual void _ApplySelection(int32_t thumSelect, int32_t sid)
    {
    }
    virtual void _ProcessSelection(void)
    {
    }
    virtual bool _FUpdateLists()
    {
        return fTrue;
    }
    virtual void _SetCbPcrmMin(void)
    {
    }
    void _CalcIthumPageFirst(void);
    bool _FIsIthumOverride(int32_t ithum)
    {
        return FPure(ithum == _ithumOverride);
    }
    PGOB _PgobFromIfrm(int32_t ifrm);
    int32_t _KidThumFromIfrm(int32_t ifrm);
    void _UnhiliteCurFrm(void);
    bool _FHiliteFrm(int32_t ifrmSelect);
    void _InitStateVars(PCMD pcmd, PSTDIO pstdio, bool fWrapScroll, int32_t cthumScroll);
    void _InitFromData(PCMD pcmd, int32_t ithumSelect, int32_t ithumDisplay);
    virtual void _CacheContext(void);

  public:
    //
    // Constructors and destructors
    //
    BRWD(PGCB pgcb) : BRWD_PAR(pgcb)
    {
        _ithumOverride = -1;
        _kidThumOverride = -1;
    }
    ~BRWD(void);

    static PBRWD PbrwdNew(PRCA prca, int32_t kidPar, int32_t kidBrwd);
    void Init(PCMD pcmd, int32_t ithumSelect, int32_t ithumDisplay, PSTDIO pstdio, bool fWrapScroll = fTrue,
              int32_t cthumScroll = ivNil);
    bool FDraw(void);
    bool FCreateAllTgob(void); // For any text based browsers

    //
    // Command Handlers
    // Selection does not exit the browser
    //
    bool FCmdFwd(PCMD pcmd);  // Page fwd
    bool FCmdBack(PCMD pcmd); // Page back
    bool FCmdSelect(PCMD pcmd);
    bool FCmdSelectThum(PCMD pcmd); // Set viewing page
    virtual void Release(void);
    virtual bool FCmdCancel(PCMD pcmd); // See brwb
    virtual bool FCmdDel(PCMD pcmd)
    {
        return fTrue;
    } // See brwm
    virtual bool FCmdOk(PCMD pcmd);
    virtual bool FCmdFile(PCMD pcmd)
    {
        return fTrue;
    } // See brwm
    virtual bool FCmdChangeCel(PCMD pcmd)
    {
        return fTrue;
    } // See brwa
};

/************************************

    Browser List Context CLass
    Optional context to carry over
    between successive instantiations
    of the same browser

*************************************/
#define BRCNL_PAR BRCN
#define kclsBRCNL 'brcl'
typedef class BRCNL *PBRCNL;
class BRCNL : public BRCNL_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    ~BRCNL(void);

  public:
    int32_t cthumCD;
    CKI ckiRoot;
    PGL pglthd;
    PGST pgst;
    PCRM pcrm;
};

//
//	Thumbnail descriptors : one per thumbnail
//
const int32_t kglstnGrow = 5;
const int32_t kglthdGrow = 10;
struct THD
{
    union {
        TAG tag; // TAG pointing to content
        struct
        {
            int32_t lwFill1; // sid
            int32_t lwFill2; // pcrf
            uint32_t grfontMask;
            uint32_t grfont;
        };
        struct
        {
            int32_t _lwFill1;
            int32_t _lwFill2;
            CTG ctg;
            CHID chid; // CHID of CD content
        };
    };

    CNO cno;       // GOKD cno
    CHID chidThum; // GOKD's parent's CHID (relative to GOKD parent's parent)
    int32_t ithd;  // Original index for this THD, before sorting (used to
                   // retrieve proper STN for the BRWN-derived browsers)
};

/* Browser Content List Base --  create one of these when you want a list of a
    specific kind of content and you don't care about the names. */
#define BCL_PAR BASE
typedef class BCL *PBCL;
#define kclsBCL 'BCL'
class BCL : public BCL_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    CTG _ctgRoot;
    CNO _cnoRoot;
    CTG _ctgContent;
    bool _fDescend;
    PGL _pglthd;

  protected:
    BCL(void)
    {
        _pglthd = pvNil;
    }
    ~BCL(void)
    {
        ReleasePpo(&_pglthd);
    }

    bool _FInit(PCRM pcrm, CKI *pckiRoot, CTG ctgContent, PGL pglthd);
    bool _FAddGokdToThd(PCFL pcfl, int32_t sid, CKI *pcki);
    bool _FAddFileToThd(PCFL pcfl, int32_t sid);
    bool _FBuildThd(PCRM pcrm);

    virtual bool _FAddGokdToThd(PCFL pcfl, int32_t sid, KID *pkid);

  public:
    static PBCL PbclNew(PCRM pcrm, CKI *pckiRoot, CTG ctgContent, PGL pglthd = pvNil, bool fOnlineOnly = fFalse);

    PGL Pglthd(void)
    {
        return _pglthd;
    }
    void GetThd(int32_t ithd, THD *pthd)
    {
        _pglthd->Get(ithd, pthd);
    }
    int32_t IthdMac(void)
    {
        return _pglthd->IvMac();
    }
};

/* Browser Content List with Strings -- create one of these when you need to
    browse content by name */
#define BCLS_PAR BCL
typedef class BCLS *PBCLS;
#define kclsBCLS 'BCLS'
class BCLS : public BCLS_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PGST _pgst;

  protected:
    BCLS(void)
    {
        _pgst = pvNil;
    }
    ~BCLS(void)
    {
        ReleasePpo(&_pgst);
    }

    bool _FInit(PCRM pcrm, CKI *pckiRoot, CTG ctgContent, PGST pgst, PGL pglthd);
    bool _FSetNameGst(PCFL pcfl, CTG ctg, CNO cno);

    virtual bool _FAddGokdToThd(PCFL pcfl, int32_t sid, KID *pkid);

  public:
    static PBCLS PbclsNew(PCRM pcrm, CKI *pckiRoot, CTG ctgContent, PGL pglthd = pvNil, PGST pgst = pvNil,
                          bool fOnlineOnly = fFalse);

    PGST Pgst(void)
    {
        return _pgst;
    }
};

/************************************

   Browser List Class
   Derived from the Display Class

*************************************/
#define BRWL_PAR BRWD
#define kclsBRWL 'BRWL'
typedef class BRWL *PBRWL;

// Browser Selection Flags
// This specifies what the sorting is based on
enum BWS
{
    kbwsIndex = 1,
    kbwsChid = 2,
    kbwsCnoRoot = 3, // defaults to CnoRoot if ctg of Par is ctgNil
    kbwsLim
};

class BRWL : public BRWL_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    bool _fEnableAccel;

    // Thumnail descriptor lists
    PCRM _pcrm;  // Chunky resource manager
    PGL _pglthd; // Thumbnail descriptor	gl
    PGST _pgst;  // Chunk name

    // Browser Search (List) parameters
    BWS _bws;         // Selection type flag
    bool _fSinglePar; // Single parent search
    CKI _ckiRoot;     // Grandparent cno=cnoNil => global search
    CTG _ctgContent;  // Parent

  protected:
    // BRWL List
    bool _FInitNew(PCMD pcmd, BWS bws, int32_t ThumSelect, CKI ckiRoot, CTG ctgContent);
    bool _FCreateBuildThd(CKI ckiRoot, CTG ctgContent, bool fBuildGl = fTrue);
    virtual bool _FGetContent(PCRM pcrm, CKI *pcki, CTG ctg, bool fBuildGl);
    virtual int32_t _Cthum(void)
    {
        AssertThis(0);
        return _pglthd->IvMac();
    }
    virtual bool _FSetThumFrame(int32_t ithd, PGOB pgobPar);
    virtual bool _FUpdateLists()
    {
        return fTrue;
    } // Eg, to include user sounds

    // BRWL util
    void _SortThd(void);
    virtual void _GetThumFromIthum(int32_t ithum, void *pThumSelect, int32_t *psid);
    virtual void _ReleaseThumFrame(int32_t ifrm);
    virtual int32_t _IthumFromThum(int32_t thum, int32_t sid);
    virtual void _CacheContext(void);
    virtual void _SetCbPcrmMin(void);

  public:
    //
    // Constructors and destructors
    //
    BRWL(PGCB pgcb) : BRWL_PAR(pgcb)
    {
    }
    ~BRWL(void);

    static PBRWL PbrwlNew(PRCA prca, int32_t kidPar, int32_t kidBrwl);
    virtual bool FInit(PCMD pcmd, BWS bws, int32_t ThumSelect, int32_t sidSelect, CKI ckiRoot, CTG ctgContent,
                       PSTDIO pstdio, PBRCNL pbrcnl = pvNil, bool fWrapScroll = fTrue, int32_t cthumScroll = ivNil);
};

/************************************

   Browser Text Class
   Derived from the Display Class

*************************************/
#define BRWT_PAR BRWD
#define kclsBRWT 'BRWT'
typedef class BRWT *PBRWT;
class BRWT : public BRWT_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PGST _pgst;
    bool _fEnableAccel;

    virtual int32_t _Cthum(void)
    {
        AssertThis(0);
        return _pgst->IvMac();
    }
    virtual bool _FSetThumFrame(int32_t istn, PGOB pgobPar);
    virtual void _ReleaseThumFrame(int32_t ifrm)
    {
    } // No gob to release

  public:
    //
    // Constructors and destructors
    //
    BRWT(PGCB pgcb) : BRWT_PAR(pgcb)
    {
        _idsFont = idsNil;
    }
    ~BRWT(void);

    static PBRWT PbrwtNew(PRCA prca, int32_t kidPar, int32_t kidBrwt);
    void SetGst(PGST pgst);
    bool FInit(PCMD pcmd, int32_t thumSelect, int32_t thumDisplay, PSTDIO pstdio, bool fWrapScroll = fTrue,
               int32_t cthumScroll = ivNil);
};

/************************************

   Browser Named List Class
   Derived from the Browser List Class

*************************************/
#define BRWN_PAR BRWL
#define kclsBRWN 'BRWN'
typedef class BRWN *PBRWN;
class BRWN : public BRWN_PAR
{
    RTCLASS_DEC

  protected:
    virtual bool _FGetContent(PCRM pcrm, CKI *pcki, CTG ctg, bool fBuildGl);
    virtual int32_t _Cthum(void)
    {
        return _pglthd->IvMac();
    }
    virtual bool _FSetThumFrame(int32_t ithd, PGOB pgobPar);
    virtual void _ReleaseThumFrame(int32_t ifrm);

  public:
    //
    // Constructors and destructors
    //
    BRWN(PGCB pgcb) : BRWN_PAR(pgcb)
    {
    }
    ~BRWN(void){};
    virtual bool FInit(PCMD pcmd, BWS bws, int32_t ThumSelect, int32_t sidSelect, CKI ckiRoot, CTG ctgContent,
                       PSTDIO pstdio, PBRCNL pbrcnl = pvNil, bool fWrapScroll = fTrue, int32_t cthumScroll = ivNil);

    virtual bool FCmdOk(PCMD pcmd);
};

/************************************

   Studio Specific Browser Classes

*************************************/
/************************************

   Browser Action Class
   Derived from the Browser Text Class
   Actions are separately classed for
   previews

*************************************/
#define BRWA_PAR BRWT
#define kclsBRWA 'BRWA'
typedef class BRWA *PBRWA;
class BRWA : public BRWA_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    int32_t _celnStart;           // Starting cel number
    PAPE _pape;                   // Actor Preview Entity
    void _ProcessSelection(void); // Action Preview
    virtual void _ApplySelection(int32_t thumSelect, int32_t sid);

  public:
    //
    // Constructors and destructors
    //
    BRWA(PGCB pgcb) : BRWA_PAR(pgcb)
    {
        _idsFont = idsActionFont;
        _celnStart = 0;
    }
    ~BRWA(void)
    {
    }

    static PBRWA PbrwaNew(PRCA prca);
    bool FBuildApe(PACTR pactr);
    bool FBuildGst(PSCEN pscen);
    virtual bool FCmdChangeCel(PCMD pcmd);
};

/************************************

   Browser Prop & Actor Class
   Derived from the Browser List Class

*************************************/
#define BRWP_PAR BRWL
#define kclsBRWP 'BRWP'
typedef class BRWP *PBRWP;
class BRWP : public BRWP_PAR
{
    RTCLASS_DEC

  protected:
    virtual void _ApplySelection(int32_t thumSelect, int32_t sid);

  public:
    //
    // Constructors and destructors
    //
    BRWP(PGCB pgcb) : BRWP_PAR(pgcb)
    {
    }
    ~BRWP(void){};

    static PBRWP PbrwpNew(PRCA prca, int32_t kidGlass);
};

/************************************

   Browser Background Class
   Derived from the Browser List Class

*************************************/
#define BRWB_PAR BRWL
#define kclsBRWB 'BRWB'
typedef class BRWB *PBRWB;
class BRWB : public BRWB_PAR
{
    RTCLASS_DEC

  protected:
    virtual void _ApplySelection(int32_t thumSelect, int32_t sid);

  public:
    //
    // Constructors and destructors
    //
    BRWB(PGCB pgcb) : BRWB_PAR(pgcb)
    {
    }
    ~BRWB(void){};

    static PBRWB PbrwbNew(PRCA prca);
    virtual bool FCmdCancel(PCMD pcmd);
};

/************************************

   Browser Camera Class
   Derived from the Browser List Class

*************************************/
#define BRWC_PAR BRWL
#define kclsBRWC 'BRWC'
typedef class BRWC *PBRWC;
class BRWC : public BRWC_PAR
{
    RTCLASS_DEC

  protected:
    virtual void _ApplySelection(int32_t thumSelect, int32_t sid);
    virtual void _SetCbPcrmMin(void)
    {
    }

  public:
    //
    // Constructors and destructors
    //
    BRWC(PGCB pgcb) : BRWC_PAR(pgcb)
    {
    }
    ~BRWC(void){};

    static PBRWC PbrwcNew(PRCA prca);

    virtual bool FCmdCancel(PCMD pcmd);
};

/************************************

   Browser Music Class (midi, speech & fx)
   Derived from the Browser Named List Class

*************************************/
#define BRWM_PAR BRWN
#define kclsBRWM 'brwm'
typedef class BRWM *PBRWM;
class BRWM : public BRWM_PAR
{
    RTCLASS_DEC

  protected:
    int32_t _sty; // Identifies type of sound
    PCRF _pcrf;   // NOT created here (autosave or BRWI file)

    virtual void _ApplySelection(int32_t thumSelect, int32_t sid);
    virtual bool _FUpdateLists(); // By all entries in pcrf of correct type
    void _ProcessSelection(void); // Sound Preview
    bool _FAddThd(STN *pstn, CKI *pcki);
    bool _FSndListed(CNO cno, int32_t *pithd = pvNil);

  public:
    //
    // Constructors and destructors
    //
    BRWM(PGCB pgcb) : BRWM_PAR(pgcb)
    {
        _idsFont = idsSoundFont;
    }
    ~BRWM(void){};

    static PBRWM PbrwmNew(PRCA prca, int32_t kidGlass, int32_t sty, PSTDIO pstdio);
    virtual bool FCmdFile(PCMD pcmd); // Upon portfolio completion
    virtual bool FCmdDel(PCMD pcmd);  // Delete user sound
};

/************************************

   Browser Import Sound Class
   Derived from the Browser List Class
   Note: Inherits pgst from the list class

*************************************/
#define BRWI_PAR BRWM
#define kclsBRWI 'BRWI'
typedef class BRWI *PBRWI;
class BRWI : public BRWI_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    // The following are already handled by BRWM
    // virtual void _ProcessSelection(void);
    virtual void _ApplySelection(int32_t thumSelect, int32_t sid);

  public:
    //
    // Constructors and destructors
    //
    BRWI(PGCB pgcb) : BRWI_PAR(pgcb)
    {
        _idsFont = idsSoundFont;
    }
    ~BRWI(void);

    static PBRWI PbrwiNew(PRCA prca, int32_t kidGlass, int32_t sty);
    bool FInit(PCMD pcmd, CKI cki, PSTDIO pstdio);
};

/************************************

   Browser Roll Call Class
   Derived from the Display Class

*************************************/
#define BRWR_PAR BRWD
#define kclsBRWR 'BRWR'
typedef class BRWR *PBRWR;
class BRWR : public BRWR_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    CTG _ctg;
    PCRM _pcrm; // Chunky resource manager
    bool _fApplyingSel;

  protected:
    virtual int32_t _Cthum(void);
    virtual bool _FSetThumFrame(int32_t istn, PGOB pgobPar);
    virtual void _ReleaseThumFrame(int32_t ifrm);
    virtual void _ApplySelection(int32_t thumSelect, int32_t sid);
    virtual void _ProcessSelection(void);
    virtual bool _FClearHelp(int32_t ifrm);
    int32_t _IaridFromIthum(int32_t ithum, int32_t iaridFirst = 0);
    int32_t _IthumFromArid(int32_t arid);

  public:
    //
    // Constructors and destructors
    //
    BRWR(PGCB pgcb) : BRWR_PAR(pgcb)
    {
        _fApplyingSel = fFalse;
        _idsFont = idsRollCallFont;
    }
    ~BRWR(void);

    static PBRWR PbrwrNew(PRCA prca, int32_t kid);
    void Init(PCMD pcmd, int32_t thumSelect, int32_t thumDisplay, PSTDIO pstdio, bool fWrapScroll = fTrue,
              int32_t cthumScroll = ivNil);
    bool FInit(PCMD pcmd, CTG ctg, int32_t ithumDisplay, PSTDIO pstdio);
    bool FUpdate(int32_t arid, PSTDIO pstdio);
    bool FApplyingSel(void)
    {
        AssertBaseThis(0);
        return _fApplyingSel;
    }
};

const int32_t kglcmgGrow = 8;
struct CMG // Gokd Cno Map
{
    CNO cnoTmpl; // Content cno
    CNO cnoGokd; // Thumbnail gokd cno
};

/************************************

   Fne for  Thumbnails
   Enumerates current product first

*************************************/
#define FNET_PAR BASE
#define kclsFNET 'FNET'
typedef class FNET *PFNET;
class FNET : public FNET_PAR
{
    RTCLASS_DEC

  protected:
    bool _fInitMSKDir;
    FNE _fne;
    FNE _fneDir;
    FNI _fniDirMSK;
    FNI _fniDir;
    FNI _fniDirProduct;
    bool _fInited;

  protected:
    bool _FNextFni(FNI *pfni, int32_t *psid);

  public:
    //
    // Constructors and destructors
    //
    FNET(void) : FNET_PAR()
    {
        _fInited = fFalse;
    }
    ~FNET(void){};

    bool FInit(void);
    bool FNext(FNI *pfni, int32_t *psid = pvNil);
};

#endif
