/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Chunky file classes. See comments in chunk.cpp.

***************************************************************************/
#ifndef CHUNK_H
#define CHUNK_H

/***************************************************************************
    These must be unsigned longs!  We sort on them and assume in the code
    that they are unsinged.
***************************************************************************/
typedef uint32_t CTG;  // chunk tag/type
typedef uint32_t CNO;  // chunk number
typedef uint32_t CHID; // child chunk id

enum
{
    fcflNil = 0x0000,
    fcflWriteEnable = 0x0001,
    fcflTemp = 0x0002,
    fcflMark = 0x0004,
    fcflAddToExtra = 0x0008,

    // This flag indicates that when data is read, it should first be
    // copied to the extra file (if it's not already there). This is
    // for chunky files that are on a CD for which we want to cache data
    // to the hard drive.
    fcflReadFromExtra = 0x0010,

#ifdef DEBUG
    // for AssertValid
    fcflGraph = 0x4000, // check the graph structure for cycles
    fcflFull = fobjAssertFull,
#endif // DEBUG
};

// chunk identification
struct CKI
{
    CTG ctg;
    CNO cno;
};
VERIFY_STRUCT_SIZE(CKI, 8);
const BOM kbomCki = 0xF0000000;

// child chunk identification
struct KID
{
    CKI cki;
    CHID chid;
};
VERIFY_STRUCT_SIZE(KID, 12);
const BOM kbomKid = 0xFC000000;

/***************************************************************************
    Chunky file class.
***************************************************************************/
typedef class CFL *PCFL;
#define CFL_PAR BLL
#define kclsCFL 'CFL'
class CFL : public CFL_PAR
{
    RTCLASS_DEC
    BLL_DEC(CFL, PcflNext)
    ASSERT
    MARKMEM

  private:
    // chunk storage
    struct CSTO
    {
        PFIL pfil;  // the file
        FP fpMac;   // logical end of file (for writing new chunks)
        PGL pglfsm; // free space map
    };

    PGG _pggcrp;     // the index
    CSTO _csto;      // the main file
    CSTO _cstoExtra; // the scratch file

    bool _fAddToExtra : 1;
    bool _fMark : 1;
    bool _fFreeMapNotRead : 1;
    bool _fReadFromExtra : 1;
    bool _fInvalidMainFile : 1;

    // for deferred reading of the free map
    FP _fpFreeMap;
    int32_t _cbFreeMap;

#ifndef CHUNK_BIG_INDEX
    struct RTIE
    {
        CTG ctg;
        CNO cno;
        int32_t rti;
    };

    PGL _pglrtie;

    bool _FFindRtie(CTG ctg, CNO cno, RTIE *prtie = pvNil, int32_t *pirtie = pvNil);
#endif //! CHUNK_BIG_INDEX

    // static member variables
    static int32_t _rtiLast;
    static PCFL _pcflFirst;

  private:
    // private methods
    CFL(void);
    ~CFL(void);

    static uint32_t _GrffilFromGrfcfl(uint32_t grfcfl);

    bool _FReadIndex(void);
    tribool _TValidIndex(void);
    bool _FWriteIndex(CTG ctgCreator);
    bool _FCreateExtra(void);
    bool _FAllocFlo(int32_t cb, PFLO pflo, bool fForceOnExtra = fFalse);
    bool _FFindCtgCno(CTG ctg, CNO cno, int32_t *picrp);
    void _GetUniqueCno(CTG ctg, int32_t *picrp, CNO *pcno);
    void _FreeFpCb(bool fOnExtra, FP fp, int32_t cb);
    bool _FAdd(int32_t cb, CTG ctg, CNO cno, int32_t icrp, PBLCK pblck);
    bool _FPut(int32_t cb, CTG ctg, CNO cno, PBLCK pblck, PBLCK pblckSrc, const void *pv);
    bool _FCopy(CTG ctgSrc, CNO cnoSrc, PCFL pcflDst, CNO *pcnoDst, bool fClone);
    bool _FFindMatch(CTG ctgSrc, CNO cnoSrc, PCFL pcflDst, CNO *pcnoDst);
    bool _FFindCtgRti(CTG ctg, int32_t rti, CNO cnoMin, CNO *pcnoDst);
    bool _FDecRefCount(int32_t icrp);
    void _DeleteCore(int32_t icrp);
    bool _FFindChild(int32_t icrpPar, CTG ctgChild, CNO cnoChild, CHID chid, int32_t *pikid);
    bool _FAdoptChild(int32_t icrpPar, int32_t ikid, CTG ctgChild, CNO cnoChild, CHID chid, bool fClearLoner);
    void _ReadFreeMap(void);
    bool _FFindChidCtg(CTG ctgPar, CNO cnoPar, CHID chid, CTG ctg, KID *pkid);
    bool _FSetName(int32_t icrp, PSTN pstn);
    bool _FGetName(int32_t icrp, PSTN pstn);
    void _GetFlo(int32_t icrp, PFLO pflo);
    void _GetBlck(int32_t icrp, PBLCK pblck);
    bool _FEnsureOnExtra(int32_t icrp, FLO *pflo = pvNil);

