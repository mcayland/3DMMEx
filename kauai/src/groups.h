/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Basic collection classes:
        General List (GL), Allocated List (AL),
        General Group (GG), Allocated Group (AG),
        General String Table (GST), Allocated String Table (AST).

        BASE ---> GRPB -+-> GLB -+-> GL
                        |        +-> AL
                        |
                        +-> GGB -+-> GG
                        |        +-> AG
                        |
                        +-> GSTB-+-> GST
                                 +-> AST

***************************************************************************/
#ifndef GROUPS_H
#define GROUPS_H

enum
{
    fgrpNil = 0,
    fgrpShrink = 1,
};

/****************************************
    GRPB is a virtual class supporting
    all group classes
****************************************/
#define GRPB_PAR BASE
#define kclsGRPB 'GRPB'
class GRPB : public GRPB_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  private:
    int32_t _cb1;
    int32_t _cb2;
    HQ _hqData1;
    HQ _hqData2;

    bool _FEnsureHqCb(HQ *phq, int32_t cb, int32_t cbMinGrow, int32_t *pcb);

  protected:
    int32_t _cbMinGrow1;
    int32_t _cbMinGrow2;
    int32_t _ivMac;

    uint8_t *_Qb1(int32_t ib)
    {
        return (uint8_t *)QvFromHq(_hqData1) + ib;
    }
    uint8_t *_Qb2(int32_t ib)
    {
        return (uint8_t *)QvFromHq(_hqData2) + ib;
    }
    int32_t _Cb1(void)
    {
        return _cb1;
    }
    int32_t _Cb2(void)
    {
        return _cb2;
    }
    bool _FEnsureSizes(int32_t cbMin1, int32_t cbMin2, uint32_t grfgrp);
    bool _FWrite(PBLCK pblck, void *pv, int32_t cb, int32_t cb1, int32_t cb2);
    bool _FReadData(PBLCK pblck, int32_t ib, int32_t cb1, int32_t cb2);
    bool _FDup(PGRPB pgrpbDst, int32_t cb1, int32_t cb2);

    GRPB(void)
    {
    }

  public:
    ~GRPB(void);

    void Lock(void)
    {
        if (_hqData1 != hqNil)
            PvLockHq(_hqData1);
    }
    void Unlock(void)
    {
        if (_hqData1 != hqNil)
            UnlockHq(_hqData1);
    }
    int32_t IvMac(void)
    {
        return _ivMac;
    }
    virtual bool FFree(int32_t iv) = 0;
    virtual void Delete(int32_t iv) = 0;

    // writing
    virtual bool FWriteFlo(PFLO pflo, int16_t bo = kboCur, int16_t osk = koskCur);
    virtual bool FWrite(PBLCK pblck, int16_t bo = kboCur, int16_t osk = koskCur) = 0;
    virtual int32_t CbOnFile(void) = 0;
};

/****************************************
    GLB is a virtual class supporting
    GL and AL
****************************************/
#define GLB_PAR GRPB
#define kclsGLB 'GLB'
class GLB : public GLB_PAR
{
    RTCLASS_DEC
    ASSERT

  protected:
    int32_t _cbEntry;

    GLB(int32_t cb);

  public:
    int32_t CbEntry(void)
    {
        return _cbEntry;
    }
    void *QvGet(int32_t iv);
    void Get(int32_t iv, void *pv);
    void Put(int32_t iv, void *pv);
    void *PvLock(int32_t iv);
    void SetMinGrow(int32_t cvAdd);

    virtual bool FAdd(void *pv, int32_t *piv = pvNil) = 0;
};

/****************************************
    GL is the basic dynamic array
****************************************/
#define GL_PAR GLB
#define kclsGL 'GL'
class GL : public GL_PAR
{
    RTCLASS_DEC

  protected:
    GL(int32_t cb);
    bool _FRead(PBLCK pblck, int16_t *pbo, int16_t *posk);

  public:
    // static methods
    static PGL PglNew(int32_t cb, int32_t cvInit = 0);
    static PGL PglRead(PBLCK pblck, int16_t *pbo = pvNil, int16_t *posk = pvNil);
    static PGL PglRead(PFIL pfil, FP fp, int32_t cb, int16_t *pbo = pvNil, int16_t *posk = pvNil);

    // duplication
    PGL PglDup(void);

