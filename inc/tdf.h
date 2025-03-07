/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************

    tdf.h: Three-D Font class

    Primary Author: ******
    Review Status: REVIEWED - any changes to this file must be reviewed!

    BASE ---> BACO ---> TDF  (Three-D Font)

***************************************************************************/
#ifndef TDF_H
#define TDF_H

/****************************************
    3-D Font class
****************************************/
typedef class TDF *PTDF;
#define TDF_PAR BACO
#define kclsTDF KLCONST3('T', 'D', 'F')
class TDF : public TDF_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    int32_t _cch; // count of chars
    BRS _dyrMax;  // max character height
    BRS *_prgdxr; // character widths
    BRS *_prgdyr; // character heights

  protected:
    TDF(void)
    {
    }
    bool _FInit(PBLCK pblck);

  public:
    static bool FReadTdf(PCRF pcrf, CTG ctg, CNO cno, PBLCK pblck, PBACO *ppbaco, int32_t *pcb);
    ~TDF(void);

    // This authoring-only API creates a new TDF based on a set of models
    static bool FCreate(PCRF pcrf, PGL pglkid, STN *pstn, CKI *pckiTdf = pvNil);

    PMODL PmodlFetch(CHID chid);
    BRS DxrChar(int32_t ich);
    BRS DyrChar(int32_t ich);
    BRS DyrMax(void)
    {
        return _dyrMax;
    }
};

#endif // TDF_H
