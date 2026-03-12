/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    MIDI stream interface: POSIX

***************************************************************************/
#ifndef MIDISTREAMPOSIX_H
#define MIDISTREAMPOSIX_H

#include <fluidsynth.h>
#include <sndma.h>

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

    struct MS
    {
        fluid_synth_t *_flsynth;
        PMiniaudioStream _pastream;
    };
    typedef MS *HMS;

    HMS _hms;
    
    MUTX _mutx;
    
#if 0
    std::thread _hevt; // event to notify the thread that the stream data has changed
    std::thread _hth;  // thread to play the stream data
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

    uint32_t _luVolSys;
    int32_t _vlmBase; // our current volume relative to _luVolSys.

    std::atomic<bool> _fChanged; // the event has been signalled
    std::atomic<bool> _fStop;    // tells the aux thread to stop all buffers
    std::atomic<bool> _fDone;    // tells the aux thread to return

    int32_t _imsbCur;
    PGL _pglmsb;
    PMEV _pmev;
    PMEV _pmevLim;
    uint32_t _tsCur;

    OMS(PFNMIDI pfn, uintptr_t luUser);
    bool _FInit(void);

    virtual bool _FOpen(void);
    virtual bool _FClose(void);

    void _Reset(void);
    void _GetSysVol(void);
    void _SetSysVol(uint32_t luVol);
    void _SetSysVlm(void);

    static int _ThreadProc(void *pv);
    uint32_t _LuThread(void);
    static int _ThreadProcRender(void *pv);
    uint32_t _LuRenderThread(void);

    void _ReleaseBuffers(void);

  public:
    static POMS PomsNew(PFNMIDI pfn, uintptr_t luUser);
    ~OMS(void);

    virtual void SetVlm(int32_t vlm);
    virtual int32_t VlmCur(void);

    virtual bool FActive(void);
    virtual bool FActivate(bool fActivate);

    virtual bool FQueueBuffer(void *pvData, int32_t cb, int32_t ibStart, int32_t cactPlay, uintptr_t luData) override;
    virtual void StopPlaying(void) override;
};

#endif //! MIDISTREAMPOSIX_H
