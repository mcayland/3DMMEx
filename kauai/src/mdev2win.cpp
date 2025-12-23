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

/***************************************************************************
    Constructor for the midi stream output object.
***************************************************************************/
MSMIX::MSMIX(void)
{
    _vlmBase = kvlmFull;
    _vlmSound = kvlmFull;
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
        SetEvent(_hevt);
        WaitForSingleObject(_hth, INFINITE);
    }

    if (hNil != _hevt)
        CloseHandle(_hevt);

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
    DWORD luThread;

    if (pvNil == (_pglmsos = GL::PglNew(SIZEOF(MSOS))))
        return fFalse;
    _pglmsos->SetMinGrow(1);

    if (pvNil == (_pmisi = WMS::PwmsNew(_MidiProc, (uintptr_t)this)) &&
        pvNil == (_pmisi = OMS::PomsNew(_MidiProc, (uintptr_t)this)))
    {
        return fFalse;
    }

    if (hNil == (_hevt = CreateEvent(pvNil, fFalse, fFalse, pvNil)))
        return fFalse;

    // create the thread
    if (hNil == (_hth = CreateThread(pvNil, 1024, MSMIX::_ThreadProc, this, 0, &luThread)))
    {
        return fFalse;
    }

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

    while (_cpvOut > 0)
        Sleep(0);

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
        }
        _SubmitBuffers(tsCur);
    }

    // signal the aux thread that the list changed
    SetEvent(_hevt);
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

    pmsmix->_Notify(pvData, pmdws);
}

/***************************************************************************
    AT: Static method. Thread function for the MSMIX object.
***************************************************************************/
DWORD __stdcall MSMIX::_ThreadProc(LPVOID pv)
{
    PMSMIX pmsmix = (PMSMIX)pv;

    AssertPo(pmsmix, 0);

    return pmsmix->_LuThread();
}

