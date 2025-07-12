/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************

    srecstub.cpp: Sound recording stub class

    Primary Author: ****** (based on ***** original srec)
    Review Status: reviewed

***************************************************************************/
#include "soc.h"

ASSERTNAME

RTCLASS(SREC)

/***************************************************************************
    Create a new SREC
***************************************************************************/
PSREC SREC::PsrecNew(int32_t csampSec, int32_t cchan, int32_t cbSample, uint32_t dtsMax)
{
    PSREC psrec;

    psrec = NewObj(SREC);
    if (pvNil == psrec)
        return pvNil;
    if (!psrec->_FInit(csampSec, cchan, cbSample, dtsMax))
    {
        ReleasePpo(&psrec);
        return pvNil;
    }
    AssertPo(psrec, 0);
    return psrec;
}

/***************************************************************************
    Init this SREC
***************************************************************************/
bool SREC::_FInit(int32_t csampSec, int32_t cchan, int32_t cbSample, uint32_t dtsMax)
{
    AssertBaseThis(0);
    AssertIn(cchan, 0, ksuMax);

    int32_t cwid;

    _csampSec = csampSec;
    _cchan = cchan;
    _cbSample = cbSample;
    _dtsMax = dtsMax;
    _fBufferAdded = fFalse;
    _fRecording = fFalse;
    _fHaveSound = fFalse;

    vpsndm->Suspend(fTrue); // turn off sndm so we can get wavein device

    PushErc(ercSocNoWaveIn);
    return fFalse;
}

/***************************************************************************
    Clean up and delete this SREC
***************************************************************************/
SREC::~SREC(void)
{
    AssertBaseThis(0);

    // make sure nothing is playing or recording
    if (_fRecording || _fPlaying)
        FStop();

    vpsndm->Suspend(fFalse); // restore sound mgr
}

/***************************************************************************
    Figure out if we're recording or not
***************************************************************************/
void SREC::_UpdateStatus(void)
{
    AssertThis(0);

    _fRecording = fFalse;
    _fPlaying = fFalse;
}

/***************************************************************************
    Start recording
***************************************************************************/
bool SREC::FStart(void)
{
    AssertThis(0);
    Assert(!_fRecording, "stop previous recording first");

    return fFalse;
}

/***************************************************************************
    Stop recording or playing
***************************************************************************/
bool SREC::FStop(void)
{
    AssertThis(0);
    Assert(_fRecording || _fPlaying, "Nothing to stop");

    return fTrue;
}

/***************************************************************************
    Start playing the current sound
***************************************************************************/
bool SREC::FPlay(void)
{
    AssertThis(0);
    Assert(_fHaveSound, "No sound to play");

    return fFalse;
}

/***************************************************************************
    Are we recording?
***************************************************************************/
bool SREC::FRecording(void)
{
    AssertThis(0);

    _UpdateStatus();
    return _fRecording;
}

/***************************************************************************
    Are we playing the current sound?
***************************************************************************/
bool SREC::FPlaying(void)
{
    AssertThis(0);

    _UpdateStatus();
    return _fPlaying;
}

/***************************************************************************
    Save the current sound to the given FNI
***************************************************************************/
bool SREC::FSave(PFNI pfni)
{
    AssertThis(0);
    Assert(_fHaveSound, "Nothing to save!");

    return fFalse;
}

#ifdef DEBUG
/***************************************************************************
    Assert the validity of the SREC.
***************************************************************************/
void SREC::AssertValid(uint32_t grf)
{
    SREC_PAR::AssertValid(fobjAllocated);
}

/***************************************************************************
    Mark memory used by the SREC
***************************************************************************/
void SREC::MarkMem(void)
{
    AssertThis(0);
    SREC_PAR::MarkMem();
}
#endif // DEBUG
