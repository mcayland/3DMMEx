/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Private declarations for mididev2 (streamed midi).

***************************************************************************/
#ifndef MDEV2PRI_H
#define MDEV2PRI_H

#include <fluidsynth.h>
#include <sndma.h>

extern const int32_t kdtsMinSlip;

#ifdef WIN

// This corresponds to the Win95 MIDIEVENT structure (with no optional data).
// We're using the older headers, so need to define our own.
struct MEV
{
    DWORD dwDeltaTime; // midi ticks between this and previous event
    DWORD dwStreamID;  // reserved - must be zero
    DWORD dwEvent;
};
typedef MEV *PMEV;

// This corresponds to the Win95 MIDIHDR structure.
// We're using the older headers, so need to define our own.
struct MH
{
    uint8_t *lpData;
    DWORD dwBufferLength;
    DWORD dwBytesRecorded;
    DWORD_PTR dwUser;
    DWORD dwFlags;
    MH *lpNext;
    DWORD_PTR reserved;
    DWORD dwOffset;
    DWORD_PTR dwReserved[8];
};
typedef MH *PMH;

typedef MIDIHDR *PMHO;

// A midi stream handle can be used as a midi out handle.
typedef HMIDIOUT HMS;

#else

#include <fluidsynth.h>

// MIDI Event
struct MEV
{
    uint32_t dwDeltaTime; // midi ticks between this and previous event
    uint32_t dwStreamID;  // reserved - must be zero
    uint32_t dwEvent;
};
typedef MEV *PMEV;

// MIDI header
struct MH
{
    uint8_t *lpData;
    uint32_t dwBufferLength;
    uint32_t dwBytesRecorded;
    uintptr_t dwUser;
    uint32_t dwFlags;
    MH *lpNext;
    uint32_t *reserved;
    uint32_t dwOffset;
    uint32_t *dwReserved[8];
};
typedef MH *PMH;

#endif

/***************************************************************************
    This is the midi stream cached object.
***************************************************************************/
typedef class MDWS *PMDWS;
#define MDWS_PAR BACO
#define kclsMDWS KLCONST4('M', 'D', 'W', 'S')
class MDWS : public MDWS_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PGL _pglmev;
    uint32_t _dts;

    MDWS(void);
    bool _FInit(PMIDS pmids);

  public:
    static bool FReadMdws(PCRF pcrf, CTG ctg, CNO cno, PBLCK pblck, PBACO *ppbaco, int32_t *pcb);
    static PMDWS PmdwsRead(PBLCK pblck);

    ~MDWS(void);

    uint32_t Dts(void)
    {
        return _dts;
    }
    void *PvLockData(int32_t *pcb);
    void UnlockData(void);
};

// forward declaration
typedef class MSMIX *PMSMIX;
typedef class MISI *PMISI;

/***************************************************************************
    Midi stream queue.
***************************************************************************/
typedef class MSQUE *PMSQUE;
#define MSQUE_PAR SNQUE
#define kclsMSQUE KLCONST4('m', 's', 'q', 'u')
class MSQUE : public MSQUE_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    MUTX _mutx;        // restricts access to member variables
    uint32_t _tsStart; // when we started the current sound
    PMSMIX _pmsmix;

    MSQUE(void);

    virtual void _Enter(void) override;
    virtual void _Leave(void) override;

    virtual bool _FInit(PMSMIX pmsmix);
    virtual PBACO _PbacoFetch(PRCA prca, CTG ctg, CNO cno) override;
    virtual void _Queue(int32_t isndinMin) override;
    virtual void _PauseQueue(int32_t isndinMin) override;
    virtual void _ResumeQueue(int32_t isndinMin) override;

  public:
    static PMSQUE PmsqueNew(PMSMIX pmsmix);
    ~MSQUE(void);

    void Notify(PMDWS pmdws);
};

/***************************************************************************
    Midi Stream "mixer". It really just chooses which midi stream to play
    (based on the (spr, sii) priority).
***************************************************************************/
typedef class MSMIX *PMSMIX;
#define MSMIX_PAR BASE
#define kclsMSMIX KLCONST4('m', 's', 'm', 'x')
class MSMIX : public MSMIX_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    struct MSOS
    {
        PMSQUE pmsque;     // the "channel" or queue that the sound is on
        PMDWS pmdws;       // the sound
        int32_t sii;       // its sound id (for a priority tie breaker)
        int32_t spr;       // its priority
        int32_t cactPlay;  // how many times to play the sound
        uint32_t dts;      // length of this sound
        uint32_t dtsStart; // position to start at
        int32_t vlm;       // volume to play at
        uint32_t tsStart;  // when we "started" the sound (minus dtsStart)
    };

    // Mutex to protect our member variables
    MUTX _mutx;
#ifdef KAUAI_WIN32
    HN _hevt; // to notify the thread that the sound list changed
    HN _hth;  // thread to terminate non-playing sounds
#else
    SDL_cond *_hevt;
    SDL_mutex *_hevtmutx;
    SDL_Thread *_hth;
    bool _hevtb;
