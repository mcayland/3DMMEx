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
#ifndef MIDIDEV_H
#define MIDIDEV_H

/***************************************************************************
    The midi player.
***************************************************************************/
typedef class MIDP *PMIDP;
#define MIDP_PAR SNDMQ
#define kclsMIDP KLCONST4('M', 'I', 'D', 'P')
class MIDP : public MIDP_PAR
{
    RTCLASS_DEC

  protected:
    MIDP(void);

    virtual PSNQUE _PsnqueNew(void);
    virtual void _Suspend(bool fSuspend);

  public:
    static PMIDP PmidpNew(void);
    ~MIDP(void);

    // inherited methods
    virtual void SetVlm(int32_t vlm);
    virtual int32_t VlmCur(void);
};

#endif //! MIDIDEV_H
