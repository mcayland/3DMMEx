/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************

    CHED document class

***************************************************************************/
#ifndef CHDOC_H
#define CHDOC_H

typedef class DOC *PDOC;
typedef class DOCE *PDOCE;
typedef class DOCH *PDOCH;
typedef class DOCG *PDOCG;
typedef class DOCI *PDOCI;
typedef class DOCPIC *PDOCPIC;
typedef class DOCMBMP *PDOCMBMP;
typedef class SEL *PSEL;
typedef class DCD *PDCD;
typedef class DCH *PDCH;
typedef class DCGB *PDCGB;
typedef class DCGL *PDCGL;
typedef class DCGG *PDCGG;
typedef class DCST *PDCST;
typedef class DCPIC *PDCPIC;
typedef class DCMBMP *PDCMBMP;

bool FGetCtgFromStn(CTG *pctg, PSTN pstn);

#define lnNil (-1L)

/***************************************************************************

    Various document classes. DOC is the chunky file based document.
    DOCE is a virtual class for documents that represent an individual
    chunk in a DOC. A DOCE is a child document of a DOC. All other
    document classes in this header are derived from DOCE.

***************************************************************************/

/***************************************************************************
    chunky file doc
***************************************************************************/
#define DOC_PAR DOCB
#define kclsDOC KLCONST3('D', 'O', 'C')
class DOC : public DOC_PAR
{
    RTCLASS_DEC
    ASSERT

  protected:
    PCFL _pcfl; // the chunky file

    DOC(void);
    ~DOC(void);

  public:
    static PDOC PdocNew(FNI *pfni);

    PCFL Pcfl(void)
    {
        return _pcfl;
    }
    virtual PDDG PddgNew(PGCB pgcb) override;
    virtual bool FGetFni(FNI *pfni) override;
    virtual bool FGetFniSave(FNI *pfni) override;
    virtual bool FSaveToFni(FNI *pfni, bool fSetFni) override;
};

/***************************************************************************
    Chunky editing doc - abstract class for editing a single chunk in a
    Chunky file. An instance of this class is a child doc of a DOC. Many
    document classes below are all derived from this.
***************************************************************************/
#define DOCE_PAR DOCB
#define kclsDOCE KLCONST4('D', 'O', 'C', 'E')
class DOCE : public DOCE_PAR
{
    RTCLASS_DEC
    ASSERT

  protected:
    PCFL _pcfl; // which chunk is being edited
    CTG _ctg;
    CNO _cno;

    DOCE(PDOCB pdocb, PCFL pcfl, CTG ctg, CNO cno);
    bool _FInit(void);

    virtual bool _FSaveToChunk(CTG ctg, CNO cno, bool fRedirect);
    virtual bool _FWrite(PBLCK pblck, bool fRedirect) = 0;
    virtual int32_t _CbOnFile(void) = 0;
    virtual bool _FRead(PBLCK pblck) = 0;

  public:
    static PDOCE PdoceFromChunk(PDOCB pdocb, PCFL pcfl, CTG ctg, CNO cno);
    static void CloseDeletedDoce(PDOCB pdocb);

    virtual void GetName(PSTN pstn) override;
    virtual bool FSave(int32_t cid) override;
};

/***************************************************************************
    Hex editor document - for editing any chunk as a hex stream.
***************************************************************************/
#define DOCH_PAR DOCE
#define kclsDOCH KLCONST4('D', 'O', 'C', 'H')
class DOCH : public DOCH_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    BSF _bsf; // the byte stream

    DOCH(PDOCB pdocb, PCFL pcfl, CTG ctg, CNO cno);
    virtual bool _FWrite(PBLCK pblck, bool fRedirect) override;
    virtual int32_t _CbOnFile(void) override;
    virtual bool _FRead(PBLCK pblck) override;

  public:
    static PDOCH PdochNew(PDOCB pdocb, PCFL pcfl, CTG ctg, CNO cno);
    virtual PDDG PddgNew(PGCB pgcb) override;
};

