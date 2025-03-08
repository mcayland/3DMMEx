/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Base classes.  All classes should be derived from BASE.
    BLL is a base class for singly linked lists.

***************************************************************************/
#ifndef BASE_H
#define BASE_H

/* Character-based constants */
#define KLCONST2(a, b) ((a << 8) | b)

#define KLCONST3(a, b, c) ((a << 16) | (b << 8) | c)

#define KLCONST4(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)

/***************************************************************************
    Run-time class determination support.  Each class, FOO, that uses this
    needs a constant, kclsFOO, defined somewhere (preferably with the class
    declaration) and needs FOO_PAR defined to be the class' parent class.
    kclsFOO should be 'FOO' if FOO is at most 4 characters long and should
    consist of a unique string of 4 lowercase characters if FOO is longer
    than 4 characters. Eg, kclsSCEG is 'SCEG', but kclsSTUDIO is 'stdo'.

    RTCLASS_DEC goes in the class definition.
    RTCLASS(FOO) goes in the .cpp file.
***************************************************************************/
#define RTCLASS_DEC                                                                                                    \
  public:                                                                                                              \
    static bool FWouldBe(int32_t cls);                                                                                 \
                                                                                                                       \
  public:                                                                                                              \
    virtual bool FIs(int32_t cls) override;                                                                            \
                                                                                                                       \
  public:                                                                                                              \
    virtual int32_t Cls(void) override;

// RTCLASS_DEC for the top-level BASE class
#define RTCLASS_DEC_BASE                                                                                               \
  public:                                                                                                              \
    static bool FWouldBe(int32_t cls);                                                                                 \
                                                                                                                       \
  public:                                                                                                              \
    virtual bool FIs(int32_t cls);                                                                                     \
                                                                                                                       \
  public:                                                                                                              \
    virtual int32_t Cls(void);

