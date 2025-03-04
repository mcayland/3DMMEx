/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    A base document class and its supporting gob classes.

***************************************************************************/
#ifndef DOCB_H
#define DOCB_H

/***************************************************************************
    base undo class
***************************************************************************/
typedef class UNDB *PUNDB;
#define UNDB_PAR BASE
#define kclsUNDB KLCONST4('U', 'N', 'D', 'B')
class UNDB : public UNDB_PAR
{
    RTCLASS_DEC
    NOCOPY(UNDB)

  protected:
    UNDB(void)
    {
    }

  public:
    // General undo funtionality
    virtual bool FUndo(PDOCB pdocb) = 0;
    virtual bool FDo(PDOCB pdocb) = 0;
};

/***************************************************************************
    base document class
***************************************************************************/
enum
{
    fdocNil = 0,
    fdocSibling = 1,
    fdocForceClose = 2, // for FQueryClose, etc
    fdocAssumeYes = 4,  // for FQueryClose, etc

    fdocUpdate = 8, // update associated DDGs
    fdocInval = 16, // invalidate associated DDGs
};

#define DOCB_PAR CMH
#define kclsDOCB KLCONST4('D', 'O', 'C', 'B')
class DOCB : public DOCB_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

    friend class DTE;

  protected:
    static int32_t _cactLast;
    static PDOCB _pdocbFirst;

    PDOCB _pdocbPar;
    PDOCB _pdocbSib;
    PDOCB _pdocbChd;

    int32_t _cactUntitled; // 0 if titled
    bool _fDirty : 1;
    bool _fFreeing : 1;
    bool _fInternal : 1;
    PGL _pglpddg; // keep track of the DDGs based on this doc

    PGL _pglpundb; // keep track of undo items
    int32_t _ipundbLimDone;
    int32_t _cundbMax;

    bool _FFindDdg(PDDG pddg, int32_t *pipddg);
    virtual tribool _TQuerySave(bool fForce);

    DOCB(PDOCB pdocb = pvNil, uint32_t grfdoc = fdocNil);
    ~DOCB(void);

  public:
    static bool FQueryCloseAll(uint32_t grfdoc);
    static PDOCB PdocbFromFni(FNI *pfni);

    static PDOCB PdocbFirst(void)
    {
        return _pdocbFirst;
    }
    PDOCB PdocbPar(void)
    {
        return _pdocbPar;
    }
    PDOCB PdocbSib(void)
    {
        return _pdocbSib;
    }
    PDOCB PdocbChd(void)
    {
        return _pdocbChd;
    }

    virtual void Release(void);

    // high level call to create a new MDI window based on the doc.
    virtual PDMD PdmdNew(void);
    void ActivateDmd(void);

    // low level calls - generally not for public consumption
    virtual PDMW PdmwNew(PGCB pgcb);
    virtual PDSG PdsgNew(PDMW pdwm, PDSG pdsgSplit, uint32_t grfdsg, int32_t rel);
    virtual PDDG PddgNew(PGCB pgcb);

    // DDG management - only to be called by DDGs
    bool FAddDdg(PDDG pddg);
    void RemoveDdg(PDDG pddg);
    void MakeFirstDdg(PDDG pddg);
    void CloseAllDdg(void);

    // General DDG management
    int32_t Cddg(void)
    {
        return pvNil == _pglpddg ? 0 : _pglpddg->IvMac();
    }
    PDDG PddgGet(int32_t ipddg);
    PDDG PddgActive(void);

    virtual void UpdateName(void);
    virtual void GetName(PSTN pstn);
    virtual bool FQueryClose(uint32_t grfdoc);
    virtual bool FQueryCloseDmd(PDMD pdmd);
    virtual bool FSave(int32_t cid = cidSave);

    virtual bool FGetFni(FNI *pfni);
    virtual bool FGetFniSave(FNI *pfni);
    virtual bool FSaveToFni(FNI *pfni, bool fSetFni);
    virtual bool FDirty(void)
    {
        return _fDirty && !FInternal();
    }
    virtual void SetDirty(bool fDirty = fTrue)
    {
        _fDirty = FPure(fDirty);
    }

    // General undo funtionality
    virtual bool FUndo(void);
    virtual bool FRedo(void);
    virtual bool FAddUndo(PUNDB pundb);
    virtual void ClearUndo(void);
    virtual void ClearRedo(void);
    virtual void SetCundbMax(int32_t cundbMax);
    virtual int32_t CundbMax(void);
    virtual int32_t CundbUndo(void);
    virtual int32_t CundbRedo(void);

    bool FInternal(void);
    void SetAsClipboard(void);
    void SetInternal(bool fInternal = fTrue);

    virtual void ExportFormats(PCLIP pclip);
    virtual bool FGetFormat(int32_t cls, PDOCB *ppdocb = pvNil);
};