    // methods required by parent class
    virtual bool FAdd(void *pv, int32_t *piv = pvNil) override;
    virtual void Delete(int32_t iv) override;
    virtual bool FWrite(PBLCK pblck, int16_t bo = kboCur, int16_t osk = koskCur) override;
    virtual int32_t CbOnFile(void) override;
    virtual bool FFree(int32_t iv) override;

    // new methods
    void Delete(int32_t iv, int32_t cv);
    bool FInsert(int32_t iv, void *pv = pvNil, int32_t cv = 1);
    bool FSetIvMac(int32_t ivMacNew);
    bool FEnsureSpace(int32_t cvAdd, uint32_t grfgrp = fgrpNil);
    void Move(int32_t ivSrc, int32_t ivTarget);
    bool FPush(void *pv)
    {
        return FInsert(_ivMac, pv);
    }
    bool FPop(void *pv = pvNil);
    bool FEnqueue(void *pv)
    {
        return FInsert(0, pv);
    }
    bool FDequeue(void *pv = pvNil)
    {
        return FPop(pv);
    }
};

/****************************************
    Allocated (fixed index) list class
****************************************/
#define AL_PAR GLB
#define kclsAL 'AL'
class AL : public AL_PAR
{
    RTCLASS_DEC
    ASSERT

  private:
    int32_t _cvFree;

  private:
    // section 2 of the data contains a bit array
    uint8_t *_Qgrfbit(int32_t iv)
    {
        return _Qb2(IbFromIbit(iv));
    }

  protected:
    AL(int32_t cb);
    bool _FRead(PBLCK pblck, int16_t *pbo, int16_t *posk);

  public:
    // static methods
    static PAL PalNew(int32_t cb, int32_t cvInit = 0);
    static PAL PalRead(PBLCK pblck, int16_t *pbo = pvNil, int16_t *posk = pvNil);
    static PAL PalRead(PFIL pfil, FP fp, int32_t cb, int16_t *pbo = pvNil, int16_t *posk = pvNil);

    // duplication
    PAL PalDup(void);

    // methods required by parent class
    virtual bool FAdd(void *pv, int32_t *piv = pvNil) override;
    virtual void Delete(int32_t iv) override;
    virtual bool FWrite(PBLCK pblck, int16_t bo = kboCur, int16_t osk = koskCur) override;
    virtual int32_t CbOnFile(void) override;
    virtual bool FFree(int32_t iv) override;

    // new methods
    bool FEnsureSpace(int32_t cvAdd, uint32_t grfgrp = fgrpNil);
    void DeleteAll(void);
};

/****************************************
    GGB is a virtual class supporting
    GG and AG
****************************************/
const BOM kbomLoc = 0xF0000000;
#define GGB_PAR GRPB
#define kclsGGB 'GGB'
class GGB : public GGB_PAR
{
    RTCLASS_DEC
    ASSERT

  protected:
    struct LOC
    {
        int32_t bv;
        int32_t cb;
    };

    int32_t _bvMac;
    int32_t _clocFree;
    int32_t _cbFixed;

  protected:
    GGB(int32_t cbFixed, bool fAllowFree);

    void _RemoveRgb(int32_t bv, int32_t cb);
    void _AdjustLocs(int32_t bvMin, int32_t bvLim, int32_t dcb);
    LOC *_Qloc(int32_t iloc)
    {
        return (LOC *)_Qb2(LwMul(iloc, SIZEOF(LOC)));
    }
    bool _FRead(PBLCK pblck, int16_t *pbo, int16_t *posk);

    bool _FDup(PGGB pggbDst);

  public:
    // methods required by parent class
    virtual bool FWrite(PBLCK pblck, int16_t bo = kboCur, int16_t osk = koskCur) override;
    virtual int32_t CbOnFile(void) override;
    virtual bool FFree(int32_t iv) override;

    bool FEnsureSpace(int32_t cvAdd, int32_t cbAdd, uint32_t grfgrp = fgrpNil);
    void SetMinGrow(int32_t cvAdd, int32_t cbAdd);

    virtual bool FAdd(int32_t cb, int32_t *piv = pvNil, const void *pv = pvNil, void *pvFixed = pvNil) = 0;

    // access to the fixed portion
    int32_t CbFixed(void)
    {
        return _cbFixed;
    }
    void *QvFixedGet(int32_t iv, int32_t *pcbVar = pvNil);
    void *PvFixedLock(int32_t iv, int32_t *pcbVar = pvNil);
    void GetFixed(int32_t iv, void *pv);
    void PutFixed(int32_t iv, void *pv);

