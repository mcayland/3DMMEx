/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: Mark Cave-Ayland
    Project: Kauai

    Graphical video object implementation stub.

***************************************************************************/

#include "frame.h"
#include "gfx.h"

#include <thread>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include <glib.h>
#include <SDL2/SDL.h>

ASSERTNAME

RTCLASS(GVID)
RTCLASS(GVDS)
RTCLASS(GVDW)

BEGIN_CMD_MAP_BASE(GVDS)
END_CMD_MAP(&GVDS::FCmdAll, pvNil, kgrfcmmAll)

const int32_t kcmhlGvds = kswMin; // put videos at the head of the list

#define GVDW_LUTHREAD_SLEEP_DELAY 5

PGVID GVID::PgvidNew(PFNI pfni, PGOB pgobBase, bool fHwndBased, int32_t hid)
{
    AssertPo(pfni, ffniFile);
    AssertPo(pgobBase, 0);

    if (fHwndBased)
        return GVDW::PgvdwNew(pfni, pgobBase, hid);
    return GVDS::PgvdsNew(pfni, pgobBase, hid);
}

GVID::GVID(int32_t hid) : GVID_PAR(hid)
{
    AssertBaseThis(0);
}

GVDS::GVDS(int32_t hid) : GVDS_PAR(hid)
{
    AssertBaseThis(0);
}

GVDS::~GVDS(void)
{
    AssertBaseThis(0);
}

bool GVDS::_FInit(PFNI pfni, PGOB pgobBase)
{
    AssertBaseThis(0);
    AssertPo(pfni, ffniFile);
    AssertPo(pgobBase, 0);

    // Not implemented: fail
    RawRtn();
    return fFalse;
}

PGVDS GVDS::PgvdsNew(PFNI pfni, PGOB pgobBase, int32_t hid)
{
    AssertPo(pfni, ffniFile);
    PGVDS pgvds;

    if (hid == hidNil)
        hid = CMH::HidUnique();

    if (pvNil == (pgvds = NewObj GVDS(hid)))
        return pvNil;

    if (!pgvds->_FInit(pfni, pgobBase))
    {
        ReleasePpo(&pgvds);
        return pvNil;
    }

    return pgvds;
}

int32_t GVDS::NfrMac(void)
{
    AssertThis(0);
    return _nfrMac;
}

int32_t GVDS::NfrCur(void)
{
    AssertThis(0);
    return _nfrCur;
}

void GVDS::GotoNfr(int32_t nfr)
{
    AssertThis(0);
    AssertIn(nfr, 0, _nfrMac);

    Stop();
    _nfrCur = nfr;
}

bool GVDS::FPlaying(void)
{
    AssertThis(0);
    return _fPlaying;
}

bool GVDS::FPlay(RC *prc)
{
    AssertThis(0);
    AssertNilOrVarMem(prc);

    Stop();
    if (!vpcex->FAddCmh(this, kcmhlGvds, kgrfcmmAll))
        return fFalse;

    SetRcPlay(prc);
    _fPlaying = fTrue;

    return fTrue;
}

void GVDS::SetRcPlay(RC *prc)
{
    AssertThis(0);
    AssertNilOrVarMem(prc);

    if (pvNil == prc)
        _rcPlay.Set(0, 0, _dxp, _dyp);
    else
        _rcPlay = *prc;
}

void GVDS::Stop(void)
{
    AssertThis(0);

    vpcex->RemoveCmh(this, kcmhlGvds);
    _fPlaying = fFalse;
}

bool GVDS::FCmdAll(PCMD pcmd)
{
    AssertThis(0);
    AssertVarMem(pcmd);

    if (!_fPlaying)
    {
        Stop();
        return fFalse;
    }

    // TODO: update _nfrCur
    _nfrCur = 0;

    if (_nfrCur != _nfrMarked)
    {
        _pgobBase->InvalRc(&_rcPlay, kginMark);
        _nfrMarked = _nfrCur;
    }
    if (_nfrCur >= _nfrMac - 1)
        Stop();

    return fFalse;
}

void GVDS::Draw(PGNV pgnv, RC *prc)
{
    AssertThis(0);
    AssertPo(pgnv, 0);
    AssertVarMem(prc);
    RC rc;
}

void GVDS::GetRc(RC *prc)
{
    AssertThis(0);
    AssertVarMem(prc);

    prc->Set(0, 0, _dxp, _dyp);
}

#ifdef DEBUG
void GVDS::AssertValid(uint32_t grf)
{
    GVDS_PAR::AssertValid(0);
    AssertPo(_pgobBase, 0);
    // REVIEW shonk: fill in GVDS::AssertValid
}
#endif // DEBUG