/***************************************************************************
    Group editor document - for editing GL, AL, GG, AG, GST, and AST.
***************************************************************************/
#define DOCG_PAR DOCE
#define kclsDOCG KLCONST4('D', 'O', 'C', 'G')
class DOCG : public DOCG_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PGRPB _pgrpb;
    int32_t _cls; // which class the group belongs to
    short _bo;
    short _osk;

    DOCG(PDOCB pdocb, PCFL pcfl, CTG ctg, CNO cno, int32_t cls);
    ~DOCG(void);
    virtual bool _FWrite(PBLCK pblck, bool fRedirect) override;
    virtual int32_t _CbOnFile(void) override;
    virtual bool _FRead(PBLCK pblck) override;

  public:
    static PDOCG PdocgNew(PDOCB pdocb, PCFL pcfl, CTG ctg, CNO cno, int32_t cls);
    virtual PDDG PddgNew(PGCB pgcb) override;

    PDOCI PdociFromItem(int32_t iv, int32_t dln);
    void CloseDeletedDoci(int32_t iv, int32_t cvDel);
    PGRPB Pgrpb(void)
    {
        return _pgrpb;
    }
};

/***************************************************************************
    Item hex editor document - for editing an item in a GRPB. An instance
    of this class is normally a child doc of a DOCG (but doesn't have to be).
***************************************************************************/
#define DOCI_PAR DOCB
#define kclsDOCI KLCONST4('D', 'O', 'C', 'I')
class DOCI : public DOCI_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PGRPB _pgrpb; // the group the data came from and gets written to.
    int32_t _cls;
    int32_t _iv; // which item is being edited
    int32_t _dln;
    bool _fFixed; // indicates if the data is fixed length
    BSF _bsf;     // the byte stream we're editing

    DOCI(PDOCB pdocb, PGRPB pgrpb, int32_t cls, int32_t iv, int32_t dln);
    bool _FInit(void);

    virtual bool _FSaveToItem(int32_t iv, bool fRedirect);
    virtual bool _FWrite(int32_t iv);
    virtual HQ _HqRead();

  public:
    static PDOCI PdociNew(PDOCB pdocb, PGRPB pgrpb, int32_t cls, int32_t iv, int32_t dln);
    virtual PDDG PddgNew(PGCB pgcb) override;

    int32_t Iv(void)
    {
        return _iv;
    }
    int32_t Dln(void)
    {
        return _dln;
    }

    virtual void GetName(PSTN pstn) override;
    virtual bool FSave(int32_t cid) override;
};

/***************************************************************************
    Picture display document.
***************************************************************************/
#define DOCPIC_PAR DOCE
#define kclsDOCPIC KLCONST4('d', 'c', 'p', 'c')
class DOCPIC : public DOCPIC_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PPIC _ppic;

    DOCPIC(PDOCB pdocb, PCFL pcfl, CTG ctg, CNO cno);
    ~DOCPIC(void);

    virtual bool _FWrite(PBLCK pblck, bool fRedirect) override;
    virtual int32_t _CbOnFile(void) override;
    virtual bool _FRead(PBLCK pblck) override;

  public:
    static PDOCPIC PdocpicNew(PDOCB pdocb, PCFL pcfl, CTG ctg, CNO cno);

    virtual PDDG PddgNew(PGCB pgcb) override;
    PPIC Ppic(void)
    {
        return _ppic;
    }
};

/***************************************************************************
    MBMP display document.
***************************************************************************/
#define DOCMBMP_PAR DOCE
#define kclsDOCMBMP KLCONST4('d', 'o', 'c', 'm')
class DOCMBMP : public DOCMBMP_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PMBMP _pmbmp;

    DOCMBMP(PDOCB pdocb, PCFL pcfl, CTG ctg, CNO cno);
    ~DOCMBMP(void);

    virtual bool _FWrite(PBLCK pblck, bool fRedirect) override;
    virtual int32_t _CbOnFile(void) override;
    virtual bool _FRead(PBLCK pblck) override;

  public:
    static PDOCMBMP PdocmbmpNew(PDOCB pdocb, PCFL pcfl, CTG ctg, CNO cno);

    virtual PDDG PddgNew(PGCB pgcb) override;
    PMBMP Pmbmp(void)
    {
        return _pmbmp;
    }
};

/***************************************************************************
    Document editing window classes follow. These are all DDG's.
    Most are also DCLB's (the first class defined below).  DCLB is
    an abstract class that handles a line based editing window.
    The DCD class is for displaying a DOC (chunky file document).
***************************************************************************/

/***************************************************************************
    abstract class for line based document windows
***************************************************************************/
#define DCLB_PAR DDG
#define kclsDCLB KLCONST4('D', 'C', 'L', 'B')
class DCLB : public DCLB_PAR
{
    RTCLASS_DEC
    ASSERT

