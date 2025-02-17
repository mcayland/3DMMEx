/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Copyright (c) Microsoft Corporation

    Compression/decompression manager and default Kauai codec.

***************************************************************************/
#ifndef CODEC_H
#define CODEC_H

/***************************************************************************
    Codec object.
***************************************************************************/
typedef class CODC *PCODC;
#define CODC_PAR BASE
#define kclsCODC 'CODC'
class CODC : public CODC_PAR
{
    RTCLASS_DEC

  public:
    // return whether this codec can handle the given format.
    virtual bool FCanDo(bool fEncode, int32_t cfmt) = 0;

    // Decompression should be extremely fast. Compression may be
    // (painfully) slow.
    virtual bool FConvert(bool fEncode, int32_t cfmt, void *pvSrc, int32_t cbSrc, void *pvDst, int32_t cbDst, int32_t *pcbDst) = 0;
};

/***************************************************************************
    Codec manager.
***************************************************************************/
typedef class CODM *PCODM;
#define CODM_PAR BASE
#define kclsCODM 'CODM'
class CODM : public CODM_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    int32_t _cfmtDef;
    PCODC _pcodcDef;
    PGL _pglpcodc;

    virtual bool _FFindCodec(bool fEncode, int32_t cfmt, PCODC *ppcodc);
    virtual bool _FCodePhq(int32_t cfmt, HQ *phq);
    virtual bool _FCode(int32_t cfmt, void *pvSrc, int32_t cbSrc, void *pvDst, int32_t cbDst, int32_t *pcbDst);

  public:
    CODM(PCODC pcodc, int32_t cfmt);
    ~CODM(void);

    int32_t CfmtDefault(void)
    {
        return _cfmtDef;
    }
    void SetCfmtDefault(int32_t cfmt);
    virtual bool FRegisterCodec(PCODC pcodc);
    virtual bool FCanDo(int32_t cfmt, bool fEncode);

    // Gets the type of compression used on the block (assuming it is
    // compressed).
    virtual bool FGetCfmtFromBlck(PBLCK pblck, int32_t *pcfmt);

    // FDecompress allows pvDst to be nil (in which case *pcbDst is filled
    // in with the buffer size required).
    // FCompress also allows pvDst to be nil, but the value returned in
    // *pcbDst will just be cbSrc - 1.
    bool FDecompressPhq(HQ *phq)
    {
        return _FCodePhq(cfmtNil, phq);
    }
    bool FCompressPhq(HQ *phq, int32_t cfmt = cfmtNil)
    {
        return _FCodePhq(cfmtNil == cfmt ? _cfmtDef : cfmt, phq);
    }

    bool FDecompress(void *pvSrc, int32_t cbSrc, void *pvDst, int32_t cbDst, int32_t *pcbDst)
    {
        return _FCode(cfmtNil, pvSrc, cbSrc, pvDst, cbDst, pcbDst);
    }
    bool FCompress(void *pvSrc, int32_t cbSrc, void *pvDst, int32_t cbDst, int32_t *pcbDst, int32_t cfmt = cfmtNil)
    {
        return _FCode(cfmtNil == cfmt ? _cfmtDef : cfmt, pvSrc, cbSrc, pvDst, cbDst, pcbDst);
    }
};

/***************************************************************************
    The standard Kauai Codec object.
***************************************************************************/
typedef class KCDC *PKCDC;
#define KCDC_PAR CODC
#define kclsKCDC 'KCDC'
class KCDC : public KCDC_PAR
{
    RTCLASS_DEC

  protected:
    bool _FEncode(void *pvSrc, int32_t cbSrc, void *pvDst, int32_t cbDst, int32_t *pcbDst);
    bool _FDecode(void *pvSrc, int32_t cbSrc, void *pvDst, int32_t cbDst, int32_t *pcbDst);
    bool _FEncode2(void *pvSrc, int32_t cbSrc, void *pvDst, int32_t cbDst, int32_t *pcbDst);
    bool _FDecode2(void *pvSrc, int32_t cbSrc, void *pvDst, int32_t cbDst, int32_t *pcbDst);

  public:
    virtual bool FCanDo(bool fEncode, int32_t cfmt)
    {
        return kcfmtKauai2 == cfmt || kcfmtKauai == cfmt;
    }
    virtual bool FConvert(bool fEncode, int32_t cfmt, void *pvSrc, int32_t cbSrc, void *pvDst, int32_t cbDst, int32_t *pcbDst);
};

#endif //! CODEC_H
