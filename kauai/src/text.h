/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Header file for edit controls and rich text editing.

***************************************************************************/
#ifndef TEXT_H
#define TEXT_H

// edit control parameters
typedef class EDPAR *PEDPAR;
class EDPAR
{
  public:
    GCB _gcb;
    int32_t _onn;
    uint32_t _grfont;
    int32_t _dypFont;
    int32_t _tah;
    int32_t _tav;
    ACR _acrFore;
    ACR _acrBack;
    int32_t _cmhl;

    EDPAR(void)
    {
    }
    EDPAR(int32_t hid, PGOB pgob, uint32_t grfgob, int32_t gin, RC *prcAbs, RC *prcRel, int32_t onn, uint32_t grfont,
          int32_t dypFont, int32_t tah = tahLeft, int32_t tav = tavTop, ACR acrFore = kacrBlack,
          ACR acrBack = kacrWhite, int32_t cmhl = 0);

    void Set(int32_t hid, PGOB pgob, uint32_t grfgob, int32_t gin, RC *prcAbs, RC *prcRel, int32_t onn, uint32_t grfont,
             int32_t dypFont, int32_t tah = tahLeft, int32_t tav = tavTop, ACR acrFore = kacrBlack,
             ACR acrBack = kacrWhite, int32_t cmhl = 0);
    void SetFont(int32_t onn, uint32_t grfont, int32_t dypFont, int32_t tah = tahLeft, int32_t tav = tavTop,
                 ACR acrFore = kacrBlack, ACR acrBack = kacrWhite);
};

/***************************************************************************
    Edit control base class.
***************************************************************************/
typedef class EDCB *PEDCB;
#define EDCB_PAR GOB
#define kclsEDCB KLCONST4('E', 'D', 'C', 'B')
class EDCB : public EDCB_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    // handler level
    int32_t _cmhl;

    // the selection
    PGNV _pgnv;
    int32_t _ichAnchor;
    int32_t _ichOther;
    bool _fSelOn : 1;
    bool _fXpValid : 1;
    bool _fSelByWord : 1;
    bool _fMark : 1;
    bool _fClear : 1;

    uint32_t _tsSel;
    int32_t _xpSel; // for avoiding migration when changing selection by lines

    // the origin
    int32_t _xp;
    int32_t _yp;

    EDCB(PGCB pgcb, int32_t cmhl);

    virtual bool _FInit(void);

    // pure virtual functions
    virtual int32_t _LnFromIch(int32_t ich) = 0;
    virtual int32_t _IchMinLn(int32_t ln) = 0;
    virtual int32_t _XpFromIch(int32_t ich) = 0;
    virtual int32_t _YpFromIch(int32_t ich);
    virtual int32_t _YpFromLn(int32_t ln) = 0;
    virtual int32_t _LnFromYp(int32_t yp) = 0;
    virtual int32_t _IchFromLnXp(int32_t ln, int32_t xp, bool fClosest = fTrue) = 0;
    virtual int32_t _LnMac(void) = 0;
    virtual void _DrawLine(PGNV pgnv, int32_t ln) = 0;
    virtual void _HiliteRc(PGNV pgnv, RC *prc) = 0;
    virtual bool _FFilterCh(achar ch) = 0;

    void _SwitchSel(bool fOn, int32_t gin = kginDraw);
    void _InvertSel(PGNV pgnv, int32_t gin = kginDraw);
    void _InvertIchRange(PGNV pgnv, int32_t ich1, int32_t ich2, int32_t gin = kginDraw);
    void _Scroll(int32_t dxp, int32_t dyp, int32_t gin = kginDraw);
    void _UpdateLn(int32_t ln, int32_t clnIns, int32_t dlnDel, int32_t dypDel, int32_t gin = kginDraw);
    int32_t _IchPrev(int32_t ich, bool fWord = fFalse);
    int32_t _IchNext(int32_t ich, bool fWord = fFalse);
    achar _ChFetch(int32_t ich);

    virtual void _NewRc(void);

    virtual void _GetRcContent(RC *prc);
    virtual void _InitGnv(PGNV pgnv);

  public:
    ~EDCB(void);

    virtual void Draw(PGNV pgnv, RC *prcClip);
    virtual bool FCmdTrackMouse(PCMD_MOUSE pcmd);
    virtual bool FCmdKey(PCMD_KEY pcmd);
    virtual bool FCmdSelIdle(PCMD pcmd);
    virtual bool FCmdActivateSel(PCMD pcmd);
    virtual void Activate(bool fActive);

    int32_t IchAnchor(void)
    {
        return _ichAnchor;
    }
    int32_t IchOther(void)
    {
        return _ichOther;
    }
    void SetSel(int32_t ichAnchor, int32_t ichOther, int32_t gin = kginDraw);
    void ShowSel(bool fForceJustification = fTrue, int32_t gin = kginDraw);

    virtual int32_t IchMac(void) = 0;
    virtual bool FReplace(const achar *prgch, int32_t cchIns, int32_t ich1, int32_t ich2, int32_t gin = kginDraw) = 0;
    virtual int32_t CchFetch(achar *prgch, int32_t ich, int32_t cchWant) = 0;
};

