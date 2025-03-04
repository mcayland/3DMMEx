/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************

    splot.h: Splot machine class

    Primary Author: ******
    Review Status: Reviewed

***************************************************************************/

#define SPLOT_PAR GOK
typedef class SPLOT *PSPLOT;
#define kclsSPLOT KLCONST4('s', 'p', 'l', 't')
class SPLOT : public SPLOT_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM
    CMD_MAP_DEC(SPLOT)

  private:
    /* The movie */
    PMVIE _pmvie;

    /* The lists of content */
    PBCL _pbclBkgd;
    SFL _sflBkgd;
    PBCL _pbclCam;
    SFL _sflCam;
    PBCL _pbclActr;
    SFL _sflActr;
    PBCL _pbclProp;
    SFL _sflProp;
    PBCL _pbclSound;
    SFL _sflSound;

    /* Current selected content */
    int32_t _ithdBkgd;
    int32_t _ithdCam;
    int32_t _ithdActr;
    int32_t _ithdProp;
    int32_t _ithdSound;

    /* State of the SPLOT */
    bool _fDirty;

    /* Miscellaneous stuff */
    PGL _pglclrSav;

    SPLOT(PGCB pgcb) : SPLOT_PAR(pgcb)
    {
        _fDirty = fFalse;
        _pbclBkgd = _pbclCam = _pbclActr = _pbclProp = _pbclSound = pvNil;
    }

  public:
    ~SPLOT(void);
    static PSPLOT PsplotNew(int32_t hidPar, int32_t hid, PRCA prca);

    bool FCmdInit(PCMD pcmd);
    bool FCmdSplot(PCMD pcmd);
    bool FCmdUpdate(PCMD pcmd);
    bool FCmdDismiss(PCMD pcmd);

    PMVIE Pmvie(void)
    {
        return _pmvie;
    }
};
