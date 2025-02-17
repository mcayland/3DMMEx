/*************************************************************************

    zbmp.h: Z-buffer Bitmap Class

    Primary Author: ******
    Review Status: REVIEWED - any changes to this file must be reviewed!

    BASE ---> BACO ---> ZBMP

*************************************************************************/
#ifndef ZBMP_H
#define ZBMP_H

#define kcbPixelZbmp 2 // Z-buffers are 2 bytes per pixel (16 bit)

// ZBMP on file
struct ZBMPF
{
    int16_t bo;
    int16_t osk;
    int16_t xpLeft;
    int16_t ypTop;
    int16_t dxp;
    int16_t dyp;
    // void *rgb; 		// pixels follow immediately after ZBMPF
};
VERIFY_STRUCT_SIZE(ZBMPF, 12);
const uint32_t kbomZbmpf = 0x55500000;

/****************************************
    ZBMP class
****************************************/
typedef class ZBMP *PZBMP;
#define ZBMP_PAR BACO
#define kclsZBMP 'ZBMP'
class ZBMP : public ZBMP_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    RC _rc;      // bounding rectangle of ZBMP
    int32_t _cbRow; // bytes per row
    int32_t _cb;    // count of bytes in Z buffer
    uint8_t *_prgb; // Z buffer
    ZBMP(void)
    {
    }

  public:
    static PZBMP PzbmpNew(int32_t dxp, int32_t dyp);
    static PZBMP PzbmpNewFromBpmp(BPMP *pbpmp);
    static PZBMP PzbmpRead(PBLCK pblck);
    static bool FReadZbmp(PCRF pcrf, CTG ctg, CNO cno, PBLCK pblck, PBACO *ppbaco, int32_t *pcb);
    ~ZBMP(void);

    uint8_t *Prgb(void)
    {
        return _prgb;
    }
    int32_t CbRow(void)
    {
        return _cbRow;
    }

    void Draw(uint8_t *prgbPixels, int32_t cbRow, int32_t dyp, int32_t xpRef, int32_t ypRef, RC *prcClip = pvNil,
              PREGN pregnClip = pvNil);
    void DrawHalf(uint8_t *prgbPixels, int32_t cbRow, int32_t dyp, int32_t xpRef, int32_t ypRef, RC *prcClip = pvNil,
                  PREGN pregnClip = pvNil);

    bool FWrite(PCFL pcfl, CTG ctg, CNO *pcno);
};

#endif // ZBMP_H
