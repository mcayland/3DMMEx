/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Copyright (c) Microsoft Corporation

    The midi player device using a Midi Stream.

***************************************************************************/
#include "frame.h"
#include "mdev2pri.h"
ASSERTNAME

#include <fluidsynth.h>

//#include <thread>
//using namespace std::chrono_literals;

/***************************************************************************
    Constructor for the midi stream output object.
***************************************************************************/
MSMIX::MSMIX(void)
{
}

/***************************************************************************
    Destructor for the midi stream output object.
***************************************************************************/
MSMIX::~MSMIX(void)
{
    Assert(pvNil == _pmisi || !_pmisi->FActive(), "MISI still active!");

    if (hNil != _hth)
    {
        // tell the thread to end and wait for it to finish
        _fDone = fTrue;
        SDL_CondSignal(_hevt);
#if 0
        WaitForSingleObject(_hth, INFINITE);
#endif
    }

#if 0
    if (hNil != _hevt)
        CloseHandle(_hevt);
#endif
    if (pvNil != _pglmsos)
    {
        Assert(_pglmsos->IvMac() == 0, "MSMIX still has active sounds");
        ReleasePpo(&_pglmsos);
    }
    ReleasePpo(&_pmisi);
    ReleasePpo(&_pglmevKey);
}

/***************************************************************************
    Initialize the MSMIX - allocate the pglmsos and the midi stream api
    object.
***************************************************************************/
bool MSMIX::_FInit(void)
{
    AssertBaseThis(0);

    if (pvNil == (_pglmsos = GL::PglNew(SIZEOF(MSOS))))
        return fFalse;
    _pglmsos->SetMinGrow(1);

    if (pvNil == (_pmisi = OMS::PomsNew(_MidiProc, (uintptr_t)this)))
    {
        return fFalse;
    }

    _hevtmutx = SDL_CreateMutex();
    _hevt = SDL_CreateCond();
    _mutx.Enter();
    _hth = SDL_CreateThread(MSMIX::_ThreadProc, "msmix-sdl", this);
    _mutx.Leave();

    return fTrue;
}

/***************************************************************************
    If we're currently playing a midi stream stop it. Assumes the mutx is
    already checked out exactly once.
***************************************************************************/
void MSMIX::_StopStream(void)
{
    AssertThis(0);

    if (!_fPlaying)
        return;

    // set _fPlaying to false first so the call back knows that we're
    // aborting the current stream - so it doesn't notify us.
    _fPlaying = fFalse;

    _pmisi->StopPlaying();

    // Wait for the buffers to be returned
    _fWaiting = fTrue;
    _mutx.Leave();

    while (_cpvOut > 0) {
        fprintf(stderr, ">>> MSMIX::cpvOut is %x\n", _cpvOut);
    //    std::this_thread::sleep_for(0ms);
    }
    fprintf(stderr, "=== MSMIX::cpvOut == 0\n");
    _mutx.Enter();
    _fWaiting = fFalse;
}

/***************************************************************************
    The sound list changed so make sure we're playing the first tune.
    Assumes the mutx is already checked out.
***************************************************************************/
void MSMIX::_Restart(bool fNew)
{
    AssertThis(0);

    if (_pmisi->FActive() && !_fPlaying && _pglmsos->IvMac() > 0)
    {
        // start playing the first MSOS
        MSOS msos;
        uint32_t tsCur = TsCurrentSystem();

        if (fNew)
        {
            _pglmsos->Get(0, &msos);
            msos.tsStart = tsCur - msos.dtsStart;
            _pglmsos->Put(0, &msos);
            fprintf(stderr, "----> Put3\n");
        }
        _SubmitBuffers(tsCur);
    }

    // signal the aux thread that the list changed
    fprintf(stderr, " >>> MSMIX::_Restart condsignal\n");
    SDL_CondSignal(_hevt);
}

/***************************************************************************
    Call back from the midi stream stuff.
***************************************************************************/
void MSMIX::_MidiProc(uintptr_t luUser, void *pvData, uintptr_t luData)
{
    PMSMIX pmsmix;
    PMDWS pmdws;

    pmsmix = (PMSMIX)luUser;
    AssertPo(pmsmix, 0);
    pmdws = (PMDWS)luData;
    AssertNilOrPo(pmdws, 0);

    fprintf(stderr, " >>> MSMIX::_MidiProc about to notify\n");
    pmsmix->_Notify(pvData, pmdws);
}

/***************************************************************************
    AT: Static method. Thread function for the MSMIX object.
***************************************************************************/
int MSMIX::_ThreadProc(void *pv)
{
    PMSMIX pmsmix = (PMSMIX)pv;

    AssertPo(pmsmix, 0);

    return pmsmix->_LuThread();
}

