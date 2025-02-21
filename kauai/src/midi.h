/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Copyright (c) Microsoft Corporation

    Midi playback class.

***************************************************************************/
#ifndef MIDI_H
#define MIDI_H

typedef class MIDS *PMIDS;

// midi event
struct MIDEV
{
    uint32_t ts;     // time stamp of this event
    int32_t cb;      // number of bytes to send (in rgbSend)
    int32_t lwTempo; // the current tempo - at a tempo change, cb will be 0
    union {
        uint8_t rgbSend[4]; // bytes to send if pvLong is nil
        int32_t lwSend;     // for convenience
    };
};
typedef MIDEV *PMIDEV;

/***************************************************************************
    Midi stream parser. Knows how to parse standard MIDI streams.
***************************************************************************/
typedef class MSTP *PMSTP;
#define MSTP_PAR BASE
#define kclsMSTP 'MSTP'
class MSTP : public MSTP_PAR
{
    RTCLASS_DEC
    NOCOPY(MSTP)
    ASSERT
    MARKMEM

  protected:
    uint32_t _tsCur;
    int32_t _lwTempo;
    uint8_t *_prgb;
    uint8_t *_pbLim;
    uint8_t *_pbCur;
    uint8_t _bStatus;

    Debug(int32_t _cactLongLock;) PMIDS _pmids;

    bool _FReadVar(uint8_t **ppbCur, int32_t *plw);

  public:
    MSTP(void);
    ~MSTP(void);

    void Init(PMIDS pmids, uint32_t tsStart = 0, int32_t lwTempo = 500000);
    bool FGetEvent(PMIDEV pmidev, bool fAdvance = fTrue);
};

/***************************************************************************
    Midi Stream object - this is like a MTrk chunk in a standard MIDI file,
    with timing in milliseconds.
***************************************************************************/
#define MIDS_PAR BACO
#define kclsMIDS 'MIDS'
class MIDS : public MIDS_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    HQ _hqrgb;

    friend MSTP;

    MIDS(void);

    static int32_t _CbEncodeLu(uint32_t lu, uint8_t *prgb);

  public:
    static bool FReadMids(PCRF pcrf, CTG ctg, CNO cno, PBLCK pblck, PBACO *ppbaco, int32_t *pcb);
    static PMIDS PmidsRead(PBLCK pblck);
    static PMIDS PmidsReadNative(FNI *pfni);
    ~MIDS(void);

    virtual bool FWrite(PBLCK pblck);
    virtual int32_t CbOnFile(void);
};

#endif //! MIDI_H