    int32_t _Rti(CTG ctg, CNO cno);
    bool _FSetRti(CTG ctg, CNO cno, int32_t rti);

  public:
    // static methods
    static PCFL PcflFirst(void)
    {
        return _pcflFirst;
    }
    static PCFL PcflOpen(FNI *pfni, uint32_t grfcfl);
    static PCFL PcflCreate(FNI *pfni, uint32_t grfcfl);
    static PCFL PcflCreateTemp(FNI *pfni = pvNil);
    static PCFL PcflFromFni(FNI *pfni);

    static void ClearMarks(void);
    static void CloseUnmarked(void);
#ifdef CHUNK_STATS
    static void DumpStn(PSTN pstn, PFIL pfil = pvNil);
#endif // CHUNK_STATS

    virtual void Release(void);
    bool FSetGrfcfl(uint32_t grfcfl, uint32_t grfcflMask = (uint32_t)~0);
    void Mark(void)
    {
        _fMark = fTrue;
    }
    void SetTemp(bool f)
    {
        _csto.pfil->SetTemp(f);
    }
    bool FTemp(void)
    {
        return _csto.pfil->FTemp();
    }
    void GetFni(FNI *pfni)
    {
        _csto.pfil->GetFni(pfni);
    }
    bool FSetFni(FNI *pfni)
    {
        return _csto.pfil->FSetFni(pfni);
    }
    int32_t ElError(void);
    void ResetEl(int32_t el = elNil);
    bool FReopen(void);

    // finding and reading chunks
    bool FOnExtra(CTG ctg, CNO cno);
    bool FEnsureOnExtra(CTG ctg, CNO cno);
    bool FFind(CTG ctg, CNO cno, BLCK *pblck = pvNil);
    bool FFindFlo(CTG ctg, CNO cno, PFLO pflo);
    bool FReadHq(CTG ctg, CNO cno, HQ *phq);
    void SetPacked(CTG ctg, CNO cno, bool fPacked);
    bool FPacked(CTG ctg, CNO cno);
    bool FUnpackData(CTG ctg, CNO cno);
    bool FPackData(CTG ctg, CNO cno);

    // creating and replacing chunks
    bool FAdd(int32_t cb, CTG ctg, CNO *pcno, PBLCK pblck = pvNil);
    bool FAddPv(void *pv, int32_t cb, CTG ctg, CNO *pcno);
    bool FAddHq(HQ hq, CTG ctg, CNO *pcno);
    bool FAddBlck(PBLCK pblckSrc, CTG ctg, CNO *pcno);
    bool FPut(int32_t cb, CTG ctg, CNO cno, PBLCK pblck = pvNil);
    bool FPutPv(const void *pv, int32_t cb, CTG ctg, CNO cno);
    bool FPutHq(HQ hq, CTG ctg, CNO cno);
    bool FPutBlck(PBLCK pblck, CTG ctg, CNO cno);
    bool FCopy(CTG ctgSrc, CNO cnoSrc, PCFL pcflDst, CNO *pcnoDst);
    bool FClone(CTG ctgSrc, CNO cnoSrc, PCFL pcflDst, CNO *pcnoDst);
    void SwapData(CTG ctg1, CNO cno1, CTG ctg2, CNO cno2);
    void SwapChildren(CTG ctg1, CNO cno1, CTG ctg2, CNO cno2);
    void Move(CTG ctg, CNO cno, CTG ctgNew, CNO cnoNew);

    // creating child chunks
    bool FAddChild(CTG ctgPar, CNO cnoPar, CHID chid, int32_t cb, CTG ctg, CNO *pcno, PBLCK pblck = pvNil);
    bool FAddChildPv(CTG ctgPar, CNO cnoPar, CHID chid, void *pv, int32_t cb, CTG ctg, CNO *pcno);
    bool FAddChildHq(CTG ctgPar, CNO cnoPar, CHID chid, HQ hq, CTG ctg, CNO *pcno);