#endif

    PMISI _pmisi;    // the midi stream interface
    PGL _pglmsos;    // the list of current sounds, in priority order
    int32_t _cpvOut; // number of buffers submitted (0, 1, or 2)

    PGL _pglmevKey;     // to accumulate state events for seeking
    bool _fChanged : 1; // the event has been signalled
    bool _fPlaying : 1; // whether we're currently playing the first stream
    bool _fWaiting : 1; // we're waiting for our buffers to get returned
    bool _fDone : 1;    // tells the aux thread to terminate

    int32_t _vlmBase;  // the base device volume
    int32_t _vlmSound; // the volume for the current sound

    MSMIX(void);
    bool _FInit(void);
    void _StopStream(void);
    bool _FGetKeyEvents(PMDWS pmdws, uint32_t dtsSeek, int32_t *pcbSkip);
    void _Restart(bool fNew = fFalse);
    void _WaitForBuffers(void);
    void _SubmitBuffers(uint32_t tsCur);

    static void _MidiProc(uintptr_t luUser, void *pvData, uintptr_t luData);
    void _Notify(void *pvData, PMDWS pmdws);

#ifdef KAUAI_WIN32
    static DWORD __stdcall _ThreadProc(void *pv);
    DWORD _LuThread(void);
#else
    static int _ThreadProc(void *pv);
    uint32_t _LuThread(void);
#endif

  public:
    static PMSMIX PmsmixNew(void);
    ~MSMIX(void);

    bool FPlay(PMSQUE pmsque, PMDWS pmdws = pvNil, int32_t sii = siiNil, int32_t spr = 0, int32_t cactPlay = 1,
               uint32_t dtsStart = 0, int32_t vlm = kvlmFull);

    void Suspend(bool fSuspend);
    void SetVlm(int32_t vlm);
    int32_t VlmCur(void);
};

// Define these so we can use old (msvc 2.1) header files
#ifndef MEVT_SHORTMSG
#define MEVT_SHORTMSG ((uint8_t)0x00) // parm = shortmsg for midiOutShortMsg
#define MEVT_TEMPO ((uint8_t)0x01)    // parm = new tempo in microsec/qn
#define MEVT_NOP ((uint8_t)0x02)      // parm = unused; does nothing
#define MIDIPROP_SET 0x80000000L
#define MIDIPROP_GET 0x40000000L
#define MIDIPROP_TIMEDIV 0x00000001L
#endif //! MEVT_SHORTMSG

/***************************************************************************
    The midi stream interface.
***************************************************************************/
typedef void (*PFNMIDI)(uintptr_t luUser, void *pvData, uintptr_t luData);

typedef class MISI *PMISI;
#define MISI_PAR BASE
#define kclsMISI KLCONST4('M', 'I', 'S', 'I')
class MISI : public MISI_PAR
{
    RTCLASS_DEC

  protected:
#ifdef KAUAI_WIN32
    HMS _hms;          // the midi stream handle
#else
    struct MS
    {
        fluid_synth_t *_flsynth;
        PMiniaudioStream _pastream;
    };
    typedef MS *HMS;

    HMS _hms;
#endif
    PFNMIDI _pfnCall;  // call back function
    uintptr_t _luUser; // user data to send back

    // system volume level - to be saved and restored. The volume we set
    // is always relative to this
    tribool _tBogusDriver; // to indicate whether midiOutGetVolume really works
    uint32_t _luVolSys;
    int32_t _vlmBase; // our current volume relative to _luVolSys.

    MISI(PFNMIDI pfn, uintptr_t luUser);

    virtual bool _FOpen(void) = 0;
    virtual bool _FClose(void) = 0;

    void _Reset(void);
    void _GetSysVol(void);
    void _SetSysVol(uint32_t luVol);
    void _SetSysVlm(void);

  public:
    virtual void SetVlm(int32_t vlm);
    virtual int32_t VlmCur(void);

    virtual bool FActive(void);
    virtual bool FActivate(bool fActivate);

    virtual bool FQueueBuffer(void *pvData, int32_t cb, int32_t ibStart, int32_t cactPlay, uintptr_t luData) = 0;
    virtual void StopPlaying(void) = 0;
};

/***************************************************************************
    The midiStreamStop API has a bug in it where it doesn't reset the
    current "buffer position" so that after calling midiStreamStop, then
    midiStreamOut and midiStreamRestart, the new buffer isn't played
    immediately, but the system waits until the previous buffer position
    expires before playing the new buffer.

    When this bug is fixed, STREAM_BUG can be undefined.
***************************************************************************/
#define STREAM_BUG

/***************************************************************************
    The real midi stream interface.
***************************************************************************/
typedef class WMS *PWMS;
#define WMS_PAR MISI
#define kclsWMS KLCONST3('W', 'M', 'S')
class WMS : public WMS_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
#define kcmhMsir 2
    struct MSIR
    {
        void *pvData;
        int32_t cb;
        int32_t cactPlay;
        uintptr_t luData;
        int32_t ibNext;

        MH rgmh[kcmhMsir];
        int32_t rgibLim[kcmhMsir];
    };
    typedef MSIR *PMSIR;

    MUTX _mutx;