/***************************************************************************
    AT: This thread just sleeps until the next sound is due to expire, then
    wakes up and nukes any expired sounds.
***************************************************************************/
DWORD MSMIX::_LuThread(void)
{
    AssertThis(0);
    uint32_t tsCur;
    int32_t imsos;
    MSOS msos;
    int32_t cactSkip;
    uint32_t dtsNextStop = kluMax;

    for (;;)
    {
        WaitForSingleObject(_hevt, dtsNextStop);

        if (_fDone)
            return 0;

        _mutx.Enter();

        if (_fWaiting)
        {
            // we're waiting for buffers to be returned, so don't touch
            // anything!
            dtsNextStop = 1;
        }
        else
        {
            // See if any sounds have expired...
            tsCur = TsCurrentSystem();
            dtsNextStop = kluMax;
            for (imsos = _pglmsos->IvMac(); imsos-- > 0;)
            {
                if (imsos == 0 && _fPlaying)
                    break;
                _pglmsos->Get(imsos, &msos);

                cactSkip = (tsCur - msos.tsStart) / msos.dts;
                if (cactSkip > 0)
                {
                    uint32_t dtsSeek;

                    if (msos.cactPlay > 0 && (msos.cactPlay -= cactSkip) <= 0)
                    {
                        // this sound is done
                        _pglmsos->Delete(imsos);
                        _mutx.Leave();

                        // do the notify
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
    int32_t iv;

    midiOutReset(_hms);

    // Reset channel pressure and pitch wheel on all channels.
    // We shouldn't have to do this, but some drivers don't reset these.
    for (iv = 0; iv < 16; iv++)
    {
        midiOutShortMsg(_hms, 0xD0 | iv);
        midiOutShortMsg(_hms, 0x004000E0 | iv);
    }
}

/***************************************************************************
    Get the system volume level.
***************************************************************************/
void MISI::_GetSysVol(void)
{
    Assert(hNil != _hms, "calling _GetSysVol with nil _hms");
    DWORD lu0, lu1, lu2;

    switch (_tBogusDriver)
    {
    case tYes:
        // just use vluSysVolFake...
        _luVolSys = vluSysVolFake;
        return;

    case tMaybe:
        // need to determine if midiOutGetVolume really works for this
        // driver.

        // Some drivers will only ever tell us what we last gave them -
        // irregardless of what the user has set the value to. Those drivers
        // will always give us full volume the first time we ask.

        // We also look for drivers that give us nonsense values.

        if (0 != midiOutGetVolume(_hms, &_luVolSys) || _luVolSys == ULONG_MAX || 0 != midiOutSetVolume(_hms, 0ul) ||
            0 != midiOutGetVolume(_hms, &lu0) || 0 != midiOutSetVolume(_hms, 0x7FFF7FFFul) ||
            0 != midiOutGetVolume(_hms, &lu1) || 0 != midiOutSetVolume(_hms, 0xFFFFFFFFul) ||
            0 != midiOutGetVolume(_hms, &lu2) || lu0 >= lu1 || lu1 >= lu2)
        {
            _tBogusDriver = tYes;
            _luVolSys = vluSysVolFake;
        }
        else
        {
            _tBogusDriver = tNo;
            vluSysVolFake = _luVolSys;
        }
        midiOutSetVolume(_hms, _luVolSys);
        break;

    default:
        if (0 != midiOutGetVolume(_hms, &_luVolSys))
        {
            // failed - use the fake value
            _luVolSys = vluSysVolFake;
        }
        else
            vluSysVolFake = _luVolSys;
        break;
    }
}

/***************************************************************************
    Set the system volume level.
***************************************************************************/
void MISI::_SetSysVol(uint32_t luVol)
{
    Assert(hNil != _hms, "calling _SetSysVol with nil _hms");
    midiOutSetVolume(_hms, DWORD(luVol));
}

/***************************************************************************
    Destructor for the Win95 Midi stream class.
***************************************************************************/
WMS::~WMS(void)
{
    if (hNil != _hth)
    {
        // tell the thread to end and wait for it to finish
        _fDone = fTrue;
        SetEvent(_hevt);
        WaitForSingleObject(_hth, INFINITE);
    }

    if (hNil != _hevt)
        CloseHandle(_hevt);

    if (pvNil != _pglpmsir)
    {
        Assert(0 == _pglpmsir->IvMac(), "WMS still has some active buffers");
        ReleasePpo(&_pglpmsir);
    }
    if (hNil != _hlib)
    {
        FreeLibrary(_hlib);
        _hlib = hNil;
    }
}

/***************************************************************************
    Initialize the WMS: get the addresses of the stream API.
***************************************************************************/
bool WMS::_FInit(void)
{
    OSVERSIONINFO osv;
    DWORD luThread;

    // Make sure we're on Win95 and not NT, since the API exists on NT 3.51
    // but it fails.
    osv.dwOSVersionInfoSize = SIZEOF(osv);
    if (!GetVersionEx(&osv))
        return fFalse;

// Old header files don't have this defined!
#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS 1
#endif //! VER_PLATFORM_WIN32_WINDOWS

    if (VER_PLATFORM_WIN32_WINDOWS != osv.dwPlatformId)
    {
        // don't bother trying - NT's scheduler works fine anyway.
        return fFalse;
    }

    if (hNil == (_hlib = LoadLibrary(PszLit("WINMM.DLL"))))
        return fFalse;

#define _Get(n)                                                                                                        \
    if (pvNil == (*(void **)&_pfn##n = (void *)GetProcAddress(_hlib, "midiStream" #n)))                                \
    {                                                                                                                  \
        return fFalse;                                                                                                 \
    }

    _Get(Open);
    _Get(Close);
    _Get(Property);
    _Get(Position);
    _Get(Out);
    _Get(Pause);
    _Get(Restart);
    _Get(Stop);

#undef _Get

    if (pvNil == (_pglpmsir = GL::PglNew(SIZEOF(PMSIR))))
        return fFalse;
    _pglpmsir->SetMinGrow(1);

    if (hNil == (_hevt = CreateEvent(pvNil, fFalse, fFalse, pvNil)))
        return fFalse;

    // create the thread
    if (hNil == (_hth = CreateThread(pvNil, 1024, WMS::_ThreadProc, this, 0, &luThread)))
    {
        return fFalse;
    }

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

    // MIDIPROPTIMEDIV struct
    struct MT
    {
        DWORD cbStruct;
        DWORD dwTimeDiv;
    };

    MT mt;
    UINT uT = MIDI_MAPPER;

    _mutx.Enter();

    if (hNil != _hms)
        goto LDone;

    if (MMSYSERR_NOERROR != (*_pfnOpen)(&_hms, &uT, 1, (uintptr_t)_MidiProc, (uintptr_t)this, CALLBACK_FUNCTION))
    {
        goto LFail;
    }

    // We set the time division to 1000 ticks per beat, so clients can
    // use 1 beat per second and just use milliseconds for timing.
    // We also un-pause the stream.
    mt.cbStruct = SIZEOF(mt);
    mt.dwTimeDiv = 1000;

    if (MMSYSERR_NOERROR != (*_pfnProperty)(_hms, (uint8_t *)&mt, MIDIPROP_SET | MIDIPROP_TIMEDIV))
    {
        (*_pfnClose)(_hms);
    LFail:
        _hms = hNil;
        _mutx.Leave();
        return fFalse;
    }

    // we know there are no buffers submitted
    AssertVar(_cmhOut == 0, "why is _cmhOut non-zero?", &_cmhOut);
    _cmhOut = 0;

    // get the system volume level
    _GetSysVol();

    // set our volume level
    _SetSysVlm();

LDone:
    _mutx.Leave();

    return fTrue;
}

/***************************************************************************
    Close the midi stream.
***************************************************************************/
bool WMS::_FClose(void)
{
    AssertThis(0);

    _mutx.Enter();

    if (hNil == _hms)
    {
        _mutx.Leave();
        return fTrue;
    }

    if (0 < _cmhOut)
    {
        BugVar("closing a stream that still has buffers!", &_cmhOut);
        _mutx.Leave();
        return fFalse;
    }

    // reset the device
    _Reset();

    // restore the volume level
    _SetSysVol(_luVolSys);

    // free the device
    (*_pfnClose)(_hms);
    _hms = hNil;

    _mutx.Leave();

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
    if (!FActive())
        return;

#ifdef STREAM_BUG
    if (hNil == _hms)
        _FOpen();
    else
    {
        (*_pfnStop)(_hms);
        _FClose();
        _FOpen();
    }
#else  //! STREAM_BUG
    (*_pfnStop)(_hms);
    _Reset();
#endif //! STREAM_BUG
}

/***************************************************************************
    Prepare and submit the given buffer. Assumes the mutx is ours.
***************************************************************************/
bool WMS::_FSubmit(PMH pmh)
{
    bool fRestart = (0 == _cmhOut);

    if (hNil == _hms)
        return fFalse;

    // prepare and submit the buffer
    if (MMSYSERR_NOERROR != midiOutPrepareHeader(_hms, (PMHO)pmh, sizeof(*pmh)))
        return fFalse;

    if (MMSYSERR_NOERROR != (*_pfnOut)(_hms, (PMHO)pmh, SIZEOF(*pmh)))
    {
        midiOutUnprepareHeader(_hms, (PMHO)pmh, SIZEOF(*pmh));
        return fFalse;
    }
    _cmhOut++;

    if (fRestart)
        (*_pfnRestart)(_hms);

    return fTrue;
}

/***************************************************************************
    Stop the midi stream.
***************************************************************************/
void WMS::StopPlaying(void)
{
    AssertThis(0);

    _mutx.Enter();

    if (hNil != _hms && _cmhOut > 0)
    {
        (*_pfnStop)(_hms);
        _ipmsirCur = _pglpmsir->IvMac();
    }

    _mutx.Leave();
}

/***************************************************************************
    Call back from the midi stream. If the number of active buffers returns
    to 0, this stops the midi stream. If the indicated sound is done,
    we notify the client.
***************************************************************************/
void __stdcall WMS::_MidiProc(HMS hms, UINT msg, DWORD_PTR luUser, DWORD_PTR lu1, DWORD_PTR lu2)
{
    PWMS pwms;
    PMH pmh;

    if (msg != MOM_DONE)
        return;

    pwms = (PWMS)luUser;
    AssertPo(pwms, 0);
    pmh = (PMH)lu1;
    AssertVarMem(pmh);
    pwms->_Notify(hms, pmh);
}

/***************************************************************************
    The this-based callback.

    The mmsys guys claim that it's illegal to call midiOutUnprepareHeader,
    midiStreamStop and midiOutReset. So we just signal another thread to
    do this work.
***************************************************************************/
void WMS::_Notify(HMS hms, PMH pmh)
{
    AssertThis(0);
    Assert(hNil != hms, 0);
    AssertVarMem(pmh);

    PMSIR pmsir;
    int32_t imh;

    _mutx.Enter();

    Assert(hms == _hms, "wrong hms");

    midiOutUnprepareHeader(hms, (PMHO)pmh, SIZEOF(*pmh));
    pmsir = (PMSIR)pmh->dwUser;
    AssertVarMem(pmsir);
    AssertPvCb(pmsir->pvData, pmsir->cb);

    for (imh = 0;; imh++)
    {
        if (imh >= kcmhMsir)
        {
            Bug("corrupt msir");
            _mutx.Leave();
            return;
        }

        if (pmh == &pmsir->rgmh[imh])
            break;
    }

    Assert(pmh->lpData == PvAddBv(pmsir->pvData, pmsir->rgibLim[imh] - pmh->dwBufferLength), "pmh->lpData is wrong");

    // mark this buffer free
    pmsir->rgibLim[imh] = 0;

    // fill and submit buffers
    _CmhSubmitBuffers();

    // update the submitted buffer count
    --_cmhOut;

    // wake up the auxillary thread to do callbacks and stop and reset
    // the device it there's nothing more to play
    SetEvent(_hevt);

    _mutx.Leave();
}

/***************************************************************************
    AT: Static method. Thread function for the WMS object. This thread
    just waits for the event to be triggered, indicating that we got
    a callback from the midiStream stuff and it's time to do our callbacks.
***************************************************************************/
DWORD __stdcall WMS::_ThreadProc(LPVOID pv)
{
    PWMS pwms = (PWMS)pv;

    AssertPo(pwms, 0);

    return pwms->_LuThread();
}

/***************************************************************************
    AT: This thread just sleeps until the next sound is due to expire, then
    wakes up and nukes any expired sounds.
***************************************************************************/
DWORD WMS::_LuThread(void)
{
    AssertThis(0);

    for (;;)
    {
        WaitForSingleObject(_hevt, INFINITE);

        if (_fDone)
            return 0;

        _mutx.Enter();

        if (hNil != _hms && 0 == _cmhOut)
            _ResetStream();

        _DoCallBacks();

        _mutx.Leave();
    }
}

/***************************************************************************
    Constructor for our own midi stream api implementation.
***************************************************************************/
OMS::OMS(PFNMIDI pfn, uintptr_t luUser) : MISI(pfn, luUser)
{
}

/***************************************************************************
    Destructor for our midi stream.
***************************************************************************/
OMS::~OMS(void)
{
    if (hNil != _hth)
    {
        // tell the thread to end and wait for it to finish
        _fDone = fTrue;
        SetEvent(_hevt);
        WaitForSingleObject(_hth, INFINITE);
    }

    _mutx.Enter();

    if (hNil != _hevt)
        CloseHandle(_hevt);

    Assert(_hms == hNil, "Still have an HMS");
    Assert(_pglmsb->IvMac() == 0, "Still have some buffers");
    ReleasePpo(&_pglmsb);

    _mutx.Leave();
}

/***************************************************************************
    Initialize the OMS.
***************************************************************************/
bool OMS::_FInit(void)
{
    AssertBaseThis(0);
    DWORD luThread;

    if (pvNil == (_pglmsb = GL::PglNew(SIZEOF(MSB))))
        return fFalse;
    _pglmsb->SetMinGrow(1);

    if (hNil == (_hevt = CreateEvent(pvNil, fFalse, fFalse, pvNil)))
        return fFalse;

    // create the thread in a suspended state
    if (hNil == (_hth = CreateThread(pvNil, 1024, OMS::_ThreadProc, this, CREATE_SUSPENDED, &luThread)))
    {
        return fFalse;
    }

    SetThreadPriority(_hth, THREAD_PRIORITY_TIME_CRITICAL);

    // start the thread
    ResumeThread(_hth);

    return fTrue;
}

/***************************************************************************
    Open the stream.
***************************************************************************/
bool OMS::_FOpen(void)
{
    AssertThis(0);

    _mutx.Enter();

    if (hNil != _hms)
        goto LDone;

    _fChanged = _fStop = fFalse;
    if (MMSYSERR_NOERROR != midiOutOpen(&_hms, MIDI_MAPPER, 0, 0, CALLBACK_NULL))
    {
        _hms = hNil;
        _mutx.Leave();
        return fFalse;
    }

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

    midiOutClose(_hms);
    _hms = hNil;

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
        SetEvent(_hevt);
        _fChanged = fTrue;
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
        _fStop = fTrue;
        SetEvent(_hevt);
        _fChanged = fTrue;
    }

    _mutx.Leave();
}

/***************************************************************************
    AT: Static method. Thread function for the midi stream object.
***************************************************************************/
DWORD __stdcall OMS::_ThreadProc(void *pv)
{
    POMS poms = (POMS)pv;

    AssertPo(poms, 0);

    return poms->_LuThread();
}

/***************************************************************************
    AT: The midi stream playback thread.
***************************************************************************/
DWORD OMS::_LuThread(void)
{
    AssertThis(0);
    MSB msb;
    bool fChanged; // whether the event went off
    uint32_t tsCur;
    const int32_t klwInfinite = klwMax;
    int32_t dtsWait = klwInfinite;

    for (;;)
    {
        fChanged =
            dtsWait > 0 && WAIT_TIMEOUT != WaitForSingleObject(_hevt, dtsWait == klwInfinite ? INFINITE : dtsWait);

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
                if (MEVT_SHORTMSG == (_pmev->dwEvent >> 24))
                    midiOutShortMsg(_hms, _pmev->dwEvent & 0x00FFFFFF);

                _pmev++;
                if (_pmev >= _pmevLim)
                    dtsWait = 0;
                else
                {
                    uint32_t tsNew = TsCurrentSystem();

                    tsCur += _pmev->dwDeltaTime;
                    dtsWait = tsCur - tsNew;
                    if (dtsWait < -kdtsMinSlip)
                    {
                        tsCur = tsNew;
                        dtsWait = 0;
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
                dtsWait = 0;
            else
            {
                dtsWait = _pmev->dwDeltaTime;
                tsCur = TsCurrentSystem() + dtsWait;
            }
        }
    LLoop:
        _mutx.Leave();
    }
}
