/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/******************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Error registration and reporting.  All error codes go here.

******************************************************************************/
#ifndef UTILERROR_H
#define UTILERROR_H

const int32_t kcerdMax = 20;

/***************************************************************************
    Error stack class
***************************************************************************/
#define ERS_PAR BASE
#define kclsERS KLCONST3('E', 'R', 'S')
class ERS : public ERS_PAR
{
    RTCLASS_DEC
    ASSERT

  private:
    struct ERD
    {
        int32_t erc;
#ifdef DEBUG
        PSZS pszsFile;
        int32_t lwLine;
#endif // DEBUG
    };

    MUTX _mutx;
    int32_t _cerd;
    ERD _rgerd[kcerdMax];

  public:
    ERS(void);

#ifdef DEBUG
    virtual void Push(int32_t erc, schar *pszsFile, int32_t lwLine);
#else  //! DEBUG
    virtual void Push(int32_t erc);
#endif //! DEBUG
    virtual bool FPop(int32_t *perc = pvNil);
    virtual bool FIn(int32_t erc);
    virtual int32_t Cerc(void);
    virtual int32_t ErcGet(int32_t ierc);
    virtual void Clear(void);
    virtual void Flush(int32_t erc);
};

extern ERS *vpers;

#ifdef DEBUG
#define PushErc(erc) vpers->Push(erc, __szsFile, __LINE__)
#else //! DEBUG
#define PushErc(erc) vpers->Push(erc)
#endif //! DEBUG

#endif //! UTILERROR_H
