/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Copyright (c) Microsoft Corporation

    Video playback.

***************************************************************************/
#ifndef VIDEO_H
#define VIDEO_H

/***************************************************************************
    Generic video class. This is an interface that supports the GVDS
    (video stream) and GVDW (video window) classes.
***************************************************************************/
typedef class GVID *PGVID;
#define GVID_PAR CMH
#define kclsGVID 'GVID'
class GVID : public GVID_PAR
{
    RTCLASS_DEC

  protected:
    GVID(int32_t hid);
    ~GVID(void)
    {
    }

  public:
    static PGVID PgvidNew(PFNI pfni, PGOB pgobBase, bool fHwndBased = fFalse, int32_t hid = hidNil);

    virtual int32_t NfrMac(void) = 0;
    virtual int32_t NfrCur(void) = 0;
    virtual void GotoNfr(int32_t nfr) = 0;

    virtual bool FPlaying(void) = 0;
    virtual bool FPlay(RC *prc = pvNil) = 0;
    virtual void Stop(void) = 0;

    virtual void Draw(PGNV pgnv, RC *prc) = 0;
    virtual void GetRc(RC *prc) = 0;
    virtual void SetRcPlay(RC *prc) = 0;
};

/****************************************
    Video stream class.
****************************************/
typedef class GVDS *PGVDS;
#define GVDS_PAR GVID
#define kclsGVDS 'GVDS'
class GVDS : public GVDS_PAR
{
    RTCLASS_DEC
    CMD_MAP_DEC(GVDS)
    ASSERT

  protected:
    int32_t _nfrMac;
    int32_t _nfrCur;
    int32_t _nfrMarked;
    int32_t _dxp;
    int32_t _dyp;

    PGOB _pgobBase;
    RC _rcPlay;
    uint32_t _tsPlay;

    bool _fPlaying;

#ifdef WIN
    int32_t _dnfr;
    PAVIFILE _pavif;
    PAVISTREAM _pavis;
    PGETFRAME _pavig;
    HDRAWDIB _hdd;
#endif // WIN

    GVDS(int32_t hid);
    ~GVDS(void);

    virtual bool _FInit(PFNI pfni, PGOB pgobBase);

  public:
    static PGVDS PgvdsNew(PFNI pfni, PGOB pgobBase, int32_t hid = hidNil);

    virtual int32_t NfrMac(void);
    virtual int32_t NfrCur(void);
    virtual void GotoNfr(int32_t nfr);

    virtual bool FPlaying(void);
    virtual bool FPlay(RC *prc = pvNil);
    virtual void Stop(void);

    virtual void Draw(PGNV pgnv, RC *prc);
    virtual void GetRc(RC *prc);
    virtual void SetRcPlay(RC *prc);

    virtual bool FCmdAll(PCMD pcmd);
};

/****************************************
    Video in a window class.
****************************************/
typedef class GVDW *PGVDW;
#define GVDW_PAR GVID
#define kclsGVDW 'GVDW'
class GVDW : public GVDW_PAR
{
    RTCLASS_DEC
    ASSERT

  protected:
    HWND _hwndMovie;
    int32_t _lwDevice;
    int32_t _dxp;
    int32_t _dyp;
    RC _rc;
    RC _rcPlay;
    int32_t _nfrMac;
    PGOB _pgobBase;
    int32_t _cactPal;

    bool _fDeviceOpen : 1;
    bool _fPlaying : 1;
    bool _fVisible : 1;

    GVDW(int32_t hid);
    ~GVDW(void);

    virtual bool _FInit(PFNI pfni, PGOB pgobBase);
    virtual void _SetRc(void);

  public:
    static PGVDW PgvdwNew(PFNI pfni, PGOB pgobBase, int32_t hid = hidNil);

    virtual int32_t NfrMac(void);
    virtual int32_t NfrCur(void);
    virtual void GotoNfr(int32_t nfr);

    virtual bool FPlaying(void);
    virtual bool FPlay(RC *prc = pvNil);
    virtual void Stop(void);

    virtual void Draw(PGNV pgnv, RC *prc);
    virtual void GetRc(RC *prc);
    virtual void SetRcPlay(RC *prc);
};

#endif //! VIDEO_H