/***************************************************************************
    AT: This thread just sleeps until the next sound is due to expire, then
    wakes up and nukes any expired sounds.
***************************************************************************/
uint32_t MSMIX::_LuThread(void)
{
    AssertThis(0);
    uint32_t tsCur;
    int32_t imsos;
    MSOS msos;
    int32_t cactSkip;
    uint32_t dtsNextStop = kluMax;

    for (;;)
    {
        fprintf(stderr, "<<<< MSMIX::_LuThread before condwait\n");
        SDL_LockMutex(_hevtmutx);
        if (!_fChanged) {
            SDL_CondWaitTimeout(_hevt, _hevtmutx, dtsNextStop);
        }
        SDL_UnlockMutex(_hevtmutx);
        fprintf(stderr, ">>>> MSMIX::_LuThread after condwait\n");

        if (_fDone)
            return 0;

        _mutx.Enter();

        if (_fWaiting)
        {
            // we're waiting for buffers to be returned, so don't touch
            // anything!
            dtsNextStop = 1;
            fprintf(stderr, " ___ fWaiting\n");
        }
        else
        {
            // See if any sounds have expired...
            tsCur = TsCurrentSystem();
            dtsNextStop = kluMax;
            fprintf(stderr, " ___ MSMIX expire check count: %d  _fPlaying: %d\n", _pglmsos->IvMac(), _fPlaying);
            for (imsos = _pglmsos->IvMac(); imsos-- > 0;)
            {
                if (imsos == 0 && _fPlaying)
                    break;
                _pglmsos->Get(imsos, &msos);

                cactSkip = (tsCur - msos.tsStart) / msos.dts;
                fprintf(stderr, " ___ cactSkip is %d\n", cactSkip);
                if (cactSkip > 0)
                {
                    uint32_t dtsSeek;

                    if (msos.cactPlay > 0 && (msos.cactPlay -= cactSkip) <= 0)
                    {
                        // this sound is done
                        _pglmsos->Delete(imsos);
                        _mutx.Leave();

                        // do the notify
                        fprintf(stderr, " ___<<<< about to NOTIFY\n");
                        msos.pmsque->Notify(msos.pmdws);

                        _mutx.Enter();
                        dtsNextStop = 0;
                        break;
                    }

                    // adjust the values in the MSOS
                    dtsSeek = (tsCur - msos.tsStart) % msos.dts;
                    msos.tsStart = tsCur - dtsSeek;
                    _pglmsos->Put(imsos, &msos);
                }

                dtsNextStop = LuMin(dtsNextStop, msos.dts - (tsCur - msos.tsStart));
            }
        }

        _mutx.Leave();
    }
}

/***************************************************************************
    Reset the midi device.
***************************************************************************/
void MISI::_Reset(void)
{
    Assert(hNil != _hms, 0);
    fluid_synth_t *_flsynth = (fluid_synth_t *)_hms;

    fluid_synth_all_notes_off(_flsynth, -1);
}

/***************************************************************************
    Get the system volume level.
***************************************************************************/
void MISI::_GetSysVol(void)
{
    Assert(hNil != _hms, "calling _GetSysVol with nil _hms");
    //fluid_synth_t *_flsynth = (fluid_synth_t *)_hms;

    //float gain = fluid_synth_get_gain(_flsynth);
    //_luVolSys = (uint32_t)(0x1fff * gain);
}

/***************************************************************************
    Set the system volume level.
***************************************************************************/
void MISI::_SetSysVol(uint32_t luVol)
{
    Assert(hNil != _hms, "calling _SetSysVol with nil _hms");
    //fluid_synth_t *_flsynth = (fluid_synth_t *)_hms;
fprintf(stderr, " FLVOL is %d\n", luVol);
    //fluid_synth_set_gain(_flsynth, ((float)luVol) / 0x1fff);
}

/***************************************************************************
    Set the system volume level from the current values of _vlmBase
    and _luVolSys. We set the system volume to the result of scaling
    _luVolSys by _vlmBase.
***************************************************************************/
void MISI::_SetSysVlm(void)
{
    uint32_t luVol;

    luVol = LuVolScale(_luVolSys, _vlmBase);
    _SetSysVol(luVol);
}

/***************************************************************************
    Destructor for the Win95 Midi stream class.
***************************************************************************/
WMS::~WMS(void)
{
}

