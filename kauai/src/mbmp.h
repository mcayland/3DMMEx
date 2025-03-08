/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Masked Bitmap management code declarations.

***************************************************************************/
#ifndef MBMP_H
#define MBMP_H

const FTG kftgBmp = 'BMP';

enum
{
    fmbmpNil = 0,
    fmbmpUpsideDown = 1,
    fmbmpMask = 2,
};

typedef class MBMP *PMBMP;
#define MBMP_PAR BACO
#define kclsMBMP KLCONST4('M', 'B', 'M', 'P')
class MBMP : public MBMP_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    // rc (in the MBMPH) is the bounding rectangle of the mbmp. It implicitly
    // holds the reference point.

    // _hqrgb holds an MBMPH followed by an array of the length of each row
    // (rgcb) followed by the actual pixel data. The rgcb is an array of shorts
    // of length rc.Dyp(). We store the whole MBMPH in the _hqrgb so that
    // loading the MBMP from a chunky file is fast. If the chunk is compressed,
    // storing anything less than the full chunk in _hqrgb requires another blt.

    // The pixel data is stored row by row with transparency encoded using
    // an RLE scheme. For each row, the first byte is the number of consecutive
    // tranparent pixels. The next byte is the number of consecutive
    // non-transparent pixels (cb). The next cb bytes are the values of
    // the non-transparent pixels. This order repeats itself for the rest of
    // the row, and then the next row begins. Rows should never end with a
    // transparent byte.

    // If fMask is true, the non-transparent pixels are not in _hqrgb. Instead,
    // all non-transparent pixels have the value bFill.
    int32_t _cbRgcb; // size of the rgcb portion of _hqrgb
    HQ _hqrgb;       // MBMPH, short rgcb[_rc.Dyp()] followed by the pixel data

    // MBMP header on file
    struct MBMPH
    {
        int16_t bo;
        int16_t osk;
        uint8_t fMask;
        uint8_t bFill;      // if fMask, the color value to use
        int16_t swReserved; // should be zero on file
        RC rc;
        int32_t cb; // length of whole chunk, including the header
    };
    VERIFY_STRUCT_SIZE(MBMPH, 28);

    MBMP(void)
    {
    }
    virtual bool _FInit(uint8_t *prgbPixels, int32_t cbRow, int32_t dyp, RC *prc, int32_t xpRef, int32_t ypRef,
                        uint8_t bTransparent, uint32_t grfmbmp = fmbmpNil, uint8_t bDefault = 0);

    int16_t *_Qrgcb(void)
    {
        return (int16_t *)PvAddBv(QvFromHq(_hqrgb), SIZEOF(MBMPH));
    }
    MBMPH *_Qmbmph(void)
    {
        return (MBMPH *)QvFromHq(_hqrgb);
    }

  public:
    ~MBMP(void);

    static PMBMP PmbmpNew(uint8_t *prgbPixels, int32_t cbRow, int32_t dyp, RC *prc, int32_t xpRef, int32_t ypRef,
                          uint8_t bTransparent, uint32_t grfmbmp = fmbmpNil, uint8_t bDefault = 0);
    static PMBMP PmbmpReadNative(FNI *pfni, uint8_t bTransparent = 0, int32_t xp = 0, int32_t yp = 0,
                                 uint32_t grfmbmp = fmbmpNil, uint8_t bDefault = 0);

    static PMBMP PmbmpRead(PBLCK pblck);

    void GetRc(RC *prc);
    void Draw(uint8_t *prgbPixels, int32_t cbRow, int32_t dyp, int32_t xpRef, int32_t ypRef, RC *prcClip = pvNil,
              PREGN pregnClip = pvNil);
    void DrawMask(uint8_t *prgbPixels, int32_t cbRow, int32_t dyp, int32_t xpRef, int32_t ypRef, RC *prcClip = pvNil);
    bool FPtIn(int32_t xp, int32_t yp);

    virtual bool FWrite(PBLCK pblck) override;
    virtual int32_t CbOnFile(void) override;

    // a chunky resource reader for an MBMP
    static bool FReadMbmp(PCRF pcrf, CTG ctg, CNO cno, PBLCK pblck, PBACO *ppbaco, int32_t *pcb);
};
const BOM kbomMbmph = 0xAFFC0000;

// reads a bitmap from the given file
bool FReadBitmap(FNI *pfni, uint8_t **pprgb, PGL *ppglclr, int32_t *pdxp, int32_t *pdyp, bool *pfUpsideDown,
                 uint8_t bTransparent = 0);

// writes a bitmap file
bool FWriteBitmap(FNI *pfni, uint8_t *prgb, PGL pglclr, int32_t dxp, int32_t dyp, bool fUpsideDown = fTrue);

#endif //! MBMP_H
