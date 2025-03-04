/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Code for implementing help balloons in kidspace.

***************************************************************************/
#ifndef KIDHELP_H
#define KIDHELP_H

/***************************************************************************
    Help topic construction information.
***************************************************************************/
struct HTOP
{
    CNO cnoBalloon;
    int32_t hidThis;
    int32_t hidTarget;
    CNO cnoScript;
    int32_t dxp;
    int32_t dyp;
    CKI ckiSnd;
};
VERIFY_STRUCT_SIZE(HTOP, 32);
typedef HTOP *PHTOP;
const BOM kbomHtop = 0xFFF00000;

// help topic on file
struct HTOPF
{
    int16_t bo;
    int16_t osk;
    HTOP htop;
};

// edit control object
struct ECOS
{
    CTG ctg;     // kctgEditControl
    int32_t dxp; // width
};

/***************************************************************************
    Help text document
***************************************************************************/
enum
{
    ftxhdNil = 0,
    ftxhdCopyText = 1,
    ftxhdExpandStrings = 2,
};

typedef class TXHD *PTXHD;
#define TXHD_PAR TXRD
#define kclsTXHD KLCONST4('T', 'X', 'H', 'D')
class TXHD : public TXHD_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    enum
    {
        sprmGroup = 64, // grouped (hot) text - uses the AG
    };

    PRCA _prca;         // source of pictures and buttons
    HTOP _htop;         // our gob creation information
    bool _fHideButtons; // whether to draw buttons

    TXHD(PRCA prca, PDOCB pdocb = pvNil, uint32_t grfdoc = fdocNil);
    ~TXHD(void);

    virtual bool _FReadChunk(PCFL pcfl, CTG ctg, CNO cno, PSTRG pstrg = pvNil, uint32_t grftxhd = ftxhdNil);
    virtual bool _FOpenArg(int32_t icact, uint8_t sprm, int16_t bo, int16_t osk);
    virtual bool _FGetObjectRc(int32_t icact, uint8_t sprm, PGNV pgnv, PCHP pchp, RC *prc);
    virtual bool _FDrawObject(int32_t icact, uint8_t sprm, PGNV pgnv, int32_t *pxp, int32_t yp, PCHP pchp, RC *prcClip);

  public:
    static PTXHD PtxhdReadChunk(PRCA prca, PCFL pcfl, CTG ctg, CNO cno, PSTRG pstrg = pvNil,
                                uint32_t grftxhd = ftxhdExpandStrings);

    virtual bool FSaveToChunk(PCFL pcfl, CKI *pcki, bool fRedirectText = fFalse);

    bool FInsertPicture(CNO cno, void *pvExtra, int32_t cbExtra, int32_t cp, int32_t ccpDel, PCHP pchp = pvNil,
                        uint32_t grfdoc = fdocUpdate);
    bool FInsertButton(CNO cno, CNO cnoTopic, void *pvExtra, int32_t cbExtra, int32_t cp, int32_t ccpDel,
                       PCHP pchp = pvNil, uint32_t grfdoc = fdocUpdate);
    PRCA Prca(void)
    {
        return _prca;
    }
    bool FGroupText(int32_t cp1, int32_t cp2, uint8_t bGroup, CNO cnoTopic = cnoNil, PSTN pstnTopic = pvNil);
    bool FGrouped(int32_t cp, int32_t *pcpMin = pvNil, int32_t *pcpLim = pvNil, uint8_t *pbGroup = pvNil,
                  CNO *pcnoTopic = pvNil, PSTN pstnTopic = pvNil);

    void GetHtop(PHTOP phtop);
    void SetHtop(PHTOP phtop);
    void HideButtons(bool fHide = fTrue)
    {
        _fHideButtons = FPure(fHide);
    }
};

/***************************************************************************
    A runtime DDG for a help topic.
***************************************************************************/
typedef class TXHG *PTXHG;
#define TXHG_PAR TXRG
#define kclsTXHG KLCONST4('T', 'X', 'H', 'G')
class TXHG : public TXHG_PAR
{
    RTCLASS_DEC
    CMD_MAP_DEC(TXHG)

  protected:
    uint8_t _bTrack;
    CNO _cnoTrack;
    int32_t _hidBase;
    uint32_t _grfcust;
    PWOKS _pwoks;

    TXHG(PWOKS pwoks, PTXHD ptxhd, PGCB pgcb);
    virtual bool _FInit(void);
    virtual bool _FRunScript(uint8_t bGroup, uint32_t grfcust, int32_t hidHit, achar ch, CNO cnoTopic = cnoNil,
                             int32_t *plwRet = pvNil);

  public:
    static PTXHG PtxhgNew(PWOKS pwoks, PTXHD ptxhd, PGCB pgcb);

    PTXHD Ptxhd(void)
    {
        return (PTXHD)_ptxtb;
    }
    virtual bool FPtIn(int32_t xp, int32_t yp);
    virtual bool FCmdTrackMouse(PCMD_MOUSE pcmd);
    virtual bool FCmdMouseMove(PCMD_MOUSE pcmd);
    virtual bool FCmdBadKey(PCMD_BADKEY pcmd);
    virtual bool FGroupFromPt(int32_t xp, int32_t yp, uint8_t *pbGroup = pvNil, CNO *pcnoTopic = pvNil);
    virtual void DoHit(uint8_t bGroup, CNO cnoTopic, uint32_t grfcust, int32_t hidHit);
    virtual void SetCursor(uint32_t grfcust);
};

/***************************************************************************
    Help balloon.
***************************************************************************/
typedef class HBAL *PHBAL;
#define HBAL_PAR GOK
#define kclsHBAL KLCONST4('H', 'B', 'A', 'L')
class HBAL : public HBAL_PAR
{
    RTCLASS_DEC

  protected:
    PTXHG _ptxhg;

    HBAL(GCB *pgcb);
    virtual void _SetGorp(PGORP pgorp, int32_t dxp, int32_t dyp);
    virtual bool _FInit(PWOKS pwoks, PTXHD ptxhd, HTOP *phtop, PRCA prca);
    virtual bool _FSetTopic(PTXHD ptxhd, PHTOP phtop, PRCA prca);

  public:
    static PHBAL PhbalCreate(PWOKS pwoks, PGOB pgobPar, PRCA prca, CNO cnoTopic, PHTOP phtop = pvNil);
    static PHBAL PhbalNew(PWOKS pwoks, PGOB pgobPar, PRCA prca, PTXHD ptxhd, PHTOP phtop = pvNil);

    virtual bool FSetTopic(PTXHD ptxhd, PHTOP phtop, PRCA prca);
};

/***************************************************************************
    Help balloon button.
***************************************************************************/
typedef class HBTN *PHBTN;
#define HBTN_PAR GOK
#define kclsHBTN KLCONST4('H', 'B', 'T', 'N')
class HBTN : public HBTN_PAR
{
    RTCLASS_DEC

  protected:
    HBTN(GCB *pgcb);

    uint8_t _bGroup;
    CNO _cnoTopic;

  public:
    static PHBTN PhbtnNew(PWOKS pwoks, PGOB pgobPar, int32_t hid, CNO cno, PRCA prca, uint8_t bGroup, CNO cnoTopic,
                          int32_t xpLeft, int32_t ypBottom);

    virtual bool FPtIn(int32_t xp, int32_t yp);
    virtual bool FCmdClicked(PCMD_MOUSE pcmd);
};

#endif //! KIDHELP_H
