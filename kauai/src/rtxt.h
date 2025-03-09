/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Rich text document and supporting views.

***************************************************************************/
#ifndef RTXT_H
#define RTXT_H

/***************************************************************************
    Character properties - if you change this, make sure to update
    FetchChp and _TGetLwFromChp.
***************************************************************************/
struct CHP
{
    uint32_t grfont;   // bold, italic, etc
    int32_t onn;       // which font
    int32_t dypFont;   // size of the font
    int32_t dypOffset; // sub/superscript (-128 to 127)
    ACR acrFore;       // text color
    ACR acrBack;       // background color

    void Clear(void)
    {
        ClearPb(this, offset(CHP, acrFore));
        acrFore = kacrBlack;
        acrBack = kacrBlack;
    }
};
typedef CHP *PCHP;

/***************************************************************************
    Paragraph properties - if you change these, make sure to update
    FetchPap and _TGetLwFromPap.  The dypExtraLine and numLine fields are
    used to change the height of lines from their default.  The line height
    used is calculated as (numLine / 256) * dyp + dypExtraLine, where dyp
    is the natural height of the line.
***************************************************************************/
enum
{
    jcMin,
    jcLeft = jcMin,
    jcRight,
    jcCenter,
    jcLim
};

enum
{
    ndMin,
    ndNone = ndMin,
    ndFirst, // just on the left
    ndRest,  // just on the left
    ndAll,   // on both sides
    ndLim
};

struct PAP
{
    uint8_t jc;
    uint8_t nd;
    int16_t dxpTab;
    int16_t numLine;
    int16_t dypExtraLine;
    int16_t numAfter;
    int16_t dypExtraAfter;
};
typedef PAP *PPAP;
const int16_t kdenLine = 256;
const int16_t kdenAfter = 256;
const int16_t kdxpTabDef = (kdzpInch / 4);
const int16_t kdxpDocDef = (kdzpInch * 6);

const achar kchObject = 1;

/***************************************************************************
    Generic text document base class
***************************************************************************/
const int32_t kcchMaxTxtbCache = 512;
typedef class TXTB *PTXTB;
#define TXTB_PAR DOCB
#define kclsTXTB KLCONST4('T', 'X', 'T', 'B')
class TXTB : public TXTB_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PFIL _pfil;
    PBSF _pbsf;
    ACR _acrBack;
    int32_t _dxpDef; // default width of the document
    int32_t _cpMinCache;
    int32_t _cpLimCache;
    achar _rgchCache[kcchMaxTxtbCache];

    int32_t _cactSuspendUndo; // > 0 means don't set up undo records.
    int32_t _cactCombineUndo; // determines whether we can combine undo records.

    TXTB(PDOCB pdocb = pvNil, uint32_t grfdoc = fdocNil);
    ~TXTB(void);
    virtual bool _FInit(PFNI pfni = pvNil, PBSF pbsf = pvNil, int16_t osk = koskCur);
    virtual bool _FLoad(int16_t osk = koskCur);
    virtual achar _ChFetch(int32_t cp);
    virtual void _CacheRange(int32_t cpMin, int32_t cpLim);
    virtual void _InvalCache(int32_t cp, int32_t ccpIns, int32_t ccpDel);

  public:
    virtual void InvalAllDdg(int32_t cp, int32_t ccpIns, int32_t ccpDel, uint32_t grfdoc = fdocUpdate);

    // REVIEW shonk: this is needed for using a text document as input to a lexer.
    // The bsf returned is read-only!!!!
    PBSF Pbsf(void)
    {
        AssertThis(0);
        return _pbsf;
    }

    int32_t CpMac(void);
    bool FMinPara(int32_t cp);
    int32_t CpMinPara(int32_t cp);
    int32_t CpLimPara(int32_t cp);
    int32_t CpPrev(int32_t cp, bool fWord = fFalse);
    int32_t CpNext(int32_t cp, bool fWord = fFalse);
    ACR AcrBack(void)
    {
        return _acrBack;
    }
    void SetAcrBack(ACR acr, uint32_t grfdoc = fdocUpdate);
    int32_t DxpDef(void)
    {
        return _dxpDef;
    }
    virtual void SetDxpDef(int32_t dxp);

    virtual void FetchRgch(int32_t cp, int32_t ccp, achar *prgch);
    virtual bool FReplaceRgch(const void *prgch, int32_t ccpIns, int32_t cp, int32_t ccpDel,
                              uint32_t grfdoc = fdocUpdate);
    virtual bool FReplaceFlo(PFLO pflo, bool fCopy, int32_t cp, int32_t ccpDel, int16_t osk = koskCur,
                             uint32_t grfdoc = fdocUpdate);
    virtual bool FReplaceBsf(PBSF pbsfSrc, int32_t cpSrc, int32_t ccpSrc, int32_t cpDst, int32_t ccpDel,
                             uint32_t grfdoc = fdocUpdate);
    virtual bool FReplaceTxtb(PTXTB ptxtbSrc, int32_t cpSrc, int32_t ccpSrc, int32_t cpDst, int32_t ccpDel,
                              uint32_t grfdoc = fdocUpdate);
    virtual bool FGetObjectRc(int32_t cp, PGNV pgnv, PCHP pchp, RC *prc);
    virtual bool FDrawObject(int32_t cp, PGNV pgnv, int32_t *pxp, int32_t yp, PCHP pchp, RC *prcClip);

    virtual bool FGetFni(FNI *pfni) override;

    virtual void HideSel(void);
    virtual void SetSel(int32_t cp1, int32_t cp2, int32_t gin = kginDraw);
    virtual void ShowSel(void);

    virtual void SuspendUndo(void);
    virtual void ResumeUndo(void);
    virtual bool FSetUndo(int32_t cp1, int32_t cp2, int32_t ccpIns);
    virtual void CancelUndo(void);
    virtual void CommitUndo(void);
    virtual void BumpCombineUndo(void);

    virtual bool FFind(const achar *prgch, int32_t cch, int32_t cpStart, int32_t *pcpMin, int32_t *pcpLim,
                       bool fCaseSensitive = fFalse);

    virtual void ExportFormats(PCLIP pclip) override;
};