  protected:
    int32_t _onn;       // fixed width font to use
    int32_t _dypHeader; // height of the header
    int32_t _dypLine;   // height of one line
    int32_t _dxpChar;   // width of a character

    DCLB(PDOCB pdocb, PGCB pgcb);
    virtual void _Scroll(int32_t scaHorz, int32_t scaVert, int32_t scvHorz = 0, int32_t scvVert = 0) override;
    virtual void _ScrollDxpDyp(int32_t dxp, int32_t dyp) override;
    virtual void GetMinMax(RC *prcMinMax) override;

    int32_t _YpFromLn(int32_t ln)
    {
        return LwMul(ln - _scvVert, _dypLine) + _dypHeader;
    }
    int32_t _XpFromIch(int32_t ich)
    {
        return LwMul(ich - _scvHorz + 1, _dxpChar);
    }
    int32_t _LnFromYp(int32_t yp);

    void _GetContent(RC *prc);
};

/***************************************************************************
    SEL: used to track a selection in a chunky file doc
***************************************************************************/
enum
{
    fselNil = 0,
    fselCki = 1,
    fselKid = 2
};

#define SEL_PAR BASE
#define kclsSEL KLCONST3('S', 'E', 'L')
class SEL : public SEL_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PCFL _pcfl;
    int32_t _icki;
    int32_t _ikid;
    CKI _cki;
    KID _kid;
    int32_t _ln;
    int32_t _lnLim;      // this is lnNil if we haven't yet calculated the lim
    PGL _pglctg;         // the ctgs to filter on
    bool _fHideList : 1; // whether to hide the ctgs in the list or show them
    bool _fHideKids : 1; // whether to hide the kids

    void _SetNil(void);
    bool _FFilter(CTG ctg, CNO cno);

  public:
    SEL(PCFL pcfl);
    SEL(SEL &selT);
    ~SEL(void);
    SEL &operator=(SEL &selT);

    void Adjust(bool fExact = fFalse);

    int32_t Icki(void)
    {
        return _icki;
    }
    int32_t Ikid(void)
    {
        return _ikid;
    }
    int32_t Ln(void)
    {
        return _ln;
    }
    uint32_t GrfselGetCkiKid(CKI *pcki, KID *pkid);

    bool FSetLn(int32_t ln);
    bool FAdvance(void);
    bool FRetreat(void);
    bool FSetCkiKid(CKI *pcki, KID *pkid = pvNil, bool fExact = fTrue);
    int32_t LnLim(void);
    void InvalLim(void)
    {
        _lnLim = lnNil;
    }

    bool FHideKids(void)
    {
        return _fHideKids;
    }
    void HideKids(bool fHide);

    bool FHideList(void)
    {
        return _fHideList;
    }
    void HideList(bool fHide);
    bool FGetCtgFilter(int32_t ictg, CTG *pctg);
    void FreeFilterList(void);
    bool FAddCtgFilter(CTG ctg);
};

/***************************************************************************
    Display for chunky document - displays a DOC.
***************************************************************************/
#define DCD_PAR DCLB
#define kclsDCD KLCONST3('D', 'C', 'D')
class DCD : public DCD_PAR
{
    RTCLASS_DEC
    CMD_MAP_DEC(DCD)
    ASSERT
    MARKMEM

  protected:
    int32_t _dypBorder; // height of border (included in _dypLine)
    PCFL _pcfl;         // the chunky file
    SEL _sel;           // the current selection

    DCD(PDOCB pdocb, PCFL pcfl, PGCB pgcb);
    void _DrawSel(PGNV pgnv);
    void _HiliteLn(int32_t ln);
    void _SetSel(int32_t ln, CKI *pcki = pvNil, KID *pkid = pvNil);
    void _ShowSel(void);

    virtual void _Activate(bool fActive) override;
    virtual int32_t _ScvMax(bool fVert) override;
    bool _FAddChunk(CTG ctgDef, CKI *pcki, bool *pfCreated);
    bool _FEditChunkInfo(CKI *pckiOld);
    bool _FChangeChid(CKI *pcki, KID *pkid);

    bool _FDoAdoptChunkDlg(CKI *pcki, KID *pkid);
    void _EditCki(CKI *pcki, int32_t cid);

    void _InvalCkiKid(CKI *pcki = pvNil, KID *pkid = pvNil);

