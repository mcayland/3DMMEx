/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Stream classes.

    A BSM is a byte stream in memory.  The data is stored contiguously, so
    should be used only for relatively small streams.

    A BSF is a byte stream with pieces stored in files and other pieces
    stored in memory.

***************************************************************************/
#ifndef STREAM_H
#define STREAM_H

/***************************************************************************
    Byte stream in memory.  The entire stream is in contiguous memory.
***************************************************************************/
typedef class BSM *PBSM;
#define BSM_PAR BASE
#define kclsBSM 'BSM'
class BSM : public BSM_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM
    NOCOPY(BSM)

  protected:
    HQ _hqrgb;
    int32_t _ibMac;
    int32_t _cbMinGrow;

    bool _FEnsureSize(int32_t cbMin, bool fShrink);

  public:
    BSM(void);
    ~BSM(void);

    void SetMinGrow(int32_t cb);
    bool FEnsureSpace(int32_t cb, bool fShrink);
    void *PvLock(int32_t ib);
    void Unlock(void);

    int32_t IbMac(void)
    {
        return _ibMac;
    }
    void FetchRgb(int32_t ib, int32_t cb, void *prgb);
    bool FReplace(const void *prgb, int32_t cbIns, int32_t ib, int32_t cbDel);
    bool FWriteRgb(PFLO pflo, int32_t ib = 0);
    bool FWriteRgb(PBLCK pblck, int32_t ib = 0);
};

/***************************************************************************
    Byte stream on file.  Parts of the stream may be in files.
***************************************************************************/
typedef class BSF *PBSF;
#define BSF_PAR BASE
#define kclsBSF 'BSF'
class BSF : public BSF_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM
    NOCOPY(BSF)

  protected:
    PGG _pggflo;
    int32_t _ibMac;

    int32_t _IfloFind(int32_t ib, int32_t *pib, int32_t *pcb = pvNil);
    bool _FEnsureSplit(int32_t ib, int32_t *piflo = pvNil);
    void _AttemptMerge(int32_t ibMin, int32_t ibLim);
    bool _FReplaceCore(void *prgb, int32_t cbIns, PFLO pflo, int32_t ib, int32_t cbDel);

  public:
    BSF(void);
    ~BSF(void);

    int32_t IbMac(void)
    {
        return _ibMac;
    }
    void FetchRgb(int32_t ib, int32_t cb, void *prgb);
    bool FReplace(const void *prgb, int32_t cbIns, int32_t ib, int32_t cbDel);
    bool FReplaceFlo(PFLO pflo, bool fCopy, int32_t ib, int32_t cbDel);
    bool FReplaceBsf(PBSF pbsfSrc, int32_t ibSrc, int32_t cbSrc, int32_t ibDst, int32_t cbDel);
    bool FWriteRgb(PFLO pflo, int32_t ib = 0);
    bool FWriteRgb(PBLCK pblck, int32_t ib = 0);
    bool FCompact(void);
};

#endif //! STREAM_H