#ifdef KAUAI_WIN32
    HINSTANCE _hlib;
#else
    void *_hlib;
#endif
    PGL _pglpmsir;
    int32_t _ipmsirCur;
    int32_t _cmhOut;

#ifdef KAUAI_WIN32
    HN _hevt; // event to wake up the thread
    HN _hth;  // thread to do callbacks and cleanup after a notify
#else
    void *_hevt;
    void *_hth;
#endif

#ifdef STREAM_BUG
    bool _fActive : 1;
#endif               // STREAM_BUG
    bool _fDone : 1; // tells the aux thread to terminate

#ifdef WIN
    MMRESULT(WINAPI *_pfnOpen)
    (HMS *phms, LPUINT puDeviceID, DWORD cMidi, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen);
    MMRESULT(WINAPI *_pfnClose)(HMS hms);
    MMRESULT(WINAPI *_pfnProperty)(HMS hms, LPBYTE lpb, DWORD dwProperty);
    MMRESULT(WINAPI *_pfnPosition)(HMS hms, LPMMTIME lpmmt, UINT cbmmt);
    MMRESULT(WINAPI *_pfnOut)(HMS hms, LPMIDIHDR pmh, UINT cbmh);
    MMRESULT(WINAPI *_pfnPause)(HMS hms);
    MMRESULT(WINAPI *_pfnRestart)(HMS hms);
    MMRESULT(WINAPI *_pfnStop)(HMS hms);
#endif

    WMS(PFNMIDI pfn, uintptr_t luUser);
    bool _FInit(void);

    virtual bool _FOpen(void) override;
    virtual bool _FClose(void) override;

    bool _FSubmit(PMH pmh);
    void _DoCallBacks(void);
    int32_t _CmhSubmitBuffers(void);
    void _ResetStream(void);

#ifdef KAUAI_WIN32
    // MidiOutProc callback function
    static void __stdcall _MidiProc(HMS hms, UINT msg, DWORD_PTR luUser, DWORD_PTR lu1, DWORD_PTR lu2);

    static DWORD __stdcall _ThreadProc(void *pv);
    DWORD _LuThread(void);
#endif
    void _Notify(HMS hms, PMH pmh);

  public:
    static PWMS PwmsNew(PFNMIDI pfn, uintptr_t luUser);
    ~WMS(void);

#ifdef STREAM_BUG
    virtual bool FActive(void) override;
    virtual bool FActivate(bool fActivate) override;
#endif // STREAM_BUG

    virtual bool FQueueBuffer(void *pvData, int32_t cb, int32_t ibStart, int32_t cactPlay, uintptr_t luData) override;
    virtual void StopPlaying(void) override;
};

/***************************************************************************
    Our fake midi stream class.
***************************************************************************/
typedef class OMS *POMS;
#define OMS_PAR MISI
#define kclsOMS KLCONST3('O', 'M', 'S')
class OMS : public OMS_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    struct MSB
    {
        void *pvData;
        int32_t cb;
        int32_t ibStart;
        int32_t cactPlay;

        uintptr_t luData;
    };

    MUTX _mutx;
#ifdef KAUAI_WIN32
    HN _hevt; // event to notify the thread that the stream data has changed
    HN _hth;  // thread to play the stream data
#else
    SDL_cond *_hevt;   // event to notify the thread that the stream data has changed
    SDL_mutex *_hevtmutx;
    SDL_Thread *_hth;  // thread to play the midi events
    SDL_Thread *_hthr; // thread to render audio samples

    fluid_settings_t *_flset;
    fluid_synth_t *_flsynth;
    short _flsynth_dest;
    fluid_sequencer_t *_flseq;
    fluid_audio_driver_t *_fldriver;
    int _flframecount;
    float *_flframe;

    PMiniaudioStream _pastream;
#endif

    bool _fChanged : 1; // the event has been signalled
    bool _fStop : 1;    // tells the aux thread to stop all buffers
    bool _fDone : 1;    // tells the aux thread to return

    int32_t _imsbCur;
    PGL _pglmsb;
    PMEV _pmev;
    PMEV _pmevLim;
    uint32_t _tsCur;

    OMS(PFNMIDI pfn, uintptr_t luUser);
    bool _FInit(void);

    virtual bool _FOpen(void) override;
    virtual bool _FClose(void) override;

#ifdef KAUAI_WIN32
    static DWORD __stdcall _ThreadProc(void *pv);
    DWORD _LuThread(void);
#else
    static int _ThreadProc(void *pv);
    uint32_t _LuThread(void);
    static int _ThreadProcRender(void *pv);
    uint32_t _LuRenderThread(void);
#endif
    void _ReleaseBuffers(void);

  public:
    static POMS PomsNew(PFNMIDI pfn, uintptr_t luUser);
    ~OMS(void);

    virtual bool FQueueBuffer(void *pvData, int32_t cb, int32_t ibStart, int32_t cactPlay, uintptr_t luData) override;
    virtual void StopPlaying(void) override;
};

#endif //! MDEV2PRI_H
