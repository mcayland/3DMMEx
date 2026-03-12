/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK, Mark Cave-Ayland
    Project: Kauai
    Copyright (c) Microsoft Corporation

    MIDI stream interface: FluidSynth

***************************************************************************/
#include "frame.h"
#include "mdev2pri.h"
#include "midistreamfluidsynth.h"

#include <chrono>

ASSERTNAME

RTCLASS(OMS)

const int32_t kdtsMinSlip = kdtsSecond / 30;

/***************************************************************************
    Constructor for our own midi stream api implementation.
***************************************************************************/
OMS::OMS(PFNMIDI pfn, uintptr_t luUser)
{
    ma_device *pdevice = MiniaudioManager::Pmanager()->Pengine()->pDevice;
    char buf[256];
    int id, ret;

    _pfnCall = pfn;
    _luUser = luUser;
    _luVolSys = (uint32_t)(-1);
    _vlmBase = kvlmFull;

    _flset = new_fluid_settings();
    Assert(_flset != pvNil, "failed to create fluidsynth settings");
    fluid_settings_setnum(_flset, "synth.sample-rate", pdevice->sampleRate);

    _flsynth = new_fluid_synth(_flset);
    Assert(_flsynth != pvNil, "failed to create fluidsynth synth");

    ret = fluid_settings_copystr(_flset, "synth.default-soundfont", buf, sizeof(buf));
    if (ret == FLUID_OK)
    {
        id = fluid_synth_sfload(_flsynth, buf, true);
        if (id == FLUID_FAILED)
        {
            FNI fniExe;
            STN path;

            // Try app directory
            fniExe.FGetExe();
            fniExe.FSetLeaf(pvNil, kftgDir);
            fniExe.GetStnPath(&path);
            snprintf(buf, sizeof(buf), "%sMS Basic.sf3", path.Psz());

            id = fluid_synth_sfload(_flsynth, buf, true);
        }
        Assert(id != FLUID_FAILED, "failed to load soundfont");
    }

    ret = fluid_settings_getint(_flset, "audio.period-size", &_flframecount);
    Assert(ret == FLUID_OK, "failed to get audio.period-size");

    // Check the output format is correct
    Assert(pdevice->playback.format == ma_format_f32, "expected f32 format");
    Assert(pdevice->playback.channels == 2, "expected stereo");
}

/***************************************************************************
    Destructor for our midi stream.
***************************************************************************/
OMS::~OMS(void)
{
    int is;

    if (_hth.joinable())
    {
        _fDone = fTrue;
        _hevt.Set();
        _hth.join();
    }

    _mutx.Enter();

    if (_hthr.joinable())
    {
        _hthr.join();
    }

    Assert(_hms == hNil, "Still have an HMS");
    Assert(_pglmsb->IvMac() == 0, "Still have some buffers");
    ReleasePpo(&_pglmsb);

    delete_fluid_synth(_flsynth);
    delete_fluid_settings(_flset);

    FreePpv((void **)&_hms);

    _mutx.Leave();
}

/***************************************************************************
    Create a new OMS.
***************************************************************************/
POMS OMS::PomsNew(PFNMIDI pfn, uintptr_t luUser)
{
    POMS poms;

    if (pvNil == (poms = NewObj OMS(pfn, luUser)))
        return pvNil;

    if (!poms->_FInit())
        ReleasePpo(&poms);

    return poms;
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

    _mutx.Enter();

    // Create the stream and start playing it
    _pastream = MiniaudioStream::PastreamNew(MiniaudioManager::Pmanager());
    AssertPo(_pastream, 0);
    _hth = std::thread([this] { return this->_LuThread(); });
    _hthr = std::thread([this] { return this->_LuRenderThread(); });
    AssertDo(_pastream->FPlay(), "Could not play");

LFail:
    _mutx.Leave();

    return fTrue;
}

#ifdef DEBUG
/***************************************************************************
    Assert the validity of a OMS.
***************************************************************************/
void OMS::AssertValid(uint32_t grf)
{
    OMS_PAR::AssertValid(0);

    _mutx.Enter();
    AssertPo(_pglmsb, 0);
    _mutx.Leave();
}

/***************************************************************************
    Mark memory for the OMS.
***************************************************************************/
void OMS::MarkMem(void)
{
    OMS_PAR::MarkMem();

    _mutx.Enter();
    MarkMemObj(_pglmsb);
    MarkMemObj(_pastream);
    MarkPv(_hms);
    _mutx.Leave();
}
#endif // DEBUG

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
    if (!FAllocPv((void **)&_hms, SIZEOF(MS), fmemClear, mprNormal))
        goto LDone;

    _hms->_pastream = _pastream;
    _hms->_flsynth = _flsynth;

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

    _hms = hNil;

    _mutx.Leave();

    return fTrue;
}

/***************************************************************************
    Reset the midi device.
***************************************************************************/
void OMS::_Reset(void)
{
    Assert(hNil != _hms, 0);

    fluid_synth_all_notes_off(_hms->_flsynth, -1);
}