    // clipboard support
    virtual bool _FCopySel(PDOCB *ppdocb = pvNil) override;
    virtual void _ClearSel(void) override;
    virtual bool _FPaste(PCLIP pclip, bool fDoIt, int32_t cid) override;

  public:
    static PDCD PdcdNew(PDOCB pdocb, PCFL pcfl, PGCB pgcb);
    static void InvalAllDcd(PDOCB pdocb, PCFL pcfl, CKI *pcki = pvNil, KID *pkid = pvNil);

    virtual void Draw(PGNV pgnv, RC *prcClip) override;
    virtual void MouseDown(int32_t xp, int32_t yp, int32_t cact, uint32_t grfcust) override;
    virtual bool FCmdKey(PCMD_KEY pcmd) override;

    virtual bool FEnableDcdCmd(PCMD pcmd, uint32_t *pgrfeds);
    virtual bool FCmdAddChunk(PCMD pcmd);
    virtual bool FCmdDeleteChunk(PCMD pcmd);
    virtual bool FCmdAdoptChunk(PCMD pcmd);
    virtual bool FCmdUnadoptChunk(PCMD pcmd);
    virtual bool FCmdEditChunk(PCMD pcmd);
    virtual bool FCmdAddPicChunk(PCMD pcmd);
    virtual bool FCmdAddBitmapChunk(PCMD pcmd);
    virtual bool FCmdImportScript(PCMD pcmd);
    virtual bool FCmdTestScript(PCMD pcmd);
    virtual bool FCmdDisasmScript(PCMD pcmd);
    virtual bool FCmdAddFileChunk(PCMD pcmd);
    virtual bool FCmdEditChunkInfo(PCMD pcmd);
    virtual bool FCmdChangeChid(PCMD pcmd);
    virtual bool FCmdSetColorTable(PCMD pcmd);
    virtual bool FCmdFilterChunk(PCMD pcmd);
    virtual bool FCmdPack(PCMD pcmd);
    virtual bool FCmdStopSound(PCMD pcmd);
    virtual bool FCmdCloneChunk(PCMD pcmd);
    virtual bool FCmdReopen(PCMD pcmd);

    bool FTestScript(CTG ctg, CNO cno, int32_t cbCache = 0x00300000L);
    bool FPlayMidi(CTG ctg, CNO cno);
    bool FPlayWave(CTG ctg, CNO cno);
};

/***************************************************************************
    Display chunk in hex - displays a BSF (byte stream), but
    doesn't necessarily display a DOCH.
***************************************************************************/
#define DCH_PAR DCLB
#define kclsDCH KLCONST3('D', 'C', 'H')
class DCH : public DCH_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PBSF _pbsf;      // the byte stream
    int32_t _cbLine; // number of bytes per line

    // the selection
    int32_t _ibAnchor;
    int32_t _ibOther;

    bool _fSelOn : 1;    // selection is showing
    bool _fRightSel : 1; // selection if on a line boundary is at the right edge
    bool _fHalfSel : 1;  // second half of hex character is selected
    bool _fHexSel : 1;   // hex area active
    bool _fFixed : 1;    // indicates if the data is fixed length

    DCH(PDOCB pdocb, PBSF pbsf, bool fFixed, PGCB pgcb);

    virtual void _Activate(bool fActive) override;
    virtual int32_t _ScvMax(bool fVert) override;

    int32_t _IchFromCb(int32_t cb, bool fHex, bool fNoTrailSpace = fFalse);
    int32_t _XpFromCb(int32_t cb, bool fHex, bool fNoTrailSpace = fFalse);
    int32_t _XpFromIb(int32_t ib, bool fHex);
    int32_t _YpFromIb(int32_t ib);
    int32_t _IbFromPt(int32_t xp, int32_t yp, tribool *ptHex, bool *pfRight = pvNil);

    void _SetSel(int32_t ibAnchor, int32_t ibOther, bool fRight);
    void _SetHalfSel(int32_t ib);
    void _SetHexSel(bool fHex);
    void _SwitchSel(bool fOn);
    void _ShowSel(void);
    void _InvertSel(PGNV pgnv);
    void _InvertIbRange(PGNV pgnv, int32_t ib1, int32_t ib2, bool fHex);

    bool _FReplace(uint8_t *prgb, int32_t cb, int32_t ibMin, int32_t ibLim, bool fHalfSel = fFalse);
    void _InvalAllDch(int32_t ib, int32_t cbIns, int32_t cbDel);
    void _InvalIb(int32_t ib, int32_t cbIns, int32_t cbDel);

    void _DrawHeader(PGNV pgnv);

    // clipboard support
    virtual bool _FCopySel(PDOCB *ppdocb = pvNil) override;
    virtual void _ClearSel(void) override;
    virtual bool _FPaste(PCLIP pclip, bool fDoIt, int32_t cid) override;

  public:
    static PDCH PdchNew(PDOCB pdocb, PBSF pbsf, bool fFixed, PGCB pgcb);

    virtual void Draw(PGNV pgnv, RC *prcClip) override;
    virtual void MouseDown(int32_t xp, int32_t yp, int32_t cact, uint32_t grfcust) override;
    virtual bool FCmdKey(PCMD_KEY pcmd) override;
};