    // access to the variable portion
    int32_t Cb(int32_t iv);
    void *QvGet(int32_t iv, int32_t *pcb = pvNil);
    void *PvLock(int32_t iv, int32_t *pcb = pvNil);
    void Get(int32_t iv, void *pv);
    void Put(int32_t iv, void *pv);
    bool FPut(int32_t iv, int32_t cb, void *pv);
    void GetRgb(int32_t iv, int32_t bv, int32_t cb, void *pv);
    void PutRgb(int32_t iv, int32_t bv, int32_t cb, void *pv);
    void DeleteRgb(int32_t iv, int32_t bv, int32_t cb);
    bool FInsertRgb(int32_t iv, int32_t bv, int32_t cb, const void *pv);
    bool FMoveRgb(int32_t ivSrc, int32_t bvSrc, int32_t ivDst, int32_t bvDst, int32_t cb);
    void Merge(int32_t ivSrc, int32_t ivDst);
};

/****************************************
    General Group - based on GGB
****************************************/
#define GG_PAR GGB
#define kclsGG 'GG'
class GG : public GG_PAR
{
    RTCLASS_DEC
    ASSERT

  protected:
    GG(int32_t cbFixed) : GGB(cbFixed, fFalse)
    {
    }

  public:
    // static methods
    static PGG PggNew(int32_t cbFixed = 0, int32_t cvInit = 0, int32_t cbInit = 0);
    static PGG PggRead(PBLCK pblck, int16_t *pbo = pvNil, int16_t *posk = pvNil);
    static PGG PggRead(PFIL pfil, FP fp, int32_t cb, int16_t *pbo = pvNil, int16_t *posk = pvNil);

    // duplication
    PGG PggDup(void);

    // methods required by parent class
    virtual bool FAdd(int32_t cb, int32_t *piv = pvNil, const void *pv = pvNil, void *pvFixed = pvNil) override;
    virtual void Delete(int32_t iv) override;

    // new methods
    bool FInsert(int32_t iv, int32_t cb, const void *pv = pvNil, const void *pvFixed = pvNil);
    bool FCopyEntries(PGG pggSrc, int32_t ivSrc, int32_t ivDst, int32_t cv);
    void Move(int32_t ivSrc, int32_t ivTarget);
    void Swap(int32_t iv1, int32_t iv2);
};

/****************************************
    Allocated Group - based on GGB
****************************************/
#define AG_PAR GGB
#define kclsAG 'AG'
class AG : public AG_PAR
{
    RTCLASS_DEC
    ASSERT

  protected:
    AG(int32_t cbFixed) : GGB(cbFixed, fTrue)
    {
    }

  public:
    // static methods
    static PAG PagNew(int32_t cbFixed = 0, int32_t cvInit = 0, int32_t cbInit = 0);
    static PAG PagRead(PBLCK pblck, int16_t *pbo = pvNil, int16_t *posk = pvNil);
    static PAG PagRead(PFIL pfil, FP fp, int32_t cb, int16_t *pbo = pvNil, int16_t *posk = pvNil);

    // duplication
    PAG PagDup(void);

    // methods required by parent class
    virtual bool FAdd(int32_t cb, int32_t *piv = pvNil, const void *pv = pvNil, void *pvFixed = pvNil) override;
    virtual void Delete(int32_t iv) override;
};

/****************************************
    String table classes
****************************************/
enum
{
    fgstNil = 0,
    fgstSorted = 1,
    fgstUserSorted = 2,
    fgstAllowFree = 4,
};

const int32_t kcchMaxGst = kcchMaxStn;

/****************************************
    GSTB is a virtual class supporting
    GST and AST.
****************************************/
#define GSTB_PAR GRPB
#define kclsGSTB 'GSTB'
class GSTB : public GSTB_PAR
{
    RTCLASS_DEC
    ASSERT

  protected:
    int32_t _cbEntry;
    int32_t _bstMac;
    int32_t _cbstFree; // this is cvNil for non-allocated GSTBs

  protected:
    GSTB(int32_t cbExtra, uint32_t grfgst);

    int32_t _Bst(int32_t ibst)
    {
        return *(int32_t *)_Qb2(LwMul(ibst, _cbEntry));
    }
    int32_t *_Qbst(int32_t ibst)
    {
        return (int32_t *)_Qb2(LwMul(ibst, _cbEntry));
    }
    PST _Qst(int32_t ibst);
    void _RemoveSt(int32_t bst);
    void _AppendRgch(const achar *prgch, int32_t cch);
    void _SwapBytesRgbst(void);
    void _TranslateGrst(int16_t osk, bool fToCur);
    bool _FTranslateGrst(int16_t osk);
    bool _FRead(PBLCK pblck, int16_t *pbo, int16_t *posk);