/***************************************************************************
    Plain text document class
***************************************************************************/
typedef class TXPD *PTXPD;
#define TXPD_PAR TXTB
#define kclsTXPD KLCONST4('T', 'X', 'P', 'D')
class TXPD : public TXPD_PAR
{
    RTCLASS_DEC

  protected:
    TXPD(PDOCB pdocb = pvNil, uint32_t grfdoc = fdocNil);

  public:
    static PTXPD PtxpdNew(PFNI pfni = pvNil, PBSF pbsf = pvNil, int16_t osk = koskCur, PDOCB pdocb = pvNil,
                          uint32_t grfdoc = fdocNil);

    virtual PDDG PddgNew(PGCB pgcb) override;
    virtual bool FSaveToFni(FNI *pfni, bool fSetFni) override;
};

/***************************************************************************
    Rich text document class.
***************************************************************************/
const int32_t kcpMaxTxrd = 0x00800000; // 8MB
typedef class RTUN *PRTUN;

typedef class TXRD *PTXRD;
#define TXRD_PAR TXTB
#define kclsTXRD KLCONST4('T', 'X', 'R', 'D')
class TXRD : public TXRD_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    // WARNING: changing these values affects the file format
    // NOTE: Originally, _FSprmInAg was a virtual TXRD method called to
    // determine if a sprm stored its value in the AG. This didn't work
    // well when a subclass had a sprm in an AG (it broke undo for that
    // sprm). To fix this I (shonk) made _FSprmInAg static and made it
    // know exactly which sprms use the AG. For sprms in the client range
    // or above sprmMinObj, odd ones are _not_ in the AG and even ones _are_
    // in the AG. I couldn't just use the odd/even rule throughout the
    // range, because that would have required changing values of old
    // sprms, which would have broken existing rich text documents.
    enum
    {
        sprmNil = 0,

        // character properties
        sprmMinChp = 1,
        sprmStyle = 1,     // bold, italic, etc, font size, dypOffset
        sprmFont = 2,      // font - in the AG
        sprmForeColor = 3, // foreground color
        sprmBackColor = 4, // background color
        sprmLimChp,

        // client character properties start at 64
        sprmMinChpClient = 64, // for subclassed character properties

        // paragraph properties
        sprmMinPap = 128,
        sprmHorz = 128,  // justification, indenting and dxpTab
        sprmVert = 129,  // numLine and dypExtraLine
        sprmAfter = 130, // numAfter and dypExtraAfter
        sprmLimPap,

        // client paragraph properties
        sprmMinPapClient = 160, // for subclassed paragraph properties

        // objects - these apply to a single character, not a range
        sprmMinObj = 192,
        sprmObject = 192,
    };

    // map property entry
    struct MPE
    {
        uint32_t spcp; // sprm in the high byte and cp in the low 3 bytes
        int32_t lw;    // the associated value - meaning depends on the sprm,
                       // but 0 is _always_ the default
    };
    VERIFY_STRUCT_SIZE(MPE, 8);

    // sprm, value, mask triple
    struct SPVM
    {
        uint8_t sprm;
        int32_t lw;
        int32_t lwMask;
    };
    VERIFY_STRUCT_SIZE(SPVM, 12);

    // rich text document properties
    struct RDOP
    {
        int16_t bo;
        int16_t oskFont;
        int32_t dxpDef;
        int32_t dypFont;
        int32_t lwAcrBack;
        // uint8_t rgbStnFont[]; font name
    };
    VERIFY_STRUCT_SIZE(RDOP, 16);