/***************************************************************************
    Virtual class that supports displaying a group chunk - displays a GRPB.
    Usually displays a DOCG, but doesn't have to.
***************************************************************************/
#define DCGB_PAR DCLB
#define kclsDCGB KLCONST4('D', 'C', 'G', 'B')
class DCGB : public DCGB_PAR
{
    RTCLASS_DEC
    CMD_MAP_DEC(DCGB)
    ASSERT
    MARKMEM

  protected:
    int32_t _dypBorder; // height of border (included in _dypLine)
    int32_t _clnItem;   // number of lines for each item
    int32_t _ivCur;     // which item is selected
    int32_t _dlnCur;    // which line in the item is selected
    PGRPB _pgrpb;       // the group we're displaying
    int32_t _cls;       // the class of the group
    bool _fAllocated;   // whether the class is allocated or general

    DCGB(PDOCB pdocb, PGRPB pgrpb, int32_t cls, int32_t clnItem, PGCB pgcb);

    virtual void _Activate(bool fActive) override;
    virtual int32_t _ScvMax(bool fVert) override;
    int32_t _YpFromIvDln(int32_t iv, int32_t dln)
    {
        return _YpFromLn(LwMul(iv, _clnItem) + dln);
    }
    int32_t _LnFromIvDln(int32_t iv, int32_t dln)
    {
        return LwMul(iv, _clnItem) + dln;
    }
    int32_t _LnLim(void)
    {
        return LwMul(_pgrpb->IvMac(), _clnItem);
    }
    void _SetSel(int32_t ln);
    void _ShowSel(void);
    void _DrawSel(PGNV pgnv);
    void _InvalIv(int32_t iv, int32_t cvIns, int32_t cvDel);
    void _EditIvDln(int32_t iv, int32_t dln);
    void _DeleteIv(int32_t iv);

  public:
    static void InvalAllDcgb(PDOCB pdocb, PGRPB pgrpb, int32_t iv, int32_t cvIns, int32_t cvDel);
    virtual bool FCmdKey(PCMD_KEY pcmd) override;
    virtual void MouseDown(int32_t xp, int32_t yp, int32_t cact, uint32_t grfcust) override;

    virtual bool FEnableDcgbCmd(PCMD pcmd, uint32_t *pgrfeds);
    virtual bool FCmdEditItem(PCMD pcmd);
    virtual bool FCmdDeleteItem(PCMD pcmd);
    virtual bool FCmdAddItem(PCMD pcmd) = 0;
};

/***************************************************************************
    Display GL or AL chunk.
***************************************************************************/
#define DCGL_PAR DCGB
#define kclsDCGL KLCONST4('D', 'C', 'G', 'L')
class DCGL : public DCGL_PAR
{
    RTCLASS_DEC

  protected:
    DCGL(PDOCB pdocb, PGLB pglb, int32_t cls, PGCB pgcb);

  public:
    static PDCGL PdcglNew(PDOCB pdocb, PGLB pglb, int32_t cls, PGCB pgcb);

    virtual void Draw(PGNV pgnv, RC *prcClip) override;
    virtual bool FCmdAddItem(PCMD pcmd) override;
};

/***************************************************************************
    Display GG or AG chunk.
***************************************************************************/
#define DCGG_PAR DCGB
#define kclsDCGG KLCONST4('D', 'C', 'G', 'G')
class DCGG : public DCGG_PAR
{
    RTCLASS_DEC

  protected:
    DCGG(PDOCB pdocb, PGGB pggb, int32_t cls, PGCB pgcb);

