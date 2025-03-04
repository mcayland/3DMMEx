/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Copyright (c) Microsoft Corporation

    The midi player sound device.

***************************************************************************/
#ifndef MIDIDEV2_H
#define MIDIDEV2_H

typedef class MSMIX *PMSMIX;

/***************************************************************************
    The midi player using a Midi stream.
***************************************************************************/
typedef class MDPS *PMDPS;
#define MDPS_PAR SNDMQ
#define kclsMDPS KLCONST4('M', 'D', 'P', 'S')
class MDPS : public MDPS_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    PMSMIX _pmsmix;

    MDPS(void);

    virtual bool _FInit(void);
    virtual PSNQUE _PsnqueNew(void);
    virtual void _Suspend(bool fSuspend);

  public:
    static PMDPS PmdpsNew(void);
    ~MDPS(void);

    // inherited methods
    virtual void SetVlm(int32_t vlm);
    virtual int32_t VlmCur(void);
};

#endif //! MIDIDEV2_H
