/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/*************************************************************************

    body.h: Body class

    Primary Author: ******
    Review Status: REVIEWED - any changes to this file must be reviewed!

    BASE ---> BODY

*************************************************************************/
#ifndef BODY_H
#define BODY_H

/****************************************
    The BODY class
****************************************/
typedef class BODY *PBODY;
#define BODY_PAR BASE
#define kclsBODY KLCONST4('B', 'O', 'D', 'Y')
class BODY : public BODY_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    static BMTL *_pbmtlHilite; // hilight material
    static BODY *_pbodyClosestClicked;
    static int32_t _dzpClosestClicked;
    static BACT *_pbactClosestClicked;
    BACT *_prgbact;      // array of BACTs
    PGL _pglibset;       // body part set IDs
    int32_t _cbset;      // count of body part sets
    PCMTL *_prgpcmtl;    // array of PCMTLs -- one per body part set
    int32_t _cbactPart;  // count of model body parts in body
    int32_t _cactHidden; // for Show() / Hide()
    PBWLD _pbwld;        // world that body lives in
    RC _rcBounds;        // bounds of body after last render
    RC _rcBoundsLastVis; // bounds of body last time it was visible
    bool _fFound;        // is the actor found under the mouse?
    int32_t _ibset;      // which body part got hit.

  protected:
    BODY(void)
    {
    } // can't instantiate directly; must use PbodyNew
    void _DestroyShape(void);
    bool _FInit(PGL pglibactPar, PGL pglibset);
    bool _FInitShape(PGL pglibactPar, PGL pglibset);
    PBACT _PbactRoot(void) // ptr to root BACT
    {
        return _prgbact;
    }
    PBACT _PbactHilite(void) // ptr to hilite BACT
    {
        return _prgbact + 1;
    }                               // skip root BACT
    PBACT _PbactPart(int32_t ipart) // ptr to ipart'th body part
    {
        return _prgbact + 1 + 1 + ipart;
    }                    // skip root and hilite BACTs
    int32_t _Cbact(void) // count in _prgbact
    {
        return 1 + 1 + _cbactPart;
    }                             // root, hilite, and body part BACTs
    int32_t _Ibset(int32_t ipart) // body part set that this part belongs to
    {
        return *(int16_t *)_pglibset->QvGet(ipart);
    }
    void _RemoveMaterial(int32_t ibset);

    // Callbacks from BRender:
    static int BR_CALLBACK _FFilterSearch(BACT *pbact, PBMDL pbmdl, PBMTL pbmtl, BVEC3 *pbvec3RayPos,
                                          BVEC3 *pbvec3RayDir, BRS dzpNear, BRS dzpFar, void *pvArg);
    static void _BactRendered(PBACT pbact, RC *prc);
    static void _PrepareToRender(PBACT pbact);
    static void _GetRc(PBACT pbact, RC *prc);

  public:
    static PBODY PbodyNew(PGL pglibactPar, PGL pglibset);
    static PBODY PbodyFromBact(BACT *pbact, int32_t *pibset = pvNil);
    static PBODY PbodyClicked(int32_t xp, int32_t yp, PBWLD pbwld, int32_t *pibset = pvNil);
    ~BODY(void);
    PBODY PbodyDup(void);
    void Restore(PBODY pbodyDup);
    static int BR_CALLBACK _FFilter(BACT *pbact, PBMDL pbmdl, PBMTL pbmtl, BVEC3 *pbvec3RayPos, BVEC3 *pbvec3RayDir,
                                    BRS dzpNear, BRS dzpFar, void *pv);

    bool FChangeShape(PGL pglibactPar, PGL pglibset);
    void SetBwld(PBWLD pbwld)
    {
        _pbwld = pbwld;
    }

    void Show(void);
    void Hide(void);
    void Hilite(void);
    void Unhilite(void);

    static void SetHiliteColor(int32_t iclr);

    void LocateOrient(BRS xr, BRS yr, BRS zr, BMAT34 *pbmat34);
    void SetPartModel(int32_t ibact, MODL *pmodl);
    void SetPartMatrix(int32_t ibact, BMAT34 *pbmat34);
    void SetPartSetMtrl(int32_t ibset, MTRL *pmtrl);
    void SetPartSetCmtl(CMTL *pcmtl);
    void GetPartSetMaterial(int32_t ibset, bool *pfMtrl, MTRL **ppmtrl, CMTL **ppcmtl);
    int32_t Cbset()
    {
        return _cbset;
    }

    void GetBcbBounds(BCB *pbcb, bool fWorld = fFalse);
    void GetRcBounds(RC *prc);
    void GetCenter(int32_t *pxp, int32_t *pyp);
    void GetPosition(BRS *pxr, BRS *pyr, BRS *pzr);
    bool FPtInBody(int32_t xp, int32_t yp, int32_t *pibset);
    bool FIsInView(void);
};

/****************************************
    The COST class, which is used to
    save and restore a BODY's entire
    costume for unwinding purposes
****************************************/
typedef class COST *PCOST;
#define COST_PAR BASE
#define kclsCOST KLCONST4('C', 'O', 'S', 'T')
class COST : public COST_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  private:
    int32_t _cbset; // count of body part sets in _prgpo
    BASE **_prgpo;  // array of MTRLs and CMTLs

  private:
    void _Clear(void); // release _prgpo and material references

  public:
    COST(void);
    ~COST(void);

    bool FGet(PBODY pbody); // read and store BODY's costume

    // replace BODY's costume with this one
    void Set(PBODY pbody, bool fAllowDifferentShape = fFalse);
};

#endif // !BODY_H