#define kbomRdop 0x5FC00000

    PCFL _pcfl;
    PGL _pglmpe;
    PAG _pagcact; // for sprm's that have more than a long's worth of data

    int32_t _onnDef; // default font and font size
    int32_t _dypFontDef;
    int16_t _oskFont; // osk for the default font
    STN _stnFontDef;  // name of default font

    // cached CHP and PAP (from FetchChp and FetchPap)
    CHP _chp;
    int32_t _cpMinChp, _cpLimChp;
    PAP _pap;
    int32_t _cpMinPap, _cpLimPap;

    // current undo record
    PRTUN _prtun;

    TXRD(PDOCB pdocb = pvNil, uint32_t grfdoc = fdocNil);
    ~TXRD(void);
    bool _FInit(PFNI pfni = pvNil, CTG ctg = kctgRichText);
    virtual bool _FReadChunk(PCFL pcfl, CTG ctg, CNO cno, bool fCopyText);
    virtual bool _FOpenArg(int32_t icact, uint8_t sprm, int16_t bo, int16_t osk);

    uint32_t _SpcpFromSprmCp(uint8_t sprm, int32_t cp)
    {
        return ((uint32_t)sprm << 24) | (cp & 0x00FFFFFF);
    }
    uint8_t _SprmFromSpcp(uint32_t spcp)
    {
        return B3Lw(spcp);
    }
    int32_t _CpFromSpcp(uint32_t spcp)
    {
        return (int32_t)(spcp & 0x00FFFFFF);
    }
    bool _FFindMpe(uint32_t spcp, MPE *pmpe, int32_t *pcpLim = pvNil, int32_t *pimpe = pvNil);
    bool _FFetchProp(int32_t impe, uint8_t *psprm, int32_t *plw = pvNil, int32_t *pcpMin = pvNil,
                     int32_t *pcpLim = pvNil);
    bool _FEnsureInAg(uint8_t sprm, void *pv, int32_t cb, int32_t *pjv);
    void _ReleaseInAg(int32_t jv);
    void _AddRefInAg(int32_t jv);
    void _ReleaseSprmLw(uint8_t sprm, int32_t lw);
    void _AddRefSprmLw(uint8_t sprm, int32_t lw);
    tribool _TGetLwFromChp(uint8_t sprm, PCHP pchpNew, PCHP pchpOld, int32_t *plw, int32_t *plwMask);
    tribool _TGetLwFromPap(uint8_t sprm, PPAP ppapNew, PPAP ppapOld, int32_t *plw, int32_t *plwMask);
    bool _FGetRgspvmFromChp(PCHP pchp, PCHP pchpDiff, SPVM *prgspvm, int32_t *pcspvm);
    bool _FGetRgspvmFromPap(PPAP ppap, PPAP ppapDiff, SPVM *prgspvm, int32_t *pcspvm);
    void _ReleaseRgspvm(SPVM *prgspvm, int32_t cspvm);
    void _ApplyRgspvm(int32_t cp, int32_t ccp, SPVM *prgspvm, int32_t cspvm);
    void _GetParaBounds(int32_t *pcpMin, int32_t *pccp, bool fExpand);
    void _AdjustMpe(int32_t cp, int32_t ccpIns, int32_t ccpDel);
    void _CopyProps(PTXRD ptxrd, int32_t cpSrc, int32_t cpDst, int32_t ccpSrc, int32_t ccpDst, uint8_t sprmMin,
                    uint8_t sprmLim);

    virtual bool _FGetObjectRc(int32_t icact, uint8_t sprm, PGNV pgnv, PCHP pchp, RC *prc);
    virtual bool _FDrawObject(int32_t icact, uint8_t sprm, PGNV pgnv, int32_t *pxp, int32_t yp, PCHP pchp, RC *prcClip);

    bool _FReplaceCore(void *prgch, PFLO pflo, bool fCopy, PBSF pbsf, int32_t cpSrc, int32_t ccpIns, int32_t cp,
                       int32_t ccpDel, PCHP pchp, PPAP ppap, uint32_t grfdoc);

    static bool _FSprmInAg(uint8_t sprm);

  public:
    static PTXRD PtxrdNew(PFNI pfni = pvNil);
    static PTXRD PtxrdReadChunk(PCFL pcfl, CTG ctg, CNO cno, bool fCopyText = fTrue);

    virtual PDDG PddgNew(PGCB pgcb) override;

    void FetchChp(int32_t cp, PCHP pchp, int32_t *pcpMin = pvNil, int32_t *pcpLim = pvNil);
    void FetchPap(int32_t cp, PPAP ppap, int32_t *pcpMin = pvNil, int32_t *pcpLim = pvNil);

    bool FApplyChp(int32_t cp, int32_t ccp, PCHP pchp, PCHP pchpDiff = pvNil, uint32_t grfdoc = fdocUpdate);
    bool FApplyPap(int32_t cp, int32_t ccp, PPAP ppap, PPAP ppapDiff, int32_t *pcpMin = pvNil, int32_t *pcpLim = pvNil,
                   bool fExpand = fTrue, uint32_t grfdoc = fdocUpdate);

    virtual bool FReplaceRgch(void *prgch, int32_t ccpIns, int32_t cp, int32_t ccpDel, uint32_t grfdoc = fdocUpdate);
    virtual bool FReplaceFlo(PFLO pflo, bool fCopy, int32_t cp, int32_t ccpDel, int16_t osk = koskCur,
                             uint32_t grfdoc = fdocUpdate) override;
    virtual bool FReplaceBsf(PBSF pbsfSrc, int32_t cpSrc, int32_t ccpSrc, int32_t cpDst, int32_t ccpDel,
                             uint32_t grfdoc = fdocUpdate) override;
    virtual bool FReplaceTxtb(PTXTB ptxtbSrc, int32_t cpSrc, int32_t ccpSrc, int32_t cpDst, int32_t ccpDel,
                              uint32_t grfdoc = fdocUpdate) override;
    bool FReplaceRgch(void *prgch, int32_t ccpIns, int32_t cp, int32_t ccpDel, PCHP pchp, PPAP ppap = pvNil,
                      uint32_t grfdoc = fdocUpdate);
    bool FReplaceFlo(PFLO pflo, bool fCopy, int32_t cp, int32_t ccpDel, PCHP pchp, PPAP ppap = pvNil,
                     int16_t osk = koskCur, uint32_t grfdoc = fdocUpdate);
    bool FReplaceBsf(PBSF pbsfSrc, int32_t cpSrc, int32_t ccpSrc, int32_t cpDst, int32_t ccpDel, PCHP pchp,
                     PPAP ppap = pvNil, uint32_t grfdoc = fdocUpdate);
    bool FReplaceTxtb(PTXTB ptxtbSrc, int32_t cpSrc, int32_t ccpSrc, int32_t cpDst, int32_t ccpDel, PCHP pchp,
                      PPAP ppap = pvNil, uint32_t grfdoc = fdocUpdate);
    bool FReplaceTxrd(PTXRD ptxrd, int32_t cpSrc, int32_t ccpSrc, int32_t cpDst, int32_t ccpDel,
                      uint32_t grfdoc = fdocUpdate);

    bool FFetchObject(int32_t cpMin, int32_t *pcp, void **ppv = pvNil, int32_t *pcb = pvNil);
    virtual bool FInsertObject(void *pv, int32_t cb, int32_t cp, int32_t ccpDel, PCHP pchp = pvNil,
                               uint32_t grfdoc = fdocUpdate);
    virtual bool FApplyObjectProps(void *pv, int32_t cb, int32_t cp, uint32_t grfdoc = fdocUpdate);

    virtual bool FGetObjectRc(int32_t cp, PGNV pgnv, PCHP pchp, RC *prc) override;
    virtual bool FDrawObject(int32_t cp, PGNV pgnv, int32_t *pxp, int32_t yp, PCHP pchp, RC *prcClip) override;

    virtual bool FGetFni(FNI *pfni) override;
    virtual bool FGetFniSave(FNI *pfni) override;
    virtual bool FSaveToFni(FNI *pfni, bool fSetFni) override;
    virtual bool FSaveToChunk(PCFL pcfl, CKI *pcki, bool fRedirectText = fFalse);

    virtual bool FSetUndo(int32_t cp1, int32_t cp2, int32_t ccpIns) override;
    virtual void CancelUndo(void) override;
    virtual void CommitUndo(void) override;
};