    bool _FDup(PGSTB pgstbDst);

  public:
    // methods required by parent class
    virtual bool FWrite(PBLCK pblck, int16_t bo = kboCur, int16_t osk = koskCur) override;
    virtual int32_t CbOnFile(void) override;
    virtual bool FFree(int32_t istn) override;

    bool FEnsureSpace(int32_t cstnAdd, int32_t cchAdd, uint32_t grfgrp = fgrpNil);
    void SetMinGrow(int32_t cstnAdd, int32_t cchAdd);

    virtual bool FAddRgch(const achar *prgch, int32_t cch, const void *pvExtra = pvNil, int32_t *pistn = pvNil) = 0;
    virtual bool FFindRgch(const achar *prgch, int32_t cch, int32_t *pistn, uint32_t grfgst = fgstNil);

    int32_t IstnMac(void)
    {
        return _ivMac;
    }
    int32_t CbExtra(void)
    {
        return _cbEntry - SIZEOF(int32_t);
    }

    bool FAddStn(PSTN pstn, void *pvExtra = pvNil, int32_t *pistn = pvNil);
    bool FPutRgch(int32_t istn, const achar *prgch, int32_t cch);
    bool FPutStn(int32_t istn, PSTN pstn);
    void GetRgch(int32_t istn, achar *prgch, int32_t cchMax, int32_t *pcch);
    void GetStn(int32_t istn, PSTN pstn);
    bool FFindStn(PSTN pstn, int32_t *pistn, uint32_t grfgst = fgstNil);

    void GetExtra(int32_t istn, void *pv);
    void PutExtra(int32_t istn, void *pv);
    bool FFindExtra(const void *prgbFind, PSTN pstn = pvNil, int32_t *pistn = pvNil);
};

/****************************************
    String table
****************************************/
#define GST_PAR GSTB
#define kclsGST 'GST'
class GST : public GST_PAR
{
    RTCLASS_DEC
    ASSERT

  protected:
    GST(int32_t cbExtra) : GSTB(cbExtra, fgstNil)
    {
    }

  public:
    // static methods
    static PGST PgstNew(int32_t cbExtra = 0, int32_t cstnInit = 0, int32_t cchInit = 0);
    static PGST PgstRead(PBLCK pblck, int16_t *pbo = pvNil, int16_t *posk = pvNil);
    static PGST PgstRead(PFIL pfil, FP fp, int32_t cb, int16_t *pbo = pvNil, int16_t *posk = pvNil);

    // duplication
    PGST PgstDup(void);

    // methods required by parent class
    virtual bool FAddRgch(const achar *prgch, int32_t cch, const void *pvExtra = pvNil,
                          int32_t *pistn = pvNil) override;
    virtual bool FFindRgch(const achar *prgch, int32_t cch, int32_t *pistn, uint32_t grfgst = fgstNil) override;
    virtual void Delete(int32_t istn) override;

    // new methods
    bool FInsertRgch(int32_t istn, const achar *prgch, int32_t cch, const void *pvExtra = pvNil);
    bool FInsertStn(int32_t istn, PSTN pstn, const void *pvExtra = pvNil);
    void Move(int32_t istnSrc, int32_t istnDst);
};

/****************************************
    Allocated string table
****************************************/
#define AST_PAR GSTB
#define kclsAST 'AST'
class AST : public AST_PAR
{
    RTCLASS_DEC
    ASSERT

  protected:
    AST(int32_t cbExtra) : GSTB(cbExtra, fgstAllowFree)
    {
    }

  public:
    // static methods
    static PAST PastNew(int32_t cbExtra = 0, int32_t cstnInit = 0, int32_t cchInit = 0);
    static PAST PastRead(PBLCK pblck, int16_t *pbo = pvNil, int16_t *posk = pvNil);
    static PAST PastRead(PFIL pfil, FP fp, int32_t cb, int16_t *pbo = pvNil, int16_t *posk = pvNil);

    // duplication
    PAST PastDup(void);

    // methods required by parent class
    virtual bool FAddRgch(const achar *prgch, int32_t cch, const void *pvExtra = pvNil,
                          int32_t *pistn = pvNil) override;
    virtual void Delete(int32_t istn) override;
};

#endif //! GROUPS_H
