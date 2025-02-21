/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/*****************************************************************************\
 *
 *	stdioscb.h
 *
 *	Author: ******
 *	Date: March, 1995
 *
 *	This file contains the studio scrollbar class SSCB.
 *
\*****************************************************************************/

#ifndef STDIOSCB_H
#define STDIOSCB_H

//
//	The studio scrollbar class.
//

const int32_t kctsFps = 20;

#define SSCB_PAR BASE
typedef class SSCB *PSSCB;
#define kclsSSCB 'SSCB'
class SSCB : public SSCB_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  private:
    int32_t _nfrmFirstOld;
    bool _fNoAutoadjust;

    bool _fBtnAddsFrames;

    //
    //	Private methods
    //
    int32_t _CxScrollbar(int32_t kidScrollbar, int32_t kidThumb);

  protected:
    PTGOB _ptgobFrame;
    PTGOB _ptgobScene;

#ifdef SHOW_FPS
    // Frame descriptor
    struct FDSC
    {
        uint32_t ts;
        int32_t cfrm;
    };

    PTGOB _ptgobFps;
    FDSC _rgfdsc[kctsFps];
    int32_t _itsNext;
#endif // SHOW_FPS

    PMVIE _pmvie;
    SSCB(PMVIE pmvie);

  public:
    //
    //	Constructors and destructors
    //
    static PSSCB PsscbNew(PMVIE pmvie);
    ~SSCB(void);

    //
    //	Notification
    //
    virtual void Update(void);
    void SetMvie(PMVIE pmvie);
    void StartNoAutoadjust(void);
    void EndNoAutoadjust(void)
    {
        AssertThis(0);
        _fNoAutoadjust = fFalse;
    }
    void SetSndFrame(bool fSoundInFrame);

    //
    //	Event handling
    //
    bool FCmdScroll(PCMD pcmd);
};

#endif // STDIOSCB_H