/***************************************************************************
    Rich text undo object.
***************************************************************************/
typedef class RTUN *PRTUN;
#define RTUN_PAR UNDB
#define kclsRTUN KLCONST4('R', 'T', 'U', 'N')
class RTUN : public RTUN_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    int32_t _cactCombine; // RTUNs with different values can't be combined
    PTXRD _ptxrd;         // copy of replaced text
    int32_t _cpMin;       // where the text came from in the original RTXD
    int32_t _ccpIns;      // how many characters the original text was replaced with

  public:
    static PRTUN PrtunNew(int32_t cactCombine, PTXRD ptxrd, int32_t cp1, int32_t cp2, int32_t ccpIns);
    ~RTUN(void);

    virtual bool FUndo(PDOCB pdocb) override;
    virtual bool FDo(PDOCB pdocb) override;

    bool FCombine(PRTUN prtun);
};

/***************************************************************************
    Text document display GOB - DDG for a TXTB.
***************************************************************************/
const int32_t kdxpIndentTxtg = (kdzpInch / 8);
const int32_t kcchMaxLineTxtg = 512;
typedef class TRUL *PTRUL;

enum
{
    ftxtgNil = 0,
    ftxtgNoColor = 1,
};

typedef class TXTG *PTXTG;
#define TXTG_PAR DDG
#define kclsTXTG KLCONST4('T', 'X', 'T', 'G')
class TXTG : public TXTG_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    // line information
    struct LIN
    {
        int32_t cpMin;  // the cp of the first character in this line
        int32_t dypTot; // the total height of lines up to this line
        int16_t ccp;
        int16_t xpLeft;
        int16_t dyp;
        int16_t dypAscent;
    };

    PTXTB _ptxtb;
    PGL _pgllin;
    int32_t _ilinDisp;
    int32_t _cpDisp;
    int32_t _dypDisp;
    int32_t _ilinInval; // LINs from here on have wrong cpMin and dypTot values
    PGNV _pgnv;

    // the selection
    int32_t _cpAnchor;
    int32_t _cpOther;
    uint32_t _tsSel;
    int32_t _xpSel;
    bool _fSelOn : 1;
    bool _fXpValid : 1;
    bool _fMark : 1;
    bool _fClear : 1;
    bool _fSelByWord : 1;

    // the ruler
    PTRUL _ptrul;

    TXTG(PTXTB ptxtb, PGCB pgcb);
    ~TXTG(void);

    virtual bool _FInit(void) override;
    virtual void _Activate(bool fActive) override;

    virtual int32_t _DxpDoc(void);
    virtual void _FetchChp(int32_t cp, PCHP pchp, int32_t *pcpMin = pvNil, int32_t *pcpLim = pvNil) = 0;
    virtual void _FetchPap(int32_t cp, PPAP ppap, int32_t *pcpMin = pvNil, int32_t *pcpLim = pvNil) = 0;

    void _CalcLine(int32_t cpMin, int32_t dyp, LIN *plin);
    void _Reformat(int32_t cp, int32_t ccpIns, int32_t ccpDel, int32_t *pyp = pvNil, int32_t *pdypIns = pvNil,
                   int32_t *pdypDel = pvNil);
    void _ReformatAndDraw(int32_t cp, int32_t ccpIns, int32_t ccpDel);

    void _FetchLin(int32_t ilin, LIN *plin, int32_t *pilinActual = pvNil);
    void _FindCp(int32_t cpFind, LIN *plin, int32_t *pilin = pvNil, bool fCalcLines = fTrue);
    void _FindDyp(int32_t dypFind, LIN *plin, int32_t *pilin = pvNil, bool fCalcLines = fTrue);

    bool _FGetCpFromPt(int32_t xp, int32_t yp, int32_t *pcp, bool fClosest = fTrue);
    bool _FGetCpFromXp(int32_t xp, LIN *plin, int32_t *pcp, bool fClosest = fTrue);
    void _GetXpYpFromCp(int32_t cp, int32_t *pypMin, int32_t *pypLim, int32_t *pxp, int32_t *pdypBaseLine = pvNil,
                        bool fView = fTrue);
    int32_t _DxpFromCp(int32_t cpLine, int32_t cp);
    void _SwitchSel(bool fOn, int32_t gin = kginDraw);
    void _InvertSel(PGNV pgnv, int32_t gin = kginDraw);
    void _InvertCpRange(PGNV pgnv, int32_t cp1, int32_t cp2, int32_t gin = kginDraw);

    virtual int32_t _ScvMax(bool fVert) override;
    virtual void _Scroll(int32_t scaHorz, int32_t scaVert, int32_t scvHorz = 0, int32_t scvVert = 0) override;
    virtual void _ScrollDxpDyp(int32_t dxp, int32_t dyp) override;
    virtual int32_t _DypTrul(void);
    virtual PTRUL _PtrulNew(PGCB pgcb);
    virtual void _DrawLinExtra(PGNV pgnv, RC *prcClip, LIN *plin, int32_t dxp, int32_t yp, uint32_t grftxtg);

  public:
    virtual void DrawLines(PGNV pgnv, RC *prcClip, int32_t dxp, int32_t dyp, int32_t ilinMin, int32_t ilinLim = klwMax,
                           uint32_t grftxtg = ftxtgNil);

    virtual void Draw(PGNV pgnv, RC *prcClip) override;
    virtual bool FCmdTrackMouse(PCMD_MOUSE pcmd) override;
    virtual bool FCmdKey(PCMD_KEY pcmd) override;
    virtual bool FCmdSelIdle(PCMD pcmd) override;
    virtual void InvalCp(int32_t cp, int32_t ccpIns, int32_t ccpDel);

    virtual void HideSel(void);
    virtual void GetSel(int32_t *pcpAnchor, int32_t *pcpOther);
    virtual void SetSel(int32_t cpAnchor, int32_t cpOther, int32_t gin = kginDraw);
    virtual bool FReplace(achar *prgch, int32_t cch, int32_t cp1, int32_t cp2);
    void ShowSel(void);
    PTXTB Ptxtb(void)
    {
        return _ptxtb;
    }

    virtual void ShowRuler(bool fShow = fTrue);
    virtual void SetDxpTab(int32_t dxp);
    virtual void SetDxpDoc(int32_t dxp);
    virtual void GetNaturalSize(int32_t *pdxp, int32_t *pdyp);
};

