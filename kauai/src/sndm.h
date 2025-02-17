/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Sound Management class for WAVE and MIDI support

***************************************************************************/
#ifndef SNDM_H
#define SNDM_H

const int32_t siiNil = 0;
const FTG kftgMidi = MacWin('MIDI', 'MID'); // REVIEW shonk: Mac: file type
const FTG kftgWave = MacWin('WAVE', 'WAV'); // REVIEW shonk: Mac: file type

/***************************************************************************
    Sound device - like audioman or our midi player.
***************************************************************************/
typedef class SNDV *PSNDV;
#define SNDV_PAR BASE
#define kclsSNDV 'SNDV'
class SNDV : public SNDV_PAR
{
    RTCLASS_DEC

  protected:
    static int32_t _siiLast;

    static int32_t _SiiAlloc(void)
    {
        return ++_siiLast;
    }

  public:
    virtual bool FActive(void) = 0;
    virtual void Activate(bool fActive) = 0; // boolean state
    virtual void Suspend(bool fSuspend) = 0; // reference count
    virtual void SetVlm(int32_t vlm) = 0;
    virtual int32_t VlmCur(void) = 0;

    virtual int32_t SiiPlay(PRCA prca, CTG ctg, CNO cno, int32_t sqn = ksqnNone, int32_t vlm = kvlmFull, int32_t cactPlay = 1,
                         uint32_t dtsStart = 0, int32_t spr = 0, int32_t scl = sclNil) = 0;

    virtual void Stop(int32_t sii) = 0;
    virtual void StopAll(int32_t sqn = sqnNil, int32_t scl = sclNil) = 0;

    virtual void Pause(int32_t sii) = 0;
    virtual void PauseAll(int32_t sqn = sqnNil, int32_t scl = sclNil) = 0;

    virtual void Resume(int32_t sii) = 0;
    virtual void ResumeAll(int32_t sqn = sqnNil, int32_t scl = sclNil) = 0;

    virtual bool FPlaying(int32_t sii) = 0;
    virtual bool FPlayingAll(int32_t sqn = sqnNil, int32_t scl = sclNil) = 0;

    virtual void Flush(void) = 0;
    virtual void BeginSynch(void);
    virtual void EndSynch(void);
};

/****************************************
    Sound manager class
****************************************/
typedef class SNDM *PSNDM;
#define SNDM_PAR SNDV
#define kclsSNDM 'SNDM'
class SNDM : public SNDM_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    struct SNDMPE
    {
        CTG ctg;
        PSNDV psndv;
    };

    PGL _pglsndmpe; // sound type to device mapper

    int32_t _cactSuspend;  // nesting level for suspending
    bool _fActive : 1;  // whether the app is active
    bool _fFreeing : 1; // we're in the destructor

    SNDM(void);
    bool _FInit(void);
    bool _FFindCtg(CTG ctg, SNDMPE *psndmpe, int32_t *pisndmpe = pvNil);

  public:
    static PSNDM PsndmNew(void);
    ~SNDM(void);

    // new methods
    virtual bool FAddDevice(CTG ctg, PSNDV psndv);
    virtual PSNDV PsndvFromCtg(CTG ctg);
    virtual void RemoveSndv(CTG ctg);

    // inherited methods
    virtual bool FActive(void);
    virtual void Activate(bool fActive);
    virtual void Suspend(bool fSuspend);
    virtual void SetVlm(int32_t vlm);
    virtual int32_t VlmCur(void);

    virtual int32_t SiiPlay(PRCA prca, CTG ctg, CNO cno, int32_t sqn = ksqnNone, int32_t vlm = kvlmFull, int32_t cactPlay = 1,
                         uint32_t dtsStart = 0, int32_t spr = 0, int32_t scl = sclNil);

    virtual void Stop(int32_t sii);
    virtual void StopAll(int32_t sqn = sqnNil, int32_t scl = sclNil);

    virtual void Pause(int32_t sii);
    virtual void PauseAll(int32_t sqn = sqnNil, int32_t scl = sclNil);

    virtual void Resume(int32_t sii);
    virtual void ResumeAll(int32_t sqn = sqnNil, int32_t scl = sclNil);

    virtual bool FPlaying(int32_t sii);
    virtual bool FPlayingAll(int32_t sqn = sqnNil, int32_t scl = sclNil);

    virtual void Flush(void);
    virtual void BeginSynch(void);
    virtual void EndSynch(void);
};