/***************************************************************************
    document tree enumerator
***************************************************************************/
enum
{
    // inputs
    fdteNil = 0,
    fdteSkipToSib = 1, // legal to FNextDoc

    // outputs
    fdtePre = 2,
    fdtePost = 4,
    fdteRoot = 8
};

#define DTE_PAR BASE
#define kclsDTE KLCONST3('D', 'T', 'E')
class DTE : public DTE_PAR
{
    RTCLASS_DEC
    ASSERT

  private:
    // enumeration states
    enum
    {
        esStart,
        esGoDown,
        esGoLeft,
        esDone
    };

    int32_t _es;
    PDOCB _pdocbRoot;
    PDOCB _pdocbCur;

  public:
    DTE(void);
    void Init(PDOCB pdocb);
    bool FNextDoc(PDOCB *ppdocb, uint32_t *pgrfdteOut, uint32_t grfdteIn = fdteNil);
};

/***************************************************************************
    document display gob - normally a child of a DSG but can be a child
    of any gob (for doc previewing, etc)
***************************************************************************/
#define DDG_PAR GOB
#define kclsDDG KLCONST3('D', 'D', 'G')
class DDG : public DDG_PAR
{
    RTCLASS_DEC
    CMD_MAP_DEC(DDG)
    ASSERT
    MARKMEM

  protected:
    PDOCB _pdocb;
    bool _fActive;
    int32_t _scvVert; // scroll values
    int32_t _scvHorz;

    DDG(PDOCB pdocb, PGCB pgcb);
    ~DDG(void);

    virtual bool _FInit(void);
    virtual void _Activate(bool fActive);
    virtual void _NewRc(void);

    // scrolling support
    virtual int32_t _ScvMax(bool fVert);
    virtual void _SetScrollValues(void);
    virtual void _Scroll(int32_t scaHorz, int32_t scaVert, int32_t scvHorz = 0, int32_t scvVert = 0);
    virtual void _ScrollDxpDyp(int32_t dxp, int32_t dyp);

    // clipboard support
    virtual bool _FCopySel(PDOCB *ppdocb = pvNil);
    virtual void _ClearSel(void);
    virtual bool _FPaste(PCLIP pclip, bool fDoIt, int32_t cid);

  public:
    static PDDG PddgNew(PDOCB pdocb, PGCB pgcb);

    PDOCB Pdocb(void)
    {
        return _pdocb;
    }
    PDMD Pdmd(void);

    // activation
    virtual void Activate(bool fActive);
    bool FActive(void)
    {
        return _fActive;
    }

    // members of GOB
    virtual void Draw(PGNV pgnv, RC *prcClip);
    virtual bool FCmdActivateSel(PCMD pcmd);

    virtual bool FCmdScroll(PCMD pcmd);
    virtual bool FCmdCloseDoc(PCMD pcmd);
    virtual bool FCmdSave(PCMD pcmd);
    virtual bool FCmdClip(PCMD pcmd);
    virtual bool FEnableDdgCmd(PCMD pcmd, uint32_t *pgrfeds);
    virtual bool FCmdUndo(PCMD pcmd);
};

/***************************************************************************
    Document mdi window - this communicates with the docb to coordinate
    closing and querying the user about saving
***************************************************************************/
#define DMD_PAR GOB
#define kclsDMD KLCONST3('D', 'M', 'D')
class DMD : public DMD_PAR
{
    RTCLASS_DEC

  protected:
    PDOCB _pdocb;

    DMD(PDOCB pdocb, PGCB pgcb);
    virtual void _ActivateHwnd(bool fActive);

  public:
    static PDMD PdmdNew(PDOCB pdocb);
    static PDMD PdmdTop(void);

    PDOCB Pdocb(void)
    {
        return _pdocb;
    }
    virtual void ActivateNext(PDDG pddg);
    virtual bool FCmdCloseWnd(PCMD pcmd);
};