/***************************************************************************
    Plain edit control - virtual class supporting single line and multi
    line edit controls with a single font.
***************************************************************************/
typedef class EDPL *PEDPL;
#define EDPL_PAR EDCB
#define kclsEDPL KLCONST4('E', 'D', 'P', 'L')
class EDPL : public EDPL_PAR
{
    RTCLASS_DEC
    ASSERT

  protected:
    // drawing parameters
    int32_t _onn;
    uint32_t _grfont;
    int32_t _dypFont;
    int32_t _tah;
    int32_t _tav;
    ACR _acrFore;
    ACR _acrBack;
    int32_t _dypLine;

    EDPL(PEDPAR pedpar);

    // methods of EDCB
    virtual bool _FInit(void);
    virtual int32_t _XpFromIch(int32_t ich);
    virtual int32_t _YpFromLn(int32_t ln);
    virtual int32_t _LnFromYp(int32_t yp);
    virtual int32_t _IchFromLnXp(int32_t ln, int32_t xp, bool fClosest = fTrue);
    virtual void _DrawLine(PGNV pgnv, int32_t ln);
    virtual void _HiliteRc(PGNV pgnv, RC *prc);

    int32_t _XpOrigin(void);
    virtual bool _FLockLn(int32_t ln, achar **pprgch, int32_t *pcch) = 0;
    virtual void _UnlockLn(int32_t ln, achar *prgch) = 0;
};

/***************************************************************************
    Single line edit control.
***************************************************************************/
const int32_t kcchMaxEdsl = kcchMaxStn;

typedef class EDSL *PEDSL;
#define EDSL_PAR EDPL
#define kclsEDSL KLCONST4('E', 'D', 'S', 'L')
class EDSL : public EDSL_PAR
{
    RTCLASS_DEC
    ASSERT

  protected:
    // the text
    int32_t _cch;
    achar _rgch[kcchMaxEdsl];

    EDSL(PEDPAR pedpar);

    // methods of EDCB
    virtual int32_t _LnFromIch(int32_t ich);
    virtual int32_t _IchMinLn(int32_t ln);
    virtual int32_t _LnMac(void);
    virtual bool _FFilterCh(achar ch);

    // methods of EDPL
    virtual bool _FLockLn(int32_t ln, achar **pprgch, int32_t *pcch);
    virtual void _UnlockLn(int32_t ln, achar *prgch);

  public:
    static PEDSL PedslNew(PEDPAR pedpar);

    virtual int32_t IchMac(void);
    virtual bool FReplace(const achar *prgch, int32_t cchIns, int32_t ich1, int32_t ich2, int32_t gin = kginDraw);
    virtual int32_t CchFetch(achar *prgch, int32_t ich, int32_t cchWant);

    // additional text APIs
    void GetStn(PSTN pstn);
    void SetStn(PSTN pstn, int32_t gin = kginDraw);
};

/***************************************************************************
    Multi line edit control.
***************************************************************************/
typedef class EDML *PEDML;
#define EDML_PAR EDPL
#define kclsEDML KLCONST4('E', 'D', 'M', 'L')
class EDML : public EDML_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    // the text
    BSM _bsm;
    PGL _pglich;

    EDML(PEDPAR pedpar);

    // methods of EDCB
    virtual bool _FInit(void);
    virtual int32_t _LnFromIch(int32_t ich);
    virtual int32_t _IchMinLn(int32_t ln);
    virtual int32_t _LnMac(void);
    virtual bool _FFilterCh(achar ch);

    // methods of EDPL
    virtual bool _FLockLn(int32_t ln, achar **pprgch, int32_t *pcch);
    virtual void _UnlockLn(int32_t ln, achar *prgch);

    virtual int32_t _ClnEstimate(const achar *prgch, int32_t cch);
    virtual int32_t _LnReformat(int32_t lnMin, int32_t *pclnDel, int32_t *pclnIns);
    virtual bool _FReplaceCore(const achar *prgch, int32_t cchIns, int32_t ich, int32_t cchDel);

  public:
    static PEDML PedmlNew(PEDPAR pedpar);
    ~EDML(void);

    virtual int32_t IchMac(void);
    virtual bool FReplace(const achar *prgch, int32_t cchIns, int32_t ich1, int32_t ich2, int32_t gin = kginDraw);
    virtual int32_t CchFetch(achar *prgch, int32_t ich, int32_t cchWant);
};

/***************************************************************************
    Multi line wrapping edit control.
***************************************************************************/
typedef class EDMW *PEDMW;
#define EDMW_PAR EDML
#define kclsEDMW KLCONST4('E', 'D', 'M', 'W')
class EDMW : public EDMW_PAR
{
    RTCLASS_DEC

  protected:
    EDMW(PEDPAR pedpar);

    // methods EDMW
    virtual int32_t _ClnEstimate(achar *prgch, int32_t cch);
    virtual int32_t _LnReformat(int32_t lnMin, int32_t *pclnDel, int32_t *pclnIns);

    int32_t _CichGetBreakables(achar *prgch, int32_t ich, int32_t *prgich, int32_t cichMax);
    virtual void _NewRc(void);

  public:
    static PEDMW PedmwNew(PEDPAR pedpar);
};

#endif //! TEXT_H