/***************************************************************************
    A useful base class for devices that support multiple queues.
***************************************************************************/
typedef class SNQUE *PSNQUE;

typedef class SNDMQ *PSNDMQ;
#define SNDMQ_PAR SNDV
#define kclsSNDMQ 'snmq'
class SNDMQ : public SNDMQ_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    // queue descriptor
    struct SNQD
    {
        PSNQUE psnque;
        int32_t sqn;
    };

    PGL _pglsnqd; // the queues

    int32_t _cactSuspend;
    bool _fActive : 1;

    virtual bool _FInit(void);
    virtual bool _FEnsureQueue(int32_t sqn, SNQD *psnqd, int32_t *pisnqd);

    virtual PSNQUE _PsnqueNew(void) = 0;
    virtual void _Suspend(bool fSuspend) = 0;

  public:
    ~SNDMQ(void);

    // inherited methods
    virtual bool FActive(void);
    virtual void Activate(bool fActive);
    virtual void Suspend(bool fSuspend);

    virtual int32_t SiiPlay(PRCA prca, CTG ctg, CNO cno, int32_t sqn = ksqnNone, int32_t vlm = kvlmFull, int32_t cactPlay = 1,
                         uint32_t dtsStart = 0, int32_t spr = 0, int32_t scl = sclNil);

    virtual void Stop(int32_t sii);
    virtual void StopAll(int32_t sqn = sqnNil, int32_t scl = sclNil);

    virtual void Pause(int32_t sii);
    virtual void PauseAll(int32_t sqn = sqnNil, int32_t scl = sclNil);

    virtual void Resume(int32_t sii);
    virtual void ResumeAll(int32_t sqn = sqnNil, int32_t scl = sclNil);

    virtual bool FPlaying(int32_t sii);
    virtual bool FPlayingAll(int32_t sqn = sqnNil, int32_t scl = sclNil);

    virtual void Flush(void);
};

/***************************************************************************
    The sound instance structure.
***************************************************************************/
struct SNDIN
{
    PBACO pbaco;    // the sound to play
    int32_t sii;       // the sound instance id
    int32_t vlm;       // volume to play at
    int32_t cactPlay;  // how many times to play
    uint32_t dtsStart; // offset to start at
    int32_t spr;       // sound priority
    int32_t scl;       // sound class

    // 0 means play, < 0 means skip, > 0 means pause
    int32_t cactPause;
};

/***************************************************************************
    Sound queue for a SNDMQ
***************************************************************************/
#define SNQUE_PAR BASE
#define kclsSNQUE 'snqu'
class SNQUE : public SNQUE_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PGL _pglsndin;   // the queue
    int32_t _isndinCur; // SNDIN that we should be playing

    SNQUE(void);

    virtual bool _FInit(void);
    virtual void _Queue(int32_t isndinMin) = 0;
    virtual void _PauseQueue(int32_t isndinMin) = 0;
    virtual void _ResumeQueue(int32_t isndinMin) = 0;
    virtual PBACO _PbacoFetch(PRCA prca, CTG ctg, CNO cno) = 0;

    virtual void _Enter(void);
    virtual void _Leave(void);
    virtual void _Flush(void);

  public:
    ~SNQUE(void);

    void Enqueue(int32_t sii, PRCA prca, CTG ctg, CNO cno, int32_t vlm, int32_t cactPlay, uint32_t dtsStart, int32_t spr, int32_t scl);

    int32_t SprCur(void);
    void Stop(int32_t sii);
    void StopAll(int32_t scl = sclNil);
    void Pause(int32_t sii);
    void PauseAll(int32_t scl = sclNil);
    void Resume(int32_t sii);
    void ResumeAll(int32_t scl = sclNil);
    bool FPlaying(int32_t sii);
    bool FPlayingAll(int32_t scl = sclNil);
    void Flush(void);
};

extern uint32_t LuVolScale(uint32_t luVolSys, int32_t vlm);
extern uint32_t vluSysVolFake;

#endif //! SNDM_H