PGVDW GVDW::PgvdwNew(PFNI pfni, PGOB pgobBase, int32_t hid)
{
    AssertPo(pfni, ffniFile);
    PGVDW pgvdw;

    if (hid == hidNil)
        hid = CMH::HidUnique();

    if (pvNil == (pgvdw = NewObj GVDW(hid)))
        return pvNil;

    if (!pgvdw->_FInit(pfni, pgobBase))
    {
        ReleasePpo(&pgvdw);
        return pvNil;
    }

    return pgvdw;
}

GVDW::GVDW(int32_t hid) : GVDW_PAR(hid)
{
    AssertBaseThis(0);
}

GVDW::~GVDW(void)
{
    AssertBaseThis(0);

    if (_hth.joinable())
    {
        _fDone = fTrue;
        _nscb.hevt.Set();
        _hth.join();
    }
    _pastream->FStop();
    ReleasePpo(&_pastream);
    SDL_FreeSurface(_surface);
}

bool GVDW::_FInit(PFNI pfni, PGOB pgobBase)
{
    AssertPo(pfni, ffniFile);
    AssertPo(pgobBase, 0);

    STN stnPath;
    STN stn;
    GError *error = NULL;
    GstElement *pipeline;
    GstSample *sample = NULL;
    GstCaps *caps = NULL;
    GstElement *sink = NULL;
    GstElement *xsrc = NULL;
    GstPad *pad = NULL;
    GstStructure *structure = NULL;
    GstQuery *query = NULL;
    GstState state;
    GstState pending;
    g_autofree gchar *uri = NULL;
    g_autofree gchar *desc = NULL;
    gint gint_val;
    gint frame_n;
    gint frame_d;
    gint64 duration;
    int wx;
    int wy;
    int res;
    RC rc;
    ma_device *pdevice;

    _pgobBase = pgobBase;
    pfni->GetStnPath(&stnPath);

    gst_init(NULL, NULL);

    pdevice = MiniaudioManager::Pmanager()->Pengine()->pDevice;

    // Check the output format is correct
    Assert(pdevice->playback.format == ma_format_f32, "expected f32 format");
    Assert(pdevice->playback.channels == 2, "expected stereo");

    uri = g_uri_escape_string(stnPath.Psz(), "/", TRUE);
    _desc = g_strdup_printf("uridecodebin uri=file://%s name=u ! videoconvert ! videoscale !"
      " appsink name=vsink caps=\"video/x-raw,format=BGRA,pixel-aspect-ratio=1/1\"" //, uri);
      " u. ! audioconvert ! audioresample ! appsink name=asink caps=\"audio/x-raw,format=F32LE,rate=%d,channels=%d,layout=interleaved\"",
      uri, pdevice->playback.converter.sampleRateOut, pdevice->playback.channels);
    fprintf(stderr, "file is %s\n", uri);
    fprintf(stderr, "desc is %s\n", _desc);

    pipeline = gst_parse_launch(_desc, &error);
    if (error != NULL) {
        goto LFail;
    }

    /* Find width, height and frame rate */
    gst_element_set_state(pipeline, GST_STATE_PAUSED);
    sink = gst_bin_get_by_name(GST_BIN(pipeline), "vsink");
    g_signal_emit_by_name(sink, "pull-preroll", &sample, NULL);
    caps = gst_sample_get_caps(sample);
    if (!caps) {
        return fFalse;
    }
    structure = gst_caps_get_structure(caps, 0);

    res = gst_structure_get_int(structure, "width", &gint_val);
    if (!res)
    {
        return fFalse;
    }
    _dxp = gint_val;
    res = gst_structure_get_int(structure, "height", &gint_val);
    if (!res)
    {
        return fFalse;
    }
    _dyp = gint_val;

    /* Determine the total number of frames in the file */
    query = gst_query_new_duration(GST_FORMAT_DEFAULT);
    res = gst_element_query(pipeline, query);
    if (!res)
    {
        return fFalse;
    }
    gst_query_parse_duration(query, NULL, &duration);
    _nfrMac = duration;

    /* Create surface */
    _surface = SDL_CreateRGBSurface(0, _dxp, _dyp, 32, 0, 0, 0, 0);

    // Create the stream and start playing it
    _pastream = MiniaudioStream::PastreamNew(MiniaudioManager::Pmanager());
    AssertPo(_pastream, 0);
    AssertDo(_pastream->FPlay(), "Could not play");

    _hth = std::thread([this] { return this->_LuThread(); });

    return fTrue;

LFail:
    return fFalse;
}

static void
new_video_sample(GstAppSink *vsink, NSCB *nscb)
{
    /* Signal video render thread */
    nscb->vsample = gst_app_sink_pull_sample(vsink);
    nscb->hevt.Set();
}

static void
new_audio_sample(GstAppSink *asink, NSCB *nscb)
{
    /* Signal video render thread */
    nscb->asample = gst_app_sink_pull_sample(asink);
    nscb->hevt.Set();
}