/***************************************************************************
    Line text document display gob
***************************************************************************/
typedef class TXLG *PTXLG;
#define TXLG_PAR TXTG
#define kclsTXLG KLCONST4('T', 'X', 'L', 'G')
class TXLG : public TXLG_PAR
{
    RTCLASS_DEC

  protected:
    // the font
    int32_t _onn;
    uint32_t _grfont;
    int32_t _dypFont;
    int32_t _dxpChar;
    int32_t _cchTab;

    TXLG(PTXTB ptxtb, PGCB pgcb, int32_t onn, uint32_t grfont, int32_t dypFont, int32_t cchTab);

    virtual int32_t _DxpDoc(void) override;
    virtual void _FetchChp(int32_t cp, PCHP pchp, int32_t *pcpMin = pvNil, int32_t *pcpLim = pvNil) override;
    virtual void _FetchPap(int32_t cp, PPAP ppap, int32_t *pcpMin = pvNil, int32_t *pcpLim = pvNil) override;

    // clipboard support
    virtual bool _FCopySel(PDOCB *ppdocb = pvNil) override;
    virtual void _ClearSel(void) override;
    virtual bool _FPaste(PCLIP pclip, bool fDoIt, int32_t cid) override;

  public:
    static PTXLG PtxlgNew(PTXTB ptxtb, PGCB pgcb, int32_t onn, uint32_t grfont, int32_t dypFont, int32_t cchTab);