    // deleting chunks
    void Delete(CTG ctg, CNO cno);
    void SetLoner(CTG ctg, CNO cno, bool fLoner);
    bool FLoner(CTG ctg, CNO cno);

    // chunk naming
    bool FSetName(CTG ctg, CNO cno, PSTN pstn);
    bool FGetName(CTG ctg, CNO cno, PSTN pstn);

    // graph structure
    bool FAdoptChild(CTG ctgPar, CNO cnoPar, CTG ctgChild, CNO cnoChild, CHID chid = 0, bool fClearLoner = fTrue);
    void DeleteChild(CTG ctgPar, CNO cnoPar, CTG ctgChild, CNO cnoChild, CHID chid = 0);
    int32_t CckiRef(CTG ctg, CNO cno);
    tribool TIsDescendent(CTG ctg, CNO cno, CTG ctgSub, CNO cnoSub);
    void ChangeChid(CTG ctgPar, CNO cnoPar, CTG ctgChild, CNO cnoChild, CHID chidOld, CHID chidNew);

    // enumerating chunks
    int32_t Ccki(void);
    bool FGetCki(int32_t icki, CKI *pcki, int32_t *pckid = pvNil, PBLCK pblck = pvNil);
    bool FGetIcki(CTG ctg, CNO cno, int32_t *picki);
    int32_t CckiCtg(CTG ctg);
    bool FGetCkiCtg(CTG ctg, int32_t icki, CKI *pcki, int32_t *pckid = pvNil, PBLCK pblck = pvNil);

    // enumerating child chunks
    int32_t Ckid(CTG ctgPar, CNO cnoPar);
    bool FGetKid(CTG ctgPar, CNO cnoPar, int32_t ikid, KID *pkid);
    bool FGetKidChid(CTG ctgPar, CNO cnoPar, CHID chid, KID *pkid);
    bool FGetKidChidCtg(CTG ctgPar, CNO cnoPar, CHID chid, CTG ctg, KID *pkid);
    bool FGetIkid(CTG ctgPar, CNO cnoPar, CTG ctg, CNO cno, CHID chid, int32_t *pikid);

    // Serialized chunk forests
    bool FWriteChunkTree(CTG ctg, CNO cno, PFIL pfilDst, FP fpDst, int32_t *pcb);
    static PCFL PcflReadForestFromFlo(PFLO pflo, bool fCopyData);
    bool FForest(CTG ctg, CNO cno);
    void SetForest(CTG ctg, CNO cno, bool fForest = fTrue);
    PCFL PcflReadForest(CTG ctg, CNO cno, bool fCopyData);

    // writing
    bool FSave(CTG ctgCreator, FNI *pfni = pvNil);
    bool FSaveACopy(CTG ctgCreator, FNI *pfni);
};

/***************************************************************************
    Chunk graph enumerator
***************************************************************************/
enum
{
    // inputs
    fcgeNil = 0x0000,
    fcgeSkipToSib = 0x0001,

    // outputs
    fcgePre = 0x0010,
    fcgePost = 0x0020,
    fcgeRoot = 0x0040,
    fcgeError = 0x0080
};

#define CGE_PAR BASE
#define kclsCGE 'CGE'
class CGE : public CGE_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM
    NOCOPY(CGE)

  private:
    // data enumeration push state
    struct DPS
    {
        KID kid;
        int32_t ikid;
    };

    // enumeration states
    enum
    {
        esStart,    // waiting to start the enumeration
        esGo,       // go to the next node
        esGoNoSkip, // there are no children to skip, so ignore fcgeSkipToSib
        esDone      // we're done with the enumeration
    };

    int32_t _es; // current state
    PCFL _pcfl;  // the chunky file
    PGL _pgldps; // our stack of DPSs
    DPS _dps;    // the current DPS

  public:
    CGE(void);
    ~CGE(void);

    void Init(PCFL pcfl, CTG ctg, CNO cno);
    bool FNextKid(KID *pkid, CKI *pckiPar, uint32_t *pgrfcgeOut, uint32_t grfcgeIn);
};

#ifdef CHUNK_STATS
extern bool vfDumpChunkRequests;
#endif // CHUNK_STATS

#endif //! CHUNK_H