#define RTCLASS_INLINE(foo)                                                                                            \
  public:                                                                                                              \
    static bool FWouldBe(int32_t cls)                                                                                  \
    {                                                                                                                  \
        if (kcls##foo == cls)                                                                                          \
            return fTrue;                                                                                              \
        return foo##_PAR::FWouldBe(cls);                                                                               \
    }                                                                                                                  \
                                                                                                                       \
  public:                                                                                                              \
    virtual bool FIs(int32_t cls) override                                                                             \
    {                                                                                                                  \
        return FWouldBe(cls);                                                                                          \
    }                                                                                                                  \
                                                                                                                       \
  public:                                                                                                              \
    virtual int32_t Cls(void) override                                                                                 \
    {                                                                                                                  \
        return kcls##foo;                                                                                              \
    }

#define RTCLASS(foo)                                                                                                   \
    bool foo::FWouldBe(int32_t cls)                                                                                    \
    {                                                                                                                  \
        if (kcls##foo == cls)                                                                                          \
            return fTrue;                                                                                              \
        return foo##_PAR::FWouldBe(cls);                                                                               \
    }                                                                                                                  \
    bool foo::FIs(int32_t cls)                                                                                         \
    {                                                                                                                  \
        return FWouldBe(cls);                                                                                          \
    }                                                                                                                  \
    int32_t foo::Cls(void)                                                                                             \
    {                                                                                                                  \
        return kcls##foo;                                                                                              \
    }

/***************************************************************************
    Debugging aids - for finding lost memory and asserting the validity
    of objects.
***************************************************************************/
#ifdef DEBUG

void AssertUnmarkedObjs(void);
#define MarkMemObj(po)                                                                                                 \
    if ((po) != pvNil)                                                                                                 \
    {                                                                                                                  \
        (po)->MarkMemStub();                                                                                           \
    }                                                                                                                  \
    else                                                                                                               \
        (void)0
#define NewObj new (__szsFile, __LINE__)
void UnmarkAllObjs(void);

const uint32_t fobjNil = 0x00000000L;
const uint32_t fobjNotAllocated = 0x40000000L;
const uint32_t fobjAllocated = 0x20000000L;
const uint32_t fobjAssertFull = 0x10000000L;

extern int32_t vcactSuspendAssertValid;
extern int32_t vcactAVSave;
extern int32_t vcactAV;
inline void SuspendAssertValid(void)
{
    if (0 == vcactSuspendAssertValid++)
    {
        vcactAVSave = vcactAV;
        vcactAV = 0;
    }
}
inline void ResumeAssertValid(void)
{
    if (0 == --vcactSuspendAssertValid)
        vcactAV = vcactAVSave;
}

#define AssertPo(po, grf)                                                                                              \
    if ((po) != 0)                                                                                                     \
    {                                                                                                                  \
        if (vcactAV > 0)                                                                                               \
        {                                                                                                              \
            vcactAV--;                                                                                                 \
            (po)->AssertValid(grf);                                                                                    \
            vcactAV++;                                                                                                 \
        }                                                                                                              \
    }                                                                                                                  \
    else                                                                                                               \
        Bug("nil")
#define AssertNilOrPo(po, grf)                                                                                         \
    if ((po) != 0 && vcactAV > 0)                                                                                      \
    {                                                                                                                  \
        vcactAV--;                                                                                                     \
        (po)->AssertValid(grf);                                                                                        \
        vcactAV++;                                                                                                     \
    }                                                                                                                  \
    else                                                                                                               \
        (void)0
#define AssertBasePo(po, grf)                                                                                          \
    if ((po) != 0)                                                                                                     \
    {                                                                                                                  \
        if (vcactAV > 0)                                                                                               \
        {                                                                                                              \
            vcactAV--;                                                                                                 \
            (po)->BASE::AssertValid(grf);                                                                              \
            vcactAV++;                                                                                                 \
        }                                                                                                              \
    }                                                                                                                  \
    else                                                                                                               \
        Bug("nil")

#define AssertThis(grf)                                                                                                \
    if (vcactAV > 0)                                                                                                   \
    {                                                                                                                  \
        vcactAV--;                                                                                                     \
        this->AssertValid(grf);                                                                                        \
        vcactAV++;                                                                                                     \
    }                                                                                                                  \
    else                                                                                                               \
        (void)0
#define AssertBaseThis(grf)                                                                                            \
    if (vcactAV > 0)                                                                                                   \
    {                                                                                                                  \
        vcactAV--;                                                                                                     \
        this->BASE::AssertValid(grf);                                                                                  \
        vcactAV++;                                                                                                     \
    }                                                                                                                  \
    else                                                                                                               \
        (void)0

#define MARKMEM                                                                                                        \
  public:                                                                                                              \
    virtual void MarkMem(void) override;

#define MARKMEM_BASE                                                                                                   \
  public:                                                                                                              \
    virtual void MarkMem(void);

#define ASSERT                                                                                                         \
  public:                                                                                                              \
    void AssertValid(uint32_t grf);
#define NOCOPY(cls)                                                                                                    \
  private:                                                                                                             \
    cls &operator=(cls &robj)                                                                                          \
    {                                                                                                                  \
        __AssertOnCopy();                                                                                              \
        return *this;                                                                                                  \
    }
void __AssertOnCopy(void);
void MarkUtilMem(void);

#else //! DEBUG

#define SuspendAssertValid()
#define ResumeAssertValid()
#define AssertUnmarkedObjs()
#define MarkMemObj(po)
#define NewObj new
#define UnmarkAllObjs()
#define AssertPo(po, grf)
#define AssertNilOrPo(po, grf)
#define AssertBasePo(po, grf)
#define AssertThis(grf)
#define AssertBaseThis(grf)
#define MARKMEM
#define MARKMEM_BASE
#define ASSERT
#define NOCOPY(cls)
#define MarkUtilMem()

#endif //! DEBUG

/***************************************************************************
    Macro to release an object and clear the pointer to it.
***************************************************************************/
#define ReleasePpo(ppo)                                                                                                \
    if (*(ppo) != pvNil)                                                                                               \
    {                                                                                                                  \
        (*(ppo))->Release();                                                                                           \
        *(ppo) = pvNil;                                                                                                \
    }                                                                                                                  \
    else                                                                                                               \
        (void)0

/***************************************************************************
    Base class. Any instances allocated using NewObj (as opposed to being
    on the stack) are guaranteed to be zero'ed out. Also provides reference
    counting and debug lost memory checks.
***************************************************************************/
#define kclsBASE 'BASE'
class BASE
{
    RTCLASS_DEC_BASE
    MARKMEM_BASE
    ASSERT

  private:
    Debug(int32_t _lwMagic;)

        protected : int32_t _cactRef;

  public:
#ifdef DEBUG
    void *operator new(size_t cb, schar *pszsFile, int32_t lwLine) noexcept;
    void operator delete(void *pv, schar *pszsFile, int32_t lwLine); // To prevent warning C4291
    void operator delete(void *pv);
    void MarkMemStub(void);
#else //! DEBUG
    void *operator new(size_t cb) noexcept;
#ifdef WIN
    void operator delete(void *pv);
#endif // WIN
#endif //! DEBUG
    BASE(void);
    virtual ~BASE(void)
    {
        AssertThis(0);
    }

    virtual void AddRef(void);
    virtual void Release(void);
    int32_t CactRef(void)
    {
        return _cactRef;
    }
};

/***************************************************************************
    Base linked list
***************************************************************************/
#define BLL_DEC(cls, rtn)                                                                                              \
  public:                                                                                                              \
    class cls *rtn(void)                                                                                               \
    {                                                                                                                  \
        return (class cls *)BLL::PbllNext();                                                                           \
    }

typedef class BLL *PBLL;
#define BLL_PAR BASE
#define kclsBLL 'BLL'
class BLL : public BLL_PAR
{
    RTCLASS_DEC
    ASSERT

  private:
    PBLL _pbllNext;
    PBLL *_ppbllPrev;

  protected:
    void _Attach(void *ppbllPrev);

  public:
    BLL(void);
    ~BLL(void);

    PBLL PbllNext(void)
    {
        return _pbllNext;
    }
};

#endif //! BASE_H