    virtual void SetDxpTab(int32_t dxp) override;
    virtual void SetDxpDoc(int32_t dxp) override;
};

/***************************************************************************
    Rich text document display gob
***************************************************************************/
typedef class TXRG *PTXRG;
#define TXRG_PAR TXTG
#define kclsTXRG KLCONST4('T', 'X', 'R', 'G')
class TXRG : public TXRG_PAR
{
    RTCLASS_DEC
    CMD_MAP_DEC(TXRG)
    ASSERT

  protected:
    TXRG(PTXRD ptxrd, PGCB pgcb);

    CHP _chpIns;
    bool _fValidChp;

    virtual void _FetchChp(int32_t cp, PCHP pchp, int32_t *pcpMin = pvNil, int32_t *pcpLim = pvNil) override;
    virtual void _FetchPap(int32_t cp, PPAP ppap, int32_t *pcpMin = pvNil, int32_t *pcpLim = pvNil) override;

    virtual bool _FGetOtherSize(int32_t *pdypFont);
    virtual bool _FGetOtherSubSuper(int32_t *pdypOffset);

    // clipboard support
    virtual bool _FCopySel(PDOCB *ppdocb = pvNil) override;
    virtual void _ClearSel(void) override;
    virtual bool _FPaste(PCLIP pclip, bool fDoIt, int32_t cid) override;