/***************************************************************************
    AT: The video stream playback thread.
***************************************************************************/
uint32_t GVDW::_LuThread(void)
{
    AssertThis(0);

    GstElement *vsink;
    GstElement *asink;
    GError *error = NULL;
    PGOB pgobScreen = GOB::PgobScreen();
    GNV gnv(pgobScreen);

    // GST_DEBUG=3,appsink:6
    _pipeline = gst_parse_launch(_desc, &error);

    vsink = gst_bin_get_by_name(GST_BIN(_pipeline), "vsink");
    g_object_set(G_OBJECT(vsink), "emit-signals", TRUE, NULL);
    g_signal_connect(vsink, "new-sample", G_CALLBACK(new_video_sample), &_nscb);
    asink = gst_bin_get_by_name(GST_BIN(_pipeline), "asink");
    g_object_set(G_OBJECT(asink), "emit-signals", TRUE, NULL);
    g_signal_connect(asink, "new-sample", G_CALLBACK(new_audio_sample), &_nscb);

    gst_element_set_state(_pipeline, GST_STATE_PLAYING);

    for (;;)
    {
        _nscb.hevt.Wait(0xffffffff);

        if (_fDone)
            break;

        if (_fPlaying)
        {
            if (_nscb.vsample != NULL)
            {
                GstBuffer *buffer;
                GstSample *sample;
                GstMapInfo map;

                buffer = gst_sample_get_buffer(_nscb.vsample);
                if (gst_buffer_map(buffer, &map, GST_MAP_READ))
                {
                    /* Update the surface with the mapped buffer */
                    CopyPb(map.data, _surface->pixels, _dxp * 4 * _dyp);

                    gnv.DrawSurface(_surface, &_rc);
                    gst_buffer_unmap(buffer, &map);
                }
                gst_sample_unref(_nscb.vsample);
                _nscb.vsample = NULL;
            }

            if (_nscb.asample != NULL)
            {
                GstBuffer *buffer;
                GstSample *sample;
                GstMapInfo map;

                buffer = gst_sample_get_buffer(_nscb.asample);
                if (gst_buffer_map(buffer, &map, GST_MAP_READ))
                {
                    _pastream->FWriteAudio(map.data, map.size / (sizeof(float) * 2));
                }
                gst_sample_unref(_nscb.asample);
                _nscb.asample = NULL;
            }

            if (gst_app_sink_is_eos(GST_APP_SINK_CAST(vsink)) &&
                gst_app_sink_is_eos(GST_APP_SINK_CAST(asink)))
            {
                Stop();
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(GVDW_LUTHREAD_SLEEP_DELAY));
        }
    }

    return 0;
}

int32_t GVDW::NfrMac(void)
{
    AssertThis(0);

    return _nfrMac;
}

int32_t GVDW::NfrCur(void)
{
    AssertThis(0);

    return 0;
}

void GVDW::GotoNfr(int32_t nfr)
{
    AssertThis(0);
    AssertIn(nfr, 0, _nfrMac);
}

bool GVDW::FPlaying(void)
{
    AssertThis(0);

    return _fPlaying;
}

bool GVDW::FPlay(RC *prc)
{
    AssertThis(0);
    AssertNilOrVarMem(prc);

    Stop();

    SetRcPlay(prc);
    _fPlaying = fTrue;

    return fTrue;
}

void GVDW::SetRcPlay(RC *prc)
{
    AssertThis(0);
    AssertNilOrVarMem(prc);

    if (pvNil == prc)
        _rcPlay.Set(0, 0, _dxp, _dyp);
    else
        _rcPlay = *prc;
}

void GVDW::Stop(void)
{
    AssertThis(0);

    if (!_fPlaying)
        return;

    gst_element_set_state(_pipeline, GST_STATE_PAUSED);
    _fPlaying = fFalse;
}

void GVDW::Draw(PGNV pgnv, RC *prc)
{
    AssertThis(0);
    AssertPo(pgnv, 0);
    AssertVarMem(prc);

    _SetRc();
}

void GVDW::_SetRc(void)
{
    AssertThis(0);
    RC rcGob, rc;

    _pgobBase->GetRc(&rcGob, cooHwnd);
    rc = _rcPlay;
    rc.Offset(rcGob.xpLeft, rcGob.ypTop);
    if (_rc != rc || !_fVisible)
    {
        _rc = rc;
    }

    if (_cactPal != vcactRealize)
    {
        _cactPal = vcactRealize;
    }
}

void GVDW::GetRc(RC *prc)
{
    AssertThis(0);
    AssertVarMem(prc);

    prc->Set(0, 0, _dxp, _dyp);
}

#ifdef DEBUG
void GVDW::AssertValid(uint32_t grf)
{
    GVDW_PAR::AssertValid(0);

    Assert(_surface != hNil, 0);
    AssertPo(_pgobBase, 0);
}

void GVDW::MarkMem(void)
{
    GVDW_PAR::MarkMem();

    MarkMemObj(_pastream);
}
#endif // DEBUG
