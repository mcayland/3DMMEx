/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************

    bkgd.h: Background class

    Primary Author: ******
    Review Status: REVIEWED - any changes to this file must be reviewed!

    BASE ---> BACO ---> BKGD

***************************************************************************/
#ifndef BKGD_H
#define BKGD_H

/****************************************
    Background on file
****************************************/
struct BKGDF
{
    int16_t bo;
    int16_t osk;
    uint8_t bIndexBase;
    uint8_t bPad;
    int16_t swPad;
};
VERIFY_STRUCT_SIZE(BKGDF, 8);
const BOM kbomBkgdf = 0x50000000;

/****************************************
    Specifies a light's kind, position,
    orientation, and brightness
****************************************/
struct LITE
{
    BMAT34 bmat34;
    BRS rIntensity;
    int32_t lt; // light type
};
VERIFY_STRUCT_SIZE(LITE, 56);
const BOM kbomLite = 0xfffffff0;

/****************************************
    Specifies a camera for a view
****************************************/
typedef union _apos {
    struct
    {
        BRS xrPlace; // Initial Actor Placement point
        BRS yrPlace;
        BRS zrPlace;
    };
    BVEC3 bvec3Actor;
} APOS;
VERIFY_STRUCT_SIZE(APOS, 12);

struct CAM
{
    int16_t bo;
    int16_t osk;
    BRS zrHither; // Hither (near) plane
    BRS zrYon;    // Yon (far) plane
    BRA aFov;     // Field of view
    int16_t swPad;
    APOS apos;
    BMAT34 bmat34Cam; // Camera view matrix
    // APOS rgapos[];
};
VERIFY_STRUCT_SIZE(CAM, 76);
const BOM kbomCamOld = 0x5f4fc000;
const BOM kbomCam = BomField(
    kbomSwapShort,
    BomField(kbomSwapShort,
             BomField(kbomSwapLong,
                      BomField(kbomSwapLong,
                               BomField(kbomSwapShort,
                                        BomField(kbomLeaveShort,
                                                 BomField(kbomSwapLong,
                                                          BomField(kbomSwapLong, BomField(kbomSwapLong, 0)))))))));

// Note that CAM is too big for a complete kbomCam.  To SwapBytes one,
// SwapBytesBom the cam, then SwapBytesRgLw from bmat34Cam on.

/****************************************
    Background Default Sound
****************************************/
struct BDS
{
    int16_t bo;
    int16_t osk;
    int32_t vlm;
    bool fLoop;
    TAG tagSnd;
};
VERIFY_STRUCT_SIZE(BDS, 28);
const BOM kbomBds = 0x5f000000 | kbomTag >> 8;

/****************************************
    The background class
****************************************/
typedef class BKGD *PBKGD;
#define BKGD_PAR BACO
#define kclsBKGD KLCONST4('B', 'K', 'G', 'D')
class BKGD : public BKGD_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    BACT *_prgbactLight; // array of light br_actors
    BLIT *_prgblitLight; // array of light data
    int32_t _cbactLight; // count of lights
    bool _fLites;        // lights are on
    bool _fLeaveLitesOn; // Don't turn out the lights
    int32_t _ccam;       // count of cameras in this background
    int32_t _icam;       // current camera
    BMAT34 _bmat34Mouse; // camera matrix for mouse model
    BRA _braRotY;        // Y rotation of current camera
    CNO _cnoSnd;         // background sound
    STN _stn;            // name of this background
    PGL _pglclr;         // palette for this background
    uint8_t _bIndexBase; // first index for palette
    int32_t _iaposLast;  // Last placement point we used
    int32_t _iaposNext;  // Next placement point to use
    PGL _pglapos;        // actor placement point(s) for current view
    BRS _xrPlace;
    BRS _yrPlace;
    BRS _zrPlace;
    BDS _bds;   // background default sound
    BRS _xrCam; // camera position in worldspace
    BRS _yrCam;
    BRS _zrCam;

  protected:
    bool _FInit(PCFL pcfl, CTG ctg, CNO cno);
    int32_t _Ccam(PCFL pcfl, CTG ctg, CNO cno);
    void _SetupLights(PGL pgllite);

  public:
    static bool FAddTagsToTagl(PTAG ptagBkgd, PTAGL ptagl);
    static bool FCacheToHD(PTAG ptagBkgd);
    static bool FReadBkgd(PCRF pcrf, CTG ctg, CNO cno, PBLCK pblck, PBACO *ppbaco, int32_t *pcb);
    ~BKGD(void);
    void GetName(PSTN pstn);

    void TurnOnLights(PBWLD pbwld);
    void TurnOffLights(void);
    bool FLeaveLitesOn(void)
    {
        return _fLeaveLitesOn;
    }
    void SetFLeaveLitesOn(bool fLeaveLitesOn)
    {
        _fLeaveLitesOn = fLeaveLitesOn;
    }

    int32_t Ccam(void)
    {
        return _ccam;
    } // count of cameras in background
    int32_t Icam(void)
    {
        return _icam;
    }                                           // currently selected camera
    bool FSetCamera(PBWLD pbwld, int32_t icam); // change camera to icam

    void GetMouseMatrix(BMAT34 *pbmat34);
    BRA BraRotYCamera(void)
    {
        return _braRotY;
    }
    void GetActorPlacePoint(BRS *pxr, BRS *pyr, BRS *pzr);
    void ReuseActorPlacePoint(void);

    void GetDefaultSound(PTAG ptagSnd, int32_t *pvlm, bool *pfLoop)
    {
        *ptagSnd = _bds.tagSnd;
        *pvlm = _bds.vlm;
        *pfLoop = _bds.fLoop;
    }

    bool FGetPalette(PGL *ppglclr, int32_t *piclrMin);
    void GetCameraPos(BRS *pxr, BRS *pyr, BRS *pzr);

#ifdef DEBUG
    // Authoring only.  Writes a special file with the given place info.
    bool FWritePlaceFile(BRS xrPlace, BRS yrPlace, BRS zrPlace);
#endif // DEBUG
};

#endif // BKGD_H