  public:
    static PDCGG PdcggNew(PDOCB pdocb, PGGB pggb, int32_t cls, PGCB pgcb);

    virtual void Draw(PGNV pgnv, RC *prcClip) override;
    virtual bool FCmdAddItem(PCMD pcmd) override;
};

/***************************************************************************
    Display GST or AST chunk.
***************************************************************************/
#define DCST_PAR DCGB
#define kclsDCST KLCONST4('D', 'C', 'S', 'T')
class DCST : public DCST_PAR
{
    RTCLASS_DEC

  protected:
    DCST(PDOCB pdocb, PGSTB pgstb, int32_t cls, PGCB pgcb);

  public:
    static PDCST PdcstNew(PDOCB pdocb, PGSTB pgstb, int32_t cls, PGCB pgcb);

    virtual void Draw(PGNV pgnv, RC *prcClip) override;
    virtual bool FCmdAddItem(PCMD pcmd) override;
};

/***************************************************************************
    Display a picture chunk.
***************************************************************************/
#define DCPIC_PAR DDG
#define kclsDCPIC KLCONST4('d', 'p', 'i', 'c')
class DCPIC : public DCPIC_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PPIC _ppic;

    DCPIC(PDOCB pdocb, PPIC ppic, PGCB pgcb);
    virtual void GetMinMax(RC *prcMinMax) override;

  public:
    static PDCPIC PdcpicNew(PDOCB pdocb, PPIC ppic, PGCB pgcb);

    virtual void Draw(PGNV pgnv, RC *prcClip) override;
};

/***************************************************************************
    Display a MBMP chunk.
***************************************************************************/
#define DCMBMP_PAR DDG
#define kclsDCMBMP KLCONST4('d', 'm', 'b', 'p')
class DCMBMP : public DCMBMP_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PMBMP _pmbmp;

    DCMBMP(PDOCB pdocb, PMBMP pmbmp, PGCB pgcb);
    virtual void GetMinMax(RC *prcMinMax) override;

  public:
    static PDCMBMP PdcmbmpNew(PDOCB pdocb, PMBMP pmbmp, PGCB pgcb);

    virtual void Draw(PGNV pgnv, RC *prcClip) override;
};

/***************************************************************************
    Main Kidspace world for testing a script.
***************************************************************************/
typedef class TSCG *PTSCG;
#define TSCG_PAR WOKS
#define kclsTSCG KLCONST4('T', 'S', 'C', 'G')
class TSCG : public TSCG_PAR
{
    RTCLASS_DEC
    CMD_MAP_DEC(TSCG)

  public:
    TSCG(PGCB pgcb) : TSCG_PAR(pgcb)
    {
    }

    virtual void Draw(PGNV pgnv, RC *prcClip) override;
};

/***************************************************************************
    Text doc for the chunky editor.
***************************************************************************/
typedef class CHTXD *PCHTXD;
#define CHTXD_PAR TXPD
#define kclsCHTXD KLCONST4('c', 'h', 't', 'x')
class CHTXD : public CHTXD_PAR
{
  protected:
    CHTXD(PDOCB pdocb = pvNil, uint32_t grfdoc = fdocNil);

  public:
    static PCHTXD PchtxdNew(PFNI pfni = pvNil, PBSF pbsf = pvNil, short osk = koskCur, PDOCB pdocb = pvNil,
                            uint32_t grfdoc = fdocNil);

    virtual PDDG PddgNew(PGCB pgcb) override;
};

/***************************************************************************
    Text display gob for the chunky editor.
***************************************************************************/
typedef class CHTDD *PCHTDD;
#define CHTDD_PAR TXLG
#define kclsCHTDD KLCONST4('c', 'h', 't', 'd')
class CHTDD : public CHTDD_PAR
{
    CMD_MAP_DEC(CHTDD)

  protected:
    CHTDD(PTXTB ptxtb, PGCB pgcb, int32_t onn, uint32_t grfont, int32_t dypFont, int32_t cchTab);

  public:
    static PCHTDD PchtddNew(PTXTB ptxtb, PGCB pgcb, int32_t onn, uint32_t grfont, int32_t dypFont, int32_t cchTab);

    virtual bool FCmdCompileChunky(PCMD pcmd);
    virtual bool FCmdCompileScript(PCMD pcmd);
};

void OpenSinkDoc(PMSFIL pmsfil);

#endif //! CHDOC_H
