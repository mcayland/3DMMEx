/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    For editing a text file or text stream as a document.  Unlike the edit
    controls in text.h/text.cpp, all the text need not be in memory (this
    uses a BSF) and there can be multiple views on the same text.

***************************************************************************/
#ifndef TEXTDOC_H
#define TEXTDOC_H

/***************************************************************************
    Text document.  A doc wrapper for a BSF.
***************************************************************************/
typedef class TXDC *PTXDC;
#define TXDC_PAR DOCB
#define kclsTXDC 'TXDC'
class TXDC : public TXDC_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PBSF _pbsf;
    PFIL _pfil;

    TXDC(PDOCB pdocb = pvNil, uint32_t grfdoc = fdocNil);
    ~TXDC(void);
    bool _FInit(PFNI pfni = pvNil, PBSF pbsf = pvNil);

  public:
    static PTXDC PtxdcNew(PFNI pfni = pvNil, PBSF pbsf = pvNil, PDOCB pdocb = pvNil, uint32_t grfdoc = fdocNil);

    PBSF Pbsf(void)
    {
        return _pbsf;
    }

    virtual PDDG PddgNew(PGCB pgcb) override;
    virtual bool FGetFni(FNI *pfni) override;
    virtual bool FSaveToFni(FNI *pfni, bool fSetFni) override;
};

/***************************************************************************
    Text document display GOB - DDG for a TXDC.
***************************************************************************/
const int32_t kcchMaxLine = 512;
const int32_t kdxpIndentTxdd = 5;

typedef class TXDD *PTXDD;
#define TXDD_PAR DDG
#define kclsTXDD 'TXDD'
class TXDD : public TXDD_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PBSF _pbsf;
    int32_t _clnDisp;
    int32_t _clnDispWhole;
    PGL _pglichStarts;

    // the selection
    int32_t _ichAnchor;
    int32_t _ichOther;
    bool _fSelOn : 1;
    uint32_t _tsSel;
    int32_t _xpSel;
    bool _fXpValid;

    // the font
    int32_t _onn;
    uint32_t _grfont;
    int32_t _dypFont;
    int32_t _dypLine;
    int32_t _dxpTab;

    // the cache
    achar _rgchCache[kcchMaxLine];
    int32_t _ichMinCache;
    int32_t _ichLimCache;

    TXDD(PDOCB pdocb, PGCB pgcb, PBSF pbsf, int32_t onn, uint32_t grfont, int32_t dypFont);
    ~TXDD(void);
    virtual bool _FInit(void) override;
    virtual void _NewRc(void) override;
    virtual void _Activate(bool fActive) override;

    void _Reformat(int32_t lnMin, int32_t *pclnIns = pvNil, int32_t *pclnDel = pvNil);
    void _ReformatEdit(int32_t ichMinEdit, int32_t cchIns, int32_t cchDel, int32_t *pln, int32_t *pclnIns = pvNil,
                       int32_t *pclnDel = pvNil);
    bool _FFetchCh(int32_t ich, achar *pch);
    void _FetchLineLn(int32_t ln, achar *prgch, int32_t cchMax, int32_t *pcch, int32_t *pichMin = pvNil);
    void _FetchLineIch(int32_t ich, achar *prgch, int32_t cchMax, int32_t *pcch, int32_t *pichMin = pvNil);
    bool _FFindNextLineStart(int32_t ich, int32_t *pich, achar *prgch = pvNil, int32_t cchMax = 0);
    bool _FFindLineStart(int32_t ich, int32_t *pich);
    bool _FFindNextLineStartCached(int32_t ich, int32_t *pich, achar *prgch = pvNil, int32_t cchMax = 0);
    bool _FFindLineStartCached(int32_t ich, int32_t *pich);
    void _DrawLine(PGNV pgnv, RC *prcClip, int32_t yp, achar *prgch, int32_t cch);
    void _SwitchSel(bool fOn, bool fDraw);
    void _InvertSel(PGNV pgnv, bool fDraw);
    void _InvertIchRange(PGNV pgnv, int32_t ich1, int32_t ich2, bool fDraw);
    int32_t _LnFromIch(int32_t ich);
    int32_t _IchMinLn(int32_t ln);
    int32_t _XpFromLnIch(PGNV pgnv, int32_t ln, int32_t ich);
    int32_t _XpFromIch(int32_t ich);
    int32_t _XpFromRgch(PGNV pgnv, achar *prgch, int32_t cch);
    int32_t _IchFromLnXp(int32_t ln, int32_t xp);
    int32_t _IchFromIchXp(int32_t ich, int32_t xp);
    int32_t _IchFromRgchXp(achar *prgch, int32_t cch, int32_t ichMinLine, int32_t xp);

    int32_t *_QichLn(int32_t ln)
    {
        return (int32_t *)_pglichStarts->QvGet(ln);
    }

    void _InvalAllTxdd(int32_t ich, int32_t cchIns, int32_t cchDel);
    void _InvalIch(int32_t ich, int32_t cchIns, int32_t cchDel);

    // scrolling support
    virtual int32_t _ScvMax(bool fVert) override;
    virtual void _Scroll(int32_t scaHorz, int32_t scaVert, int32_t scvHorz = 0, int32_t scvVert = 0) override;

    // clipboard support
    virtual bool _FCopySel(PDOCB *ppdocb = pvNil) override;
    virtual void _ClearSel(void) override;
    virtual bool _FPaste(PCLIP pclip, bool fDoIt, int32_t cid) override;

  public:
    static PTXDD PtxddNew(PDOCB pdocb, PGCB pgcb, PBSF pbsf, int32_t onn, uint32_t grfont, int32_t dypFont);

    virtual void Draw(PGNV pgnv, RC *prcClip) override;
    virtual bool FCmdTrackMouse(PCMD_MOUSE pcmd) override;
    virtual bool FCmdKey(PCMD_KEY pcmd) override;
    virtual bool FCmdSelIdle(PCMD pcmd) override;

    void SetSel(int32_t ichAnchor, int32_t ichOther, bool fDraw);
    void ShowSel(bool fDraw);
    bool FReplace(achar *prgch, int32_t cch, int32_t ich1, int32_t ich2, bool fDraw);
};

#endif //! TEXTDOC_H