/***************************************************************************
    Initialize the WMS: get the addresses of the stream API.
***************************************************************************/
bool WMS::_FInit(void)
{
    _hlib = (void *)-1;
    _hevt = (void *)-1;
    _hth = (void *)-1;

    if (pvNil == (_pglpmsir = GL::PglNew(SIZEOF(PMSIR))))
        return fFalse;
    _pglpmsir->SetMinGrow(1);

    AssertThis(0);
    return fTrue;
}

/***************************************************************************
    Opens the midi stream and sets the time division to 1000 ticks per
    quarter note. It is assumed that the midi data has a tempo record
    indicating 1 quarter note per second (1000000 microseconds per quarter).
    The end result is that ticks are milliseconds.
***************************************************************************/
bool WMS::_FOpen(void)
{
    AssertThis(0);

    _hms = (void *)-1;

    return fFalse;
}

/***************************************************************************
    Close the midi stream.
***************************************************************************/
bool WMS::_FClose(void)
{
    AssertThis(0);

    return fTrue;
}

#ifdef STREAM_BUG
/***************************************************************************
    Just return the value of our flag, not (hNil != _hms).
***************************************************************************/
bool WMS::FActive(void)
{
    return _fActive;
}

/***************************************************************************
    Need to set _fActive as well.
***************************************************************************/
bool WMS::FActivate(bool fActivate)
{
    bool fRet;

    fRet = WMS_PAR::FActivate(fActivate);
    if (fRet)
        _fActive = FPure(fActivate);
    return fRet;
}
#endif // STREAM_BUG

/***************************************************************************
    Reset the midi stream so it's ready to accept new input. Assumes we
    already have the mutx.
***************************************************************************/
void WMS::_ResetStream(void)
{
    return;
}

/***************************************************************************
    Prepare and submit the given buffer. Assumes the mutx is ours.
***************************************************************************/
bool WMS::_FSubmit(PMH pmh)
{
    bool fRestart = (0 == _cmhOut);
    PMEV pmevStart;
    PMEV pmevCur;
    int32_t iMevCount;
    int32_t iMevCur;

    if (hNil == _hms)
        return fFalse;

    iMevCur = 0;
    iMevCount = pmh->dwBufferLength / SIZEOF(MEV);
    pmevStart = (PMEV)pmh->lpData;
    while (iMevCur < iMevCount) {
        pmevCur = &pmevStart[iMevCur];

        fprintf(stderr, "Delta: %d  Event: 0x%x\n", pmevCur->dwDeltaTime, pmevCur->dwEvent);

        iMevCur++;
    }

    fprintf(stderr, "DONE!\n");
    return fTrue;
}

/***************************************************************************
    Stop the midi stream.
***************************************************************************/
void WMS::StopPlaying(void)
{
    AssertThis(0);
}

/***************************************************************************
    Constructor for our own midi stream api implementation.
***************************************************************************/
OMS::OMS(PFNMIDI pfn, uintptr_t luUser) : MISI(pfn, luUser)
{
    fluid_audio_driver_t* adriver;
    int id;

    _flset = new_fluid_settings();
    Assert(_flset != pvNil, "failed to create fluidsynth settings");
    fluid_settings_setnum(_flset, "synth.sample-rate", 44100.0);
    _flsynth = new_fluid_synth(_flset);
    Assert(_flsynth != pvNil, "failed to create fluidsynth synth");

    id = fluid_synth_sfload(_flsynth, "/usr/share/sounds/sf2/default-GM.sf2", true);
    Assert(id != FLUID_FAILED, "failed to load soundfont");

    fluid_settings_setstr(_flset, "audio.driver", "pulseaudio");
    adriver = new_fluid_audio_driver(_flset, _flsynth);
    Assert(adriver != pvNil, "failed to load pulse driver");
}

/***************************************************************************
    Destructor for our midi stream.
***************************************************************************/
OMS::~OMS(void)
{
    delete_fluid_synth(_flsynth);
    delete_fluid_settings(_flset);
}

/***************************************************************************
    Initialize the OMS.
***************************************************************************/
bool OMS::_FInit(void)
{
    AssertBaseThis(0);

    if (pvNil == (_pglmsb = GL::PglNew(SIZEOF(MSB))))
        return fFalse;
    _pglmsb->SetMinGrow(1);

    _hevtmutx = SDL_CreateMutex();
    _hevt = SDL_CreateCond();
    _mutx.Enter();
    _hth = SDL_CreateThread(OMS::_ThreadProc, "sdl-midi-event", this);
    _hthr = SDL_CreateThread(OMS::_ThreadProcRender, "sdl-midi-render", this);
    _mutx.Leave();

    return fTrue;
}

