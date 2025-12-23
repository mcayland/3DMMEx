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
}

/***************************************************************************
    Destructor for the midi stream output object.
***************************************************************************/
MSMIX::~MSMIX(void)
{
#if 0
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
#endif
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

    if (pvNil == (_pmisi = WMS::PwmsNew(_MidiProc, (uintptr_t)this)) &&
        pvNil == (_pmisi = OMS::PomsNew(_MidiProc, (uintptr_t)this)))
    {
        return fFalse;
    }

    _hevt = (void *)-1;
    _hth = (void *)-1;

    return fTrue;
}

/***************************************************************************
    If we're currently playing a midi stream stop it. Assumes the mutx is
    already checked out exactly once.
***************************************************************************/
void MSMIX::_StopStream(void)
{
    AssertThis(0);
}

/***************************************************************************
    The sound list changed so make sure we're playing the first tune.
    Assumes the mutx is already checked out.
***************************************************************************/
void MSMIX::_Restart(bool fNew)
{
    AssertThis(0);
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
    Reset the midi device.
***************************************************************************/
void MISI::_Reset(void)
{
    //Assert(hNil != _hms, 0);
}

/***************************************************************************
    Set the system volume level.
***************************************************************************/
void MISI::_SetSysVol(uint32_t luVol)
{
    //Assert(hNil != _hms, "calling _SetSysVol with nil _hms");
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

    return fTrue;
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
}

/***************************************************************************
    Destructor for our midi stream.
***************************************************************************/
OMS::~OMS(void)
{
}

/***************************************************************************
    Initialize the OMS.
***************************************************************************/
bool OMS::_FInit(void)
{
    AssertBaseThis(0);

    return fTrue;
}

/***************************************************************************
    Open the stream.
***************************************************************************/
bool OMS::_FOpen(void)
{
    AssertThis(0);

    return fTrue;
}

/***************************************************************************
    Close the stream.
***************************************************************************/
bool OMS::_FClose(void)
{
    AssertThis(0);

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
    //Assert(cb % SIZEOF(MEV) == 0, "bad cb");
    //Assert(ibStart % SIZEOF(MEV) == 0, "bad cb");

    return fTrue;
}

/***************************************************************************
    Stop the stream and release all buffers. The buffer notifies are
    asynchronous.
***************************************************************************/
void OMS::StopPlaying(void)
{
    AssertThis(0);
}
