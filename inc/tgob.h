/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************

    Lite, low-cholestoral, politically correct, ethinically and genderally
    mixed text gobs.

        TGOB 	--->   	GOB

***************************************************************************/

#ifndef TGOB_H
#define TGOB_H

#include "frame.h"

//
// Tgob class
//
#define TGOB_PAR GOB
#define kclsTGOB 'tgob'
typedef class TGOB *PTGOB;
class TGOB : public TGOB_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    int32_t _onn;
    int32_t _dypFont;
    STN _stn;
    int32_t _tah;
    int32_t _tav;
    ACR _acrFore;
    ACR _acrBack;
    ~TGOB(void)
    {
    }

  public:
    //
    // Create and destroy functions
    //
    TGOB(PGCB pgcb);
    TGOB(int32_t hid);

    void SetFont(int32_t onn)
    {
        AssertThis(0);
        _onn = onn;
    }
    void SetFontSize(int32_t dypFont)
    {
        AssertThis(0);
        _dypFont = dypFont;
    }
    void SetText(PSTN pstn)
    {
        AssertThis(0);
        _stn = *pstn;
        InvalRc(pvNil, kginMark);
    }
    void SetAcrFore(ACR acrFore)
    {
        AssertThis(0);
        _acrFore = acrFore;
    }
    void SetAcrBack(ACR acrBack)
    {
        AssertThis(0);
        _acrBack = acrBack;
    }
    void SetAlign(int32_t tah = tahLim, int32_t tav = tavLim);
    int32_t GetFont(void)
    {
        AssertThis(0);
        return (_onn);
    }
    int32_t GetFontSize(void)
    {
        AssertThis(0);
        return _dypFont;
    }
    ACR GetAcrFore(void)
    {
        AssertThis(0);
        return (_acrFore);
    }
    ACR GetAcrBack(void)
    {
        AssertThis(0);
        return (_acrBack);
    }
    void GetAlign(int32_t *ptah = pvNil, int32_t *ptav = pvNil);
    static PTGOB PtgobCreate(int32_t kidFrm, int32_t idsFont, int32_t tav = tavTop, int32_t hid = hidNil);

    virtual void Draw(PGNV pgnv, RC *prcClip) override;
};

#endif