/***************************************************************************
    Get the system volume level.
***************************************************************************/
void OMS::_GetSysVol(void)
{
    Assert(hNil != _hms, "calling _GetSysVol with nil _hms");
}

/***************************************************************************
    Set the system volume level.
***************************************************************************/
void OMS::_SetSysVol(uint32_t luVol)
{
    Assert(hNil != _hms, "calling _SetSysVol with nil _hms");

    _hms->_pastream->SetVlm(luVol);
}

/***************************************************************************
    Set the system volume level from the current values of _vlmBase
    and _luVolSys. We set the system volume to the result of scaling
    _luVolSys by _vlmBase.
***************************************************************************/
void OMS::_SetSysVlm(void)
{
    uint32_t luVol;

    luVol = LuVolScale(_luVolSys, _vlmBase);
    _SetSysVol(luVol);
}

/***************************************************************************
    Set the volume for the midi stream output device.
***************************************************************************/
void OMS::SetVlm(int32_t vlm)
{
    AssertThis(0);

    if (vlm != _vlmBase)
    {
        _vlmBase = vlm;
        if (hNil != _hms)
            _SetSysVlm();
    }
}

/***************************************************************************
    Get the current volume.
***************************************************************************/
int32_t OMS::VlmCur(void)
{
    AssertThis(0);

    return _vlmBase;
}

/***************************************************************************
    Return whether the midi stream output device is active.
***************************************************************************/
bool OMS::FActive(void)
{
    return hNil != _hms;
}

/***************************************************************************
    Activate or deactivate the Midi stream output object.
***************************************************************************/
bool OMS::FActivate(bool fActivate)
{
    AssertThis(0);

    return fActivate ? _FOpen() : _FClose();
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
        _fChanged = fTrue;
        _hevt.Set();
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
        _hevt.Set();
        _fChanged = fTrue;
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
        fChanged = (dtsWait > 0 && _hevt.Wait(dtsWait == klwInfinite ? SDL_MUTEX_MAXWAIT : dtsWait));

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
                {
                    switch (_pmev->dwEvent & 0xf0)
                    {
                    case 0x80: /* Note off */
                        fluid_synth_noteoff(_flsynth, _pmev->dwEvent & 0xf, (_pmev->dwEvent & 0x7f00) >> 8);
                        break;

                    case 0x90: /* Note on */
                        fluid_synth_noteon(_flsynth, _pmev->dwEvent & 0xf, (_pmev->dwEvent & 0x7f00) >> 8,
                                           (_pmev->dwEvent & 0x7f0000) >> 16);
                        break;

                    case 0xb0: /* Control change */
                        fluid_synth_cc(_flsynth, _pmev->dwEvent & 0xf, (_pmev->dwEvent & 0x7f00) >> 8,
                                       (_pmev->dwEvent & 0x7f0000) >> 16);
                        break;

                    case 0xc0: /* Program change */
                        fluid_synth_program_change(_flsynth, _pmev->dwEvent & 0xf, (_pmev->dwEvent & 0x7f00) >> 8);
                        break;

                    case 0xd0: /* Channel pressure */
                        fluid_synth_channel_pressure(_flsynth, _pmev->dwEvent & 0xf, (_pmev->dwEvent & 0x7f00) >> 8);
                        break;

                    case 0xe0: /* Pitch wheel */
                        fluid_synth_pitch_bend(_flsynth, _pmev->dwEvent & 0xf,
                                               ((_pmev->dwEvent & 0x7f00) >> 8) | ((_pmev->dwEvent & 0x7f0000) >> 9));
                        break;
                    }
                }

                _pmev++;
                if (_pmev >= _pmevLim)
                {
                    dtsWait = 0;
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
            }
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
    float *flFrame = pvNil;

    if (!FAllocPv((void **)&flFrame, SIZEOF(float) * _flframecount * 2, fmemClear, mprNormal))
        goto LFail;

    for (;;)
    {
        SDL_Event event;

        while (_pastream->FGetPendingFrames() < 8192)
        {
            if (_fDone)
                return 0;

            if (!_fStop)
            {
                fluid_synth_write_float(_flsynth, _flframecount, flFrame, 0, 2, flFrame, 1, 2);
                _pastream->FWriteAudio(flFrame, _flframecount);
            }
        }

        if (_fDone)
        {
            _hevt.Set();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

LFail:
    FreePpv((void **)flFrame);

    return 0;
}

/***************************************************************************
    Release all buffers up to _imsbCur. Assumes that we have the mutx
    checked out exactly once.
***************************************************************************/
void OMS::_ReleaseBuffers(void)
{
    MSB msb;

    if (_imsbCur >= _pglmsb->IvMac() && hNil != _hms)
        _Reset();

    while (_imsbCur > 0)
    {
        _pglmsb->Get(0, &msb);
        _pglmsb->Delete(0);
        _imsbCur--;

        _mutx.Leave();

        // call the notify proc
        (*_pfnCall)(_luUser, msb.pvData, msb.luData);

        _mutx.Enter();
    }
}