/***************************************************************************
    Document main window
    provides basic pane management - including splitting, etc
***************************************************************************/
#define DMW_PAR GOB
#define kclsDMW KLCONST3('D', 'M', 'W')
class DMW : public DMW_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    // DSG edge struct - these form a locally-balanced binary tree
    // with DSGs as the leafs.  Locally-balanced means that a node has a left
    // child iff it has a right child.
    struct DSED
    {
        bool fVert;  // splits its parent vertically, so the edge is horizontal
        int32_t rel; // where it splits its parent
        RC rcRel;    // current relative rectangle (in the DMW)
        int32_t idsedLeft;
        int32_t idsedRight;
        int32_t idsedPar;
        PDSG pdsg;
    };
    PAL _paldsed; // the tree of DSEDs
    int32_t _idsedRoot;
    PDOCB _pdocb;

    DMW(PDOCB pdocb, PGCB pgcb);

    virtual bool _FInit(void);
    virtual void _NewRc(void);

    void _Layout(int32_t idsedStart);
    int32_t _IdsedNext(int32_t idsed, int32_t idsedRoot);
    int32_t _IdsedEdge(int32_t idsed, int32_t idsedRoot);
    void _RemoveDsg(PDSG pdsg, int32_t *pidsedStartLayout);
    DSED *_Qdsed(int32_t idsed)
    {
        return (DSED *)_paldsed->QvGet(idsed);
    }
    void _SplitRcRel(int32_t idsed, RC *prcLeft, RC *prcRight);

  public:
    static PDMW PdmwNew(PDOCB pdocb, PGCB pgcb);

    PDOCB Pdocb(void)
    {
        return _pdocb;
    }

    bool FAddDsg(PDSG pdsg, PDSG pdsgSplit, uint32_t grfdsg, int32_t rel);
    void RemoveDsg(PDSG pdsg);
    int32_t Cdsg(void);

    void GetRcSplit(PDSG pdsg, RC *prcBounds, RC *prcSplit);
    void MoveSplit(PDSG pdsg, int32_t relNew);
    tribool TVert(PDSG pdsg);

    virtual void Release(void);
};

/***************************************************************************
    document scroll gob - child gob of a DMW
    holds any scroll bars, splitter boxes and split movers
    dialogs tightly with DMW and DDG
***************************************************************************/
#define DSG_PAR GOB
#define kclsDSG KLCONST3('D', 'S', 'G')
class DSG : public DSG_PAR
{
    RTCLASS_DEC
    CMD_MAP_DEC(DSG)
    ASSERT

    friend DMW;

  private:
    int32_t _dsno; // this is how the DMW refers to this DSG
    PDDG _pddg;

  protected:
    DSG(PGCB pgcb);
    ~DSG(void);

    virtual bool _FInit(PDSG pdsgSplit, uint32_t grfdsg, int32_t rel);

  public:
    static PDSG PdsgNew(PDMW pdmw, PDSG pdsgSplit, uint32_t grfdsg, int32_t rel);
    virtual void GetMinMax(RC *prcMinMax);

    PDMW Pdmw(void)
    {
        return (PDMW)PgobPar();
    }

    virtual void Split(uint32_t grfdsg, int32_t rel);
    virtual bool FCmdScroll(PCMD pcmd);
};

enum
{
    fdsgNil = 0,
    fdsgVert = 1, // for splitting and PdsgNew
    fdsgHorz = 2, // for splitting and PdsgNew
    fdsgAfter = 4 // for PdsgNew
};

/***************************************************************************
    document scroll window splitter - must be a child of a DSG
***************************************************************************/
typedef class DSSP *PDSSP;
#define DSSP_PAR GOB
#define kclsDSSP KLCONST4('D', 'S', 'S', 'P')
class DSSP : public DSSP_PAR
{
    RTCLASS_DEC

  protected:
    DSSP(PGCB pgcb);

  public:
    static int32_t DypNormal(void)
    {
        return SCB::DypNormal() / 2;
    }
    static int32_t DxpNormal(void)
    {
        return SCB::DxpNormal() / 2;
    }
    static PDSSP PdsspNew(PDSG pdsg, uint32_t grfdssp);

    virtual void Draw(PGNV pgnv, RC *prcClip);
    virtual void MouseDown(int32_t xp, int32_t yp, int32_t cact, uint32_t grfcust);
};

enum
{
    fdsspNil = 0,
    fdsspVert = 1,
    fdsspHorz = 2
};

/***************************************************************************
    document scroll split mover - must be a child of a DSG
***************************************************************************/
typedef class DSSM *PDSSM;
#define DSSM_PAR GOB
#define kclsDSSM KLCONST4('D', 'S', 'S', 'M')
class DSSM : public DSSM_PAR
{
    RTCLASS_DEC

  private:
    bool _fVert;

  protected:
    DSSM(PGCB pgcb);

    void _DrawTrackBar(PGNV pgnv, RC *prcOld, RC *prcNew);

  public:
    static PDSSM PdssmNew(PDSG pdsg);

    virtual void Draw(PGNV pgnv, RC *prcClip);
    virtual void MouseDown(int32_t xp, int32_t yp, int32_t cact, uint32_t grfcust);
    tribool TVert(void);
};

#endif //! DOCB_H
