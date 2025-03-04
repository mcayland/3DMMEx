/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/*************************************************************************

    modl.h: Model class

    Primary Author: ******
    Review Status: REVIEWED - any changes to this file must be reviewed!

    BASE ---> BACO ---> MODL

*************************************************************************/
#ifndef MODL_H
#define MODL_H

// Model on file:
struct MODLF
{
    int16_t bo;
    int16_t osk;
    int16_t cver; // count of vertices
    int16_t cfac; // count of faces
    BRS rRadius;
    BRB brb; // bounds
    BVEC3 bvec3Pivot;
    //	br_vertex rgbrv[]; // vertices
    //	br_face rgbrf[]; // faces
};
VERIFY_STRUCT_SIZE(MODLF, 48);
typedef MODLF *PMODLF;
const BOM kbomModlf = 0x55fffff0;

/****************************************
    MODL: a wrapper for BRender models
****************************************/
typedef class MODL *PMODL;
#define MODL_PAR BACO
#define kclsMODL KLCONST4('M', 'O', 'D', 'L')
class MODL : public MODL_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    BMDL *_pbmdl; // BRender model data
  protected:
    MODL(void)
    {
    }
    bool _FInit(PBLCK pblck);
    bool _FPrelight(int32_t cblit, BVEC3 *prgbvec3Light);

  public:
    static PMODL PmodlNew(int32_t cbrv, BRV *prgbrv, int32_t cbrf, BRF *prgbrf);
    static bool FReadModl(PCRF pcrf, CTG ctg, CNO cno, PBLCK pblck, PBACO *ppbaco, int32_t *pcb);
    static PMODL PmodlReadFromDat(FNI *pfni);
    static PMODL PmodlFromBmdl(PBMDL pbmdl);
    ~MODL(void);
    PBMDL Pbmdl(void)
    {
        return _pbmdl;
    }
    void AdjustTdfCharacter(void);
    bool FWrite(PCFL pcfl, CTG ctg, CNO cno);

    BRS Dxr(void)
    {
        return _pbmdl->bounds.max.v[0] - _pbmdl->bounds.min.v[0];
    }
    BRS Dyr(void)
    {
        return _pbmdl->bounds.max.v[1] - _pbmdl->bounds.min.v[1];
    }
    BRS Dzr(void)
    {
        return _pbmdl->bounds.max.v[2] - _pbmdl->bounds.min.v[2];
    }
};

#endif // TMPL_H
