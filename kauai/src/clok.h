/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Clock class. See comments in clok.cpp.

***************************************************************************/
#ifndef CLOK_H
#define CLOK_H

enum
{
    fclokNil = 0,
    fclokReset = 1,
    fclokNoSlip = 2,
};

typedef class CLOK *PCLOK;
#define CLOK_PAR CMH
#define kclsCLOK 'CLOK'
class CLOK : public CLOK_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM
    CMD_MAP_DEC(CLOK)

  protected:
    // alarm descriptor
    struct ALAD
    {
        PCMH pcmh;
        uint32_t tim;
        int32_t lw;
    };

    static PCLOK _pclokFirst;

    PCLOK _pclokNext;
    uint32_t _tsBase;
    uint32_t _timBase;
    uint32_t _timCur;    // current time
    uint32_t _dtimAlarm; // processing alarms up to _timCur + _dtimAlarm
    uint32_t _timNext;   // next alarm time to process (for speed)
    uint32_t _grfclok;
    PGL _pglalad; // the registered alarms

  public:
    CLOK(int32_t hid, uint32_t grfclok = fclokNil);
    ~CLOK(void);
    static PCLOK PclokFromHid(int32_t hid);
    static void BuryCmh(PCMH pcmh);
    void RemoveCmh(PCMH pcmh);

    void Start(uint32_t tim);
    void Stop(void);
    uint32_t TimCur(bool fAdjustForDelay = fFalse);
    uint32_t DtimAlarm(void)
    {
        return _dtimAlarm;
    }

    bool FSetAlarm(int32_t dtim, PCMH pcmhNotify = pvNil, int32_t lwUser = 0, bool fAdjustForDelay = fFalse);

    // idle handling
    virtual bool FCmdAll(PCMD pcmd);

#ifdef DEBUG
    static void MarkAllCloks(void);
#endif // DEBUG
};

#endif //! CLOK_H
