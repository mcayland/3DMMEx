/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Picture management code declarations.

***************************************************************************/
#ifndef PIC_H
#define PIC_H

const FTG kftgPict = 'PICT';
const FTG kftgMeta = 'WMF';
const FTG kftgEnhMeta = 'EMF';

/***************************************************************************
    Picture class.  This is a wrapper around a system picture (Mac Pict or
    Win MetaFile).
***************************************************************************/
typedef class PIC *PPIC;
#define PIC_PAR BACO
#define kclsPIC KLCONST3('P', 'I', 'C')
class PIC : public PIC_PAR
{
    RTCLASS_DEC
    ASSERT

  protected:
    struct PICH
    {
        RC rc;
        int32_t cb;
    };

    HPIC _hpic;
    RC _rc;

    PIC(void);
#ifdef WIN
    static HPIC _HpicReadWmf(FNI *pfni);
#endif // WIN

  public:
    ~PIC(void);

    static PPIC PpicFetch(PCFL pcfl, CTG ctg, CNO cno, CHID chid = 0);
    static PPIC PpicRead(PBLCK pblck);
    static PPIC PpicReadNative(FNI *pfni);
    static PPIC PpicNew(HPIC hpic, RC *prc);

    void GetRc(RC *prc);
    HPIC Hpic(void)
    {
        return _hpic;
    }
    bool FAddToCfl(PCFL pcfl, CTG ctg, CNO *pcno, CHID chid = 0);
    bool FPutInCfl(PCFL pcfl, CTG ctg, CNO cno, CHID chid = 0);
    virtual int32_t CbOnFile(void);
    virtual bool FWrite(PBLCK pblck);
};

// a chunky resource reader to read picture 0 from a GRAF chunk
bool FReadMainPic(PCFL pcfl, CTG ctg, CNO cno, PBLCK pblck, PBACO *ppbaco, int32_t *pcb);

#endif //! PIC_H
