/***************************************************************************
    Author: Ben Stone
    Project: Kauai
    Reviewed:

    Miniaudio stream

***************************************************************************/
#include "frame.h"
ASSERTNAME

// TODO: move this
#define AssertMaSuccess(var, msg) AssertVar(var == MA_SUCCESS, msg, &var)

#include "sndma.h"
#include "sndmapri.h"

RTCLASS(MiniaudioStream)

MiniaudioStream::~MiniaudioStream()
{
    if (_fInit)
    {
        ma_sound_uninit(&_sound);
        _fInit = fFalse;
    }

    ReleasePpo(&_pmanager);
}

PMiniaudioStream MiniaudioStream::PastreamNew(PMiniaudioManager pmanager)
{
    AssertPo(pmanager, 0);

    PMiniaudioStream pastream = pvNil;

    pastream = NewObj MiniaudioStream();
    if (pastream != pvNil)
    {
        if (!pastream->FInit(pmanager))
        {
            ReleasePpo(&pastream);
        }
    }

    return pastream;
}

bool MiniaudioStream::FWriteAudio(const void *pvframe, int32_t cframe)
{
    AssertThis(0);
    Assert(pvframe != pvNil, "no audio frames");
    Assert(_fInit, "not initialized");

    ma_result result;
    ma_uint32 cframeBuffer, iframe;
    void *pvBuffer = pvNil;

    if (!_fInit)
    {
        return fFalse;
    }

    iframe = 0;
    while (iframe < cframe)
    {
        cframeBuffer = (cframe - iframe);
        result = ma_pcm_rb_acquire_write(&_buffer, &cframeBuffer, &pvBuffer);
        AssertMaSuccess(result, "Could not acquire ring buffer for write");
        if (result != MA_SUCCESS)
        {
            break;
        }
        if (cframeBuffer == 0)
        {
            break;
        }

        ma_copy_pcm_frames(pvBuffer, ma_offset_pcm_frames_const_ptr(pvframe, iframe, _format, _cchannel), cframeBuffer,
                           _format, _cchannel);
        result = ma_pcm_rb_commit_write(&_buffer, cframeBuffer);
        AssertMaSuccess(result, "Could not commit to ring buffer");

        if (result != MA_SUCCESS)
        {
            break;
        }

        iframe += cframeBuffer;
    }

    return (iframe == cframe);
}

MiniaudioStream::MiniaudioStream()
{
    _fInit = fFalse;
    _buffer = {0};
    _cchannel = 0;
    _format = ma_format_unknown;
}

bool MiniaudioStream::FInit(PMiniaudioManager pmanager)
{
    Assert(pmanager != pvNil, "no object!");

    ma_result result;
    ma_engine *pengine;
    ma_device *pdevice;

    _pmanager = pmanager;
    _pmanager->AddRef();

    // Check we have an engine
    pengine = _pmanager->Pengine();
    if (pengine == pvNil)
    {
        return fFalse;
    }

    // Initialise the ring buffer
    pdevice = _pmanager->Pengine()->pDevice;
    _format = pdevice->playback.format;
    _cchannel = pdevice->playback.channels;

    const int32_t kcframeBuffer = 48000; // TODO: how big should this be?
    result = ma_pcm_rb_init(_format, _cchannel, kcframeBuffer, pvNil, pvNil, &_buffer);
    AssertMaSuccess(result, "Could not create ring buffer");
    if (result != MA_SUCCESS)
    {
        return fFalse;
    }

    // Create a sound from the ring buffer data source
    result = ma_sound_init_from_data_source(pengine, &_buffer, 0, pvNil, &_sound);
    AssertMaSuccess(result, "Could not create sound from ring buffer");
    if (result != MA_SUCCESS)
    {
        ma_pcm_rb_uninit(&_buffer);
        return fFalse;
    }

    _fInit = fTrue;

    SetVlm(kvlmFull);

    // Start playing
    AssertDo(FPlay(), "couldn't start playing");

    return _fInit;
}

bool MiniaudioStream::FPlay()
{
    AssertThis(0);
    Assert(_fInit, "not initialised");

    ma_result result = ma_sound_start(&_sound);
    AssertMaSuccess(result, "Failed to start audio stream");
    return (result == MA_SUCCESS);
}

bool MiniaudioStream::FStop()
{
    AssertThis(0);
    Assert(_fInit, "not initialised");

    ma_result result = ma_sound_stop(&_sound);
    AssertMaSuccess(result, "Failed to stop audio stream");
    return (result == MA_SUCCESS);
}

void MiniaudioStream::SetVlm(int32_t vlm)
{
    AssertThis(0);
    AssertIn(vlm, 0, kvlmFull * 2 + 1);
    Assert(_fInit, "not initialised");

    _vlm = vlm;
    ma_sound_set_volume(&_sound, ScaleVlm(_vlm));
}

int32_t MiniaudioStream::GetVlm()
{
    AssertThis(0);

    return _vlm;
}