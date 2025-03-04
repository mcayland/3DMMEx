/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Script compiler declarations.

***************************************************************************/
#ifndef SCRCOM_H
#define SCRCOM_H

/***************************************************************************
    Opcodes for scripts - these are the opcodes that can actually exist in
    script.  They don't necessarily map directly to the compiler's notion of
    an operator.
***************************************************************************/
// if you change this enum, bump the version numbers below
enum
{
    opNil,

    // ops that act on variables come first
    kopPushLocVar,
    kopPopLocVar,
    kopPushThisVar,
    kopPopThisVar,
    kopPushGlobalVar,
    kopPopGlobalVar,
    kopPushRemoteVar,
    kopPopRemoteVar,

    kopMinArray = 0x80,
    kopPushLocArray = kopMinArray,
    kopPopLocArray,
    kopPushThisArray,
    kopPopThisArray,
    kopPushGlobalArray,
    kopPopGlobalArray,
    kopPushRemoteArray,
    kopPopRemoteArray,
    kopLimArray,

    kopLimVarSccb = kopLimArray,

    // ops that just act on the stack - no variables
    //  for the comments below, a is the top value on the execution stack
    //  b is the next to top value on the stack, and c is next on the stack
    kopAdd = 0x100, // addition: b + a
    kopSub,         // subtraction: b - a
    kopMul,         // multiplication: b * a
    kopDiv,         // integer division: b / a
    kopMod,         // mod: b % a
    kopNeg,         // unary negation: -a
    kopInc,         // unary increment: a + 1
    kopDec,         // unary decrement: a - 1
    kopShr,         // shift right: b >> a
    kopShl,         // shift left: b << a
    kopBOr,         // bitwise or: b | a
    kopBAnd,        // bitwise and: b & a
    kopBXor,        // bitwise exclusive or: b ^ a
    kopBNot,        // unary bitwise not: ~a
    kopLXor,        // logical exclusive or: (b != 0) != (a != 0)
    kopLNot,        // unary logical not: !a
    kopEq,          // equality: b == a
    kopNe,          // not equal: b != a
    kopGt,          // greater than: b > a
    kopLt,          // less than: b < a
    kopGe,          // greater or equal: b >= a
    kopLe,          // less or equal: b <= a

    kopAbs,         // unary absolute value: LwAbs(a)
    kopRnd,         // a random number between 0 and a - 1 (inclusive)
    kopMulDiv,      // a * b / c without loss of precision
    kopDup,         // duplicates the top value
    kopPop,         // discards the top value
    kopSwap,        // swaps the top two values
    kopRot,         // rotates b values by a (c is placed a slots deeper in the stack)
    kopRev,         // reverses the next a slots on the stack
    kopDupList,     // duplicates the next a slots on the stack
    kopPopList,     // pops the next a slots on the stack
    kopRndList,     // picks one of the next a stack values at random
    kopSelect,      // picks the b'th entry among the next a stack values
    kopGoEq,        // if (b == a) jumps to label c
    kopGoNe,        // if (b != a) jumps to label c
    kopGoGt,        // if (b > a) jumps to label c
    kopGoLt,        // if (b < a) jumps to label c
    kopGoGe,        // if (b >= a) jumps to label c
    kopGoLe,        // if (b <= a) jumps to label c
    kopGoZ,         // if (a == 0) jumps to label b
    kopGoNz,        // if (a != 0) jumps to label b
    kopGo,          // jumps to label a
    kopExit,        // terminates the script
    kopReturn,      // terminates the script with return value a
    kopSetReturn,   // sets the return value to a
    kopShuffle,     // shuffles the numbers 0,1,..,a-1 for calls to NextCard
    kopShuffleList, // shuffles the top a values for calls to NextCard
    kopNextCard,    // returns the next value from the shuffled values
                    // when all values have been used, the values are reshuffled
    kopMatch,       // a is a count of pairs, b is the key, c is the default value
                    // if b matches the first of any of the a pairs, the second
                    // value of the pair is pushed. if not, c is pushed.
    kopPause,       // pause the script (can be resumed later from C code)
    kopCopyStr,     // copy a string within the registry
    kopMoveStr,     // move a string within the registry
    kopNukeStr,     // delete a string from the registry
    kopMergeStrs,   // merge a string table into the registry
    kopScaleTime,   // scale the application clock
    kopNumToStr,    // convert a number to a decimal string
    kopStrToNum,    // convert a string to a number
    kopConcatStrs,  // concatenate two strings
    kopLenStr,      // return the number of characters in the string
    kopCopySubStr,  // copy a piece of the string

    kopLimSccb
};

// structure to map a string to an opcode (post-fix)
struct SZOP
{
    int32_t op;
    PCSZ psz;
};

// structure to map a string to an opcode and argument information (in-fix)
struct AROP
{
    int32_t op;
    PCSZ psz;
    int32_t clwFixed;   // number of fixed arguments
    int32_t clwVar;     // number of arguments per variable group
    int32_t cactMinVar; // minimum number of variable groups
    bool fVoid;         // return a value?
};

// script version numbers
// if you bump these, also bump the numbers in scrcomg.h
const int16_t kswCurSccb = 0xA;  // this version
const int16_t kswBackSccb = 0xA; // we can be read back to this version
const int16_t kswMinSccb = 0xA;  // we can read back to this version

// high byte of a label value
const uint8_t kbLabel = 0xCC;

/***************************************************************************
    Run-time variable name.  The first 8 characters of the name are
    significant.  These 8 characters are packed into lu2 and the low
    2 bytes of lu1, so clients can store the info in 6 bytes. The high
    2 bytes of lu1 are used for array subscripts.
***************************************************************************/
struct RTVN
{
    uint32_t lu1;
    uint32_t lu2;

    void SetFromStn(PSTN pstn);
    void GetStn(PSTN pstn);
};

/***************************************************************************
    The script compiler base class.
***************************************************************************/
typedef class SCCB *PSCCB;
#define SCCB_PAR BASE
#define kclsSCCB KLCONST4('S', 'C', 'C', 'B')
class SCCB : public SCCB_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    enum
    {
        fsccNil = 0,
        fsccWantVoid = 1,
        fsccTop = 2
    };

    PLEXB _plexb;         // the lexer
    PSCPT _pscpt;         // the script we're building
    PGL _pgletnTree;      // expression tree (in-fix only)
    PGL _pgletnStack;     // token stack for building expression tree (in-fix only)
    PGL _pglcstd;         // control structure stack (in-fix only)
    PGST _pgstNames;      // encountered names (in-fix only)
    PGST _pgstLabel;      // encountered labels, sorted, extra long is label value
    PGST _pgstReq;        // label references, extra long is address of reference
    int32_t _ilwOpLast;   // address of the last opcode
    int32_t _lwLastLabel; // for internal temporary labels
    bool _fError : 1;     // whether an error has occured during compiling
    bool _fForceOp : 1;   // when pushing a constant, make sure the last long
                          // is an opcode (because a label references this loc)
    bool _fHitEnd : 1;    // we've exhausted our input stream
    int32_t _ttEnd;       // stop compiling when we see this
    PMSNK _pmsnk;         // the message sink - for error reporting when compiling

    bool _FInit(PLEXB plexb, bool fInFix, PMSNK pmsnk);
    void _Free(void);

    // general compilation methods
    void _PushLw(int32_t lw);
    void _PushString(PSTN pstn);
    void _PushOp(int32_t op);
    void _EndOp(void);
    void _PushVarOp(int32_t op, RTVN *prtvn);
    bool _FFindLabel(PSTN pstn, int32_t *plwLoc);
    void _AddLabel(PSTN pstn);
    void _PushLabelRequest(PSTN pstn);
    void _AddLabelLw(int32_t lw);
    void _PushLabelRequestLw(int32_t lw);

    virtual void _ReportError(PCSZ psz);
    virtual int16_t _SwCur(void);
    virtual int16_t _SwBack(void);
    virtual int16_t _SwMin(void);

    virtual bool _FGetTok(PTOK ptok);

    // post-fix compiler routines
    virtual void _CompilePost(void);
    int32_t _OpFromStnRgszop(PSTN pstn, SZOP *prgszop);
    virtual int32_t _OpFromStn(PSTN pstn);
    bool _FGetStnFromOpRgszop(int32_t op, PSTN pstn, SZOP *prgszop);
    virtual bool _FGetStnFromOp(int32_t op, PSTN pstn);

    // in-fix compiler routines
    virtual void _CompileIn(void);
    bool _FResolveToOpl(int32_t opl, int32_t oplMin, int32_t *pietn);
    void _EmitCode(int32_t ietnTop, uint32_t grfscc, int32_t *pclwArg);
    void _EmitVarAccess(int32_t ietn, RTVN *prtvn, int32_t *popPush, int32_t *popPop, int32_t *pclwStack);
    virtual bool _FGetOpFromName(PSTN pstn, int32_t *pop, int32_t *pclwFixed, int32_t *pclwVar, int32_t *pcactMinVar,
                                 bool *pfVoid);
    bool _FGetArop(PSTN pstn, AROP *prgarop, int32_t *pop, int32_t *pclwFixed, int32_t *pclwVar, int32_t *pcactMinVar,
                   bool *pfVoid);
    void _PushLabelRequestIetn(int32_t ietn);
    void _AddLabelIetn(int32_t ietn);
    void _PushOpFromName(int32_t ietn, uint32_t grfscc, int32_t clwArg);
    void _GetIstnNameFromIetn(int32_t ietn, int32_t *pistn);
    void _GetRtvnFromName(int32_t istn, RTVN *prtvn);
    bool _FKeyWord(PSTN pstn);
    void _GetStnFromIstn(int32_t istn, PSTN pstn);
    void _AddNameRef(PSTN pstn, int32_t *pistn);
    int32_t _CstFromName(int32_t ietn);
    void _BeginCst(int32_t cst, int32_t ietn);
    bool _FHandleCst(int32_t ietn);
    bool _FAddToTree(struct ETN *petn, int32_t *pietn);
    bool _FConstEtn(int32_t ietn, int32_t *plw);
    bool _FCombineConstValues(int32_t op, int32_t lw1, int32_t lw2, int32_t *plw);
    void _SetDepth(struct ETN *petn, bool fCommute = fFalse);
    void _PushStringIstn(int32_t istn);

  public:
    SCCB(void);
    ~SCCB(void);

    virtual PSCPT PscptCompileLex(PLEXB plexb, bool fInFix, PMSNK pmsnk, int32_t ttEnd = ttNil);
    virtual PSCPT PscptCompileFil(PFIL pfil, bool fInFix, PMSNK pmsnk);
    virtual PSCPT PscptCompileFni(FNI *pfni, bool fInFix, PMSNK pmsnk);
    virtual bool FDisassemble(PSCPT pscpt, PMSNK pmsnk, PMSNK pmsnkError = pvNil);
};

#endif //! SCRCOM_H
