/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    A lexer for command scripts.

***************************************************************************/
#ifndef LEX_H
#define LEX_H

enum
{
    fctNil = 0,   // invalid character
    fctLow = 1,   // lowercase letter
    fctUpp = 2,   // uppercase letter
    fctOct = 4,   // octal
    fctDec = 8,   // digit
    fctHex = 16,  // hex digit
    fctSpc = 32,  // space character
    fctOp1 = 64,  // first character of a multi-character operator
    fctOp2 = 128, // last character of a multi-character operator
    fctOpr = 256, // lone character operator
    fctQuo = 512, // quote character
};
#define kgrfctDigit (fctOct | fctDec | fctHex)

enum
{
    ttNil,
    ttError,  // bad token
    ttLong,   // numeric constant
    ttName,   // identifier
    ttString, // string constant

    ttAdd,        // +
    ttSub,        // -
    ttMul,        // *
    ttDiv,        // /
    ttMod,        // %
    ttInc,        // ++
    ttDec,        // --
    ttBOr,        // |
    ttBAnd,       // &
    ttBXor,       // ^
    ttBNot,       // ~
    ttShr,        // >>
    ttShl,        // <<
    ttLOr,        // ||
    ttLAnd,       // &&
    ttLXor,       // ^^
    ttEq,         // ==
    ttNe,         // !=
    ttGt,         // >
    ttGe,         // >=
    ttLt,         // <
    ttLe,         // <=
    ttLNot,       // !
    ttAssign,     // =
    ttAAdd,       // +=
    ttASub,       // -=
    ttAMul,       // *=
    ttADiv,       // /=
    ttAMod,       // %=
    ttABOr,       // |=
    ttABAnd,      // &=
    ttABXor,      // ^=
    ttAShr,       // >>=
    ttAShl,       // <<=
    ttArrow,      // ->
    ttDot,        // .
    ttQuery,      // ?
    ttColon,      // :
    ttComma,      // ,
    ttSemi,       // ;
    ttOpenRef,    // [
    ttCloseRef,   // ]
    ttOpenParen,  // (
    ttCloseParen, // )
    ttOpenBrace,  // {
    ttCloseBrace, // }
    ttPound,      // #
    ttDollar,     // $
    ttAt,         // @
    ttAccent,     // `
    ttBackSlash,  // backslash character (\)
    ttScope,      // ::

    ttLimBase
};

struct TOK
{
    int32_t tt;
    int32_t lw;
    STN stn;
};
typedef TOK *PTOK;

/***************************************************************************
    Base lexer.
***************************************************************************/
#define kcchLexbBuf 512

typedef class LEXB *PLEXB;
#define LEXB_PAR BASE
#define kclsLEXB KLCONST4('L', 'E', 'X', 'B')
class LEXB : public LEXB_PAR
{
    RTCLASS_DEC
    ASSERT
    MARKMEM

  protected:
    static uint16_t _mpchgrfct[];

    PFIL _pfil; // exactly one of _pfil, _pbsf should be non-nil
    PBSF _pbsf;
    STN _stnFile;
    int32_t _lwLine;  // which line
    int32_t _ichLine; // which character on the line

    FP _fpCur;
    FP _fpMac;
    int32_t _ichLim;
    int32_t _ichCur;
    achar _rgch[kcchLexbBuf];
    bool _fLineStart : 1;
    bool _fSkipToNextLine : 1;
    bool _fUnionStrings : 1;

    uint32_t _GrfctCh(achar ch)
    {
        return (uchar)ch < 128 ? _mpchgrfct[(uint8_t)ch] : fctNil;
    }
    bool _FFetchRgch(achar *prgch, int32_t cch = 1);
    void _Advance(int32_t cch = 1)
    {
        _ichCur += cch;
        _ichLine += cch;
    }
    bool _FSkipWhiteSpace(void);
    virtual void _ReadNumber(int32_t *plw, achar ch, int32_t lwBase, int32_t cchMax);
    virtual void _ReadNumTok(PTOK ptok, achar ch, int32_t lwBase, int32_t cchMax)
    {
        _ReadNumber(&ptok->lw, ch, lwBase, cchMax);
    }
    bool _FReadHex(int32_t *plw);
    bool _FReadControlCh(achar *pch);

  public:
    LEXB(PFIL pfil, bool fUnionStrings = fTrue);
    LEXB(PBSF pbsf, PSTN pstnFile, bool fUnionStrings = fTrue);
    ~LEXB(void);

    virtual bool FGetTok(PTOK ptok);
    virtual int32_t CbExtra(void);
    virtual void GetExtra(void *pv);

    void GetStnFile(PSTN pstn);
    int32_t LwLine(void)
    {
        return _lwLine;
    }
    int32_t IchLine(void)
    {
        return _ichLine;
    }
};

#endif //! LEX_H
