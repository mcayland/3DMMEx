/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Random number generator and shuffler stuff.

***************************************************************************/
#ifndef UTILRND_H
#define UTILRND_H

/***************************************************************************
    A pseudo-random number generator. LwNext returns values from 0 to
    (lwLim - 1), inclusive.
***************************************************************************/
typedef class RND *PRND;
#define RND_PAR BASE
#define kclsRND KLCONST3('R', 'N', 'D')
class RND : public RND_PAR
{
    RTCLASS_DEC
    NOCOPY(RND)

  protected:
    uint32_t _luSeed;

  public:
    RND(uint32_t luSeed = 0L);
    virtual int32_t LwNext(int32_t lwLim);
};

/***************************************************************************
    A shuffled array of numbers.
***************************************************************************/
typedef class SFL *PSFL;
#define SFL_PAR RND
#define kclsSFL KLCONST3('S', 'F', 'L')
class SFL : public SFL_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM
    NOCOPY(SFL)

  protected:
    int32_t _clw;
    int32_t _ilw;
    HQ _hqrglw;
    bool _fCustom; // false iff the values in the hq are [0, _clw)

    bool _FEnsureHq(int32_t clw);
    void _ShuffleCore(void);

  public:
    SFL(uint32_t luSeed = 0L);
    ~SFL(void);
    void Shuffle(int32_t lwLim);
    void ShuffleRglw(int32_t clw, int32_t *prglw);

    virtual int32_t LwNext(int32_t lwLim = 0) override;
};

#endif // UTILRND_H
