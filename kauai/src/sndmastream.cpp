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

PMiniaudioStream MiniaudioStream::PastreamNew(PMiniaudioManager pmanager, ma_format format, ma_uint32 cchannel,
                                              ma_uint32 csample)
{
    AssertPo(pmanager, 0);

    PMiniaudioStream pastream = pvNil;

    pastream = NewObj MiniaudioStream();
    if (pastream != pvNil)
    {
        if (!pastream->FInit(pmanager, format, cchannel, csample))
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
        //fprintf(stderr, " -- before cframebuffer %d\n", cframeBuffer);
        result = ma_pcm_rb_acquire_write(&_buffer, &cframeBuffer, &pvBuffer);
        //fprintf(stderr, " -- after RB addr %p, cframebuffer %d, distance %d\n", pvBuffer, cframeBuffer, ma_pcm_rb_pointer_distance(&_buffer));
        AssertMaSuccess(result, "Could not acquire ring buffer for write");
        if (result != MA_SUCCESS)
        {
            break;
        }
        if (cframeBuffer == 0)
        {
            break;
        }

        ma_copy_pcm_frames(pvBuffer, ma_offset_pcm_frames_const_ptr(pvframe, iframe, _buffer.format, _buffer.channels),
                           cframeBuffer, _buffer.format, _buffer.channels);
        result = ma_pcm_rb_commit_write(&_buffer, cframeBuffer);
        AssertMaSuccess(result, "Could not commit to ring buffer");

        if (result != MA_SUCCESS)
        {
            break;
        }

        iframe += cframeBuffer;
        //fprintf(stderr, "    ~~~~~ ERROR: iframe: %d, cframe: %d\n", iframe, cframe);
    }

    //if (iframe != cframe)
    //{
    //    fprintf(stderr, "    ~~~~~ ERROR: iframe: %d, cframe: %d\n", iframe, cframe);
    //}

    return (iframe == cframe);
}

int MiniaudioStream::FGetPendingFrames()
{
    return ma_pcm_rb_pointer_distance(&_buffer);
}

MiniaudioStream::MiniaudioStream()
{
    _fInit = fFalse;
    _vlm = vluSysVolFake;
    _buffer = {0};
}

bool MiniaudioStream::FInit(PMiniaudioManager pmanager, ma_format format, ma_uint32 cchannel, ma_uint32 csample)
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

    if (format == ma_format_unknown)
        format = pdevice->playback.format;

    cchannel = cchannel;
    if (cchannel == 0)
        cchannel = pdevice->playback.channels;

    // Default to one second of audio
    if (csample == 0)
        csample = pdevice->sampleRate;

    result = ma_pcm_rb_init(format, cchannel, 16384, pvNil, pvNil, &_buffer);
    AssertMaSuccess(result, "Could not create ring buffer");
    if (result != MA_SUCCESS)
    {
        return fFalse;
    }

    // Create a sound from the ring buffer data source
    result = ma_sound_init_from_data_source(pengine, &_buffer, MA_SOUND_FLAG_LOOPING, pvNil, &_sound);
    //ma_sound_init_from_file(pengine, "/home/mca/3dmm/Microsoft-3D-Movie-Maker/src/studio/sound/tooltips/vzr018.wav", 0, NULL, NULL, &_sound);
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
    fprintf(stderr, "PPPPPPLLLLLLLL\n");
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

void MiniaudioStream::SetVlm(uint32_t vlm)
{
    AssertThis(0);
    Assert(_fInit, "not initialised");

    float fvlm;

    _vlm = vlm;
    fvlm = ScaleVlm(_vlm & 0xffff);
    ma_sound_set_volume(&_sound, fvlm);
}

ma_uint32 MiniaudioStream::Cchannel()
{
    AssertThis(0);
    return _buffer.channels;
}

ma_format MiniaudioStream::Format()
{
    AssertThis(0);
    return _buffer.format;
}

ma_uint32 MiniaudioStream::SampleRate()
{
    AssertThis(0);
    return _buffer.sampleRate;
}

uint32_t MiniaudioStream::GetVlm()
{
    AssertThis(0);
    
    float fvlm = ma_sound_get_volume(&_sound);

    return _vlm;
}