/***************************************************************************
    Open the stream.
***************************************************************************/
bool OMS::_FOpen(void)
{
    AssertThis(0);

    _mutx.Enter();
    fprintf(stderr, "OMS open\n");
    if (hNil != _hms)
        goto LDone;

    _fChanged = _fStop = fFalse;

    _hms = _flsynth;

    // get the system volume level
    _GetSysVol();

    // set our volume level
    _SetSysVlm();

LDone:
    _mutx.Leave();

    return fTrue;
}

/***************************************************************************
    Close the stream.
***************************************************************************/
bool OMS::_FClose(void)
{
    AssertThis(0);

    fprintf(stderr, "OMS close\n");

    _mutx.Enter();

    if (hNil == _hms)
    {
        _mutx.Leave();
        return fTrue;
    }

    if (_pglmsb->IvMac() > 0)
    {
        Bug("closing a stream that still has buffers!");
        _mutx.Leave();
        return fFalse;
    }

    // reset the device
    _Reset();

    // restore the volume level
    _SetSysVol(_luVolSys);

    //midiOutClose(_hms);
    //_hms = hNil;

    _mutx.Leave();

    return fTrue;
}

/***************************************************************************
    Queue a buffer to the midi stream.
***************************************************************************/
bool OMS::FQueueBuffer(void *pvData, int32_t cb, int32_t ibStart, int32_t cactPlay, uintptr_t luData)
{
    AssertThis(0);
    AssertPvCb(pvData, cb);
    AssertIn(ibStart, 0, cb);
    Assert(cb % SIZEOF(MEV) == 0, "bad cb");
    Assert(ibStart % SIZEOF(MEV) == 0, "bad cb");

    MSB msb;

    _mutx.Enter();

    if (hNil == _hms)
        goto LFail;

    msb.pvData = pvData;
    msb.cb = cb;
    msb.ibStart = ibStart;
    msb.cactPlay = cactPlay;
    msb.luData = luData;

    if (!_pglmsb->FAdd(&msb))
    {
    LFail:
        _mutx.Leave();
        return fFalse;
    }

    if (1 == _pglmsb->IvMac())
    {
        // Start the buffer
        fprintf(stderr, "OMS::FQueue signal\n");
        _fChanged = fTrue;
        SDL_CondSignal(_hevt);
    }

    _mutx.Leave();

    return fTrue;
}

/***************************************************************************
    Stop the stream and release all buffers. The buffer notifies are
    asynchronous.
***************************************************************************/
void OMS::StopPlaying(void)
{
    AssertThis(0);

    _mutx.Enter();

    if (hNil != _hms)
    {
        fprintf(stderr, ">>> OMS::StopPlaying\n");
        _fStop = fTrue;
        _fChanged = fTrue;
        SDL_CondSignal(_hevt);
    }

    _mutx.Leave();
}

/***************************************************************************
    AT: Static method. Thread function for the midi stream object.
***************************************************************************/
int OMS::_ThreadProc(void *pv)
{
    POMS poms = (POMS)pv;

    AssertPo(poms, 0);

    return poms->_LuThread();
}

/***************************************************************************
    AT: The midi stream playback thread.
***************************************************************************/
uint32_t OMS::_LuThread(void)
{
    AssertThis(0);
    MSB msb;
    bool fChanged; // whether the event went off
    uint32_t tsCur;
    const int32_t klwInfinite = klwMax;
    int32_t dtsWait = klwInfinite;

    for (;;)
    {
        fprintf(stderr, "<<<< OMS::_LuThread before condwait: dtsWait is %d\n", dtsWait);
        SDL_LockMutex(_hevtmutx);
        if (!_fChanged) {
            fChanged =
                dtsWait > 0 && SDL_MUTEX_TIMEDOUT != SDL_CondWaitTimeout(_hevt, _hevtmutx, dtsWait == klwInfinite ? SDL_MUTEX_MAXWAIT : dtsWait);
        }
        else
        {
            fChanged = true;
        }
        SDL_UnlockMutex(_hevtmutx);
        fprintf(stderr, ">>>> OMS::_LuThread after condwait: fChanged is %d, dtsWait is %d\n", fChanged, dtsWait);

        if (_fDone)
            return 0;

        _mutx.Enter();
        if (_fChanged && !fChanged)
        {
            // the event went off before we got the mutx.
            dtsWait = klwInfinite;
            goto LLoop;
        }

        _fChanged = fFalse;
        if (!fChanged)
        {
            // play the event
            if (_pmev < _pmevLim)
            {
                if (MEVT_SHORTMSG == (_pmev->dwEvent >> 24)) {

                switch (_pmev->dwEvent & 0xf0)
                {
                    case 0x80: /* Note off */
                        fluid_synth_noteoff(_flsynth, _pmev->dwEvent & 0xf,
                                            (_pmev->dwEvent & 0x7f00) >> 8);
                        break;

                    case 0x90: /* Note on */
                        fluid_synth_noteon(_flsynth,
                                           _pmev->dwEvent & 0xf,
                                           (_pmev->dwEvent & 0x7f00) >> 8,
                                           (_pmev->dwEvent & 0x7f0000) >> 16);
                        break;

                    case 0xb0: /* Control change */
                        fluid_synth_cc(_flsynth,
                                       _pmev->dwEvent & 0xf,
                                       (_pmev->dwEvent & 0x7f00) >> 8,
                                       (_pmev->dwEvent & 0x7f0000) >> 16);
                        break;

                    case 0xc0: /* Program change */
                        fluid_synth_program_change(_flsynth,
                                                   _pmev->dwEvent & 0xf,
                                                   (_pmev->dwEvent & 0x7f00) >> 8);
                        break;

                    case 0xd0: /* Channel pressure */
                        fluid_synth_channel_pressure(_flsynth,
                                                     _pmev->dwEvent & 0xf,
                                                     (_pmev->dwEvent & 0x7f00) >> 8);
                        break;

                    case 0xe0: /* Pitch wheel */
                        fluid_synth_pitch_bend(_flsynth,
                                               _pmev->dwEvent & 0xf,
                                               ((_pmev->dwEvent & 0x7f00) >> 8) |
                                               ((_pmev->dwEvent & 0x7f0000) >> 9));
                        break;
                }

                }
                //if (MEVT_SHORTMSG == (_pmev->dwEvent >> 24))
                //    fprintf(stderr, "#### key 0x%x  status 0x%x\n", _pmev->dwEvent, _pmev->dwEvent & 0xf0);

                _pmev++;
                if (_pmev >= _pmevLim)
                {
                    dtsWait = 0;
                    fprintf(stderr, " >>> dtsWait 0.1\n");
                }
                else
                {
                    uint32_t tsNew = TsCurrentSystem();

                    tsCur += _pmev->dwDeltaTime;
                    dtsWait = tsCur - tsNew;
                    if (dtsWait < -kdtsMinSlip)
                    {
                        tsCur = tsNew;
                        dtsWait = 0;
                        fprintf(stderr, " >>> dtsWait 0.2\n");
                    }
                }
                goto LLoop;
            }

            // ran out of events in the current buffer - see if we should
            // repeat it
            _pglmsb->Get(0, &msb);
            if (msb.cactPlay == 1)
            {
                _imsbCur = 1;
                _ReleaseBuffers();
            }
            else
            {
                // repeat the current buffer
                if (msb.cactPlay > 0)
                    msb.cactPlay--;
                msb.ibStart = 0;
                _pglmsb->Put(0, &msb);
            }
        }
        else if (_fStop)
        {
            // release all buffers
            _fStop = fFalse;
            _imsbCur = _pglmsb->IvMac();
            _ReleaseBuffers();
        }

        if (0 == _pglmsb->IvMac())
        {
            // no buffers to play
            dtsWait = klwInfinite;
        }
        else
        {
            // start playing the new buffers
            _pglmsb->Get(0, &msb);
            _pmev = (PMEV)PvAddBv(msb.pvData, msb.ibStart);
            _pmevLim = (PMEV)PvAddBv(msb.pvData, msb.cb);
            if (_pmev >= _pmevLim)
            {
                dtsWait = 0;
                fprintf(stderr, " >>> dtsWait 0.3\n");
            }
            else
            {
                dtsWait = _pmev->dwDeltaTime;
                tsCur = TsCurrentSystem() + dtsWait;
                fprintf(stderr, " >>> dtsWait 0.4 tsCur %d, dtsWait %d\n", tsCur, dtsWait);
            }
        }
    LLoop:
        _mutx.Leave();
    }
}

/***************************************************************************
    AT: Static method. Thread function for the midi event renderer.
***************************************************************************/
int OMS::_ThreadProcRender(void *pv)
{
    POMS poms = (POMS)pv;

    AssertPo(poms, 0);

    return poms->_LuRenderThread();
}

/***************************************************************************
    AT: The midi stream playback thread.
***************************************************************************/
uint32_t OMS::_LuRenderThread(void)
{
    for (;;)
    {
        if (_fDone)
        {
            fprintf(stderr, "##### FINISH\n");
            return 0;
        }
    }
}