    void _FetchChpSel(int32_t cp1, int32_t cp2, PCHP pchp);
    void _EnsureChpIns(void);

  public:
    static PTXRG PtxrgNew(PTXRD ptxrd, PGCB pgcb);

    virtual void SetSel(int32_t cpAnchor, int32_t cpOther, int32_t gin = kginDraw) override;
    virtual bool FReplace(achar *prgch, int32_t cch, int32_t cp1, int32_t cp2) override;
    virtual bool FApplyChp(PCHP pchp, PCHP pchpDiff = pvNil);
    virtual bool FApplyPap(PPAP ppap, PPAP ppapDiff = pvNil, bool fExpand = fTrue);

    virtual bool FCmdApplyProperty(PCMD pcmd);
    virtual bool FEnablePropCmd(PCMD pcmd, uint32_t *pgrfeds);
    bool FSetColor(ACR *pacrFore, ACR *pacrBack);

    virtual void SetDxpTab(int32_t dxp) override;
};

/***************************************************************************
    The ruler for a rich text document.
***************************************************************************/
typedef class TRUL *PTRUL;
#define TRUL_PAR GOB
#define kclsTRUL KLCONST4('T', 'R', 'U', 'L')
class TRUL : public TRUL_PAR
{
    RTCLASS_DEC

  protected:
    TRUL(GCB *pgcb) : TRUL_PAR(pgcb)
    {
    }

  public:
    virtual void SetDxpTab(int32_t dxpTab) = 0;
    virtual void SetDxpDoc(int32_t dxpDoc) = 0;
    virtual void SetXpLeft(int32_t xpLeft) = 0;
};

#endif //! RTXT_H
