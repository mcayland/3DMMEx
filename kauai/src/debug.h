/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Debug routines.

***************************************************************************/
#ifndef DEBUG_H
#define DEBUG_H

#ifdef WIN
inline void Debugger(void)
{
    DebugBreak();
}
#endif // WIN

#ifdef DEBUG
bool FAssertProc(schar *pszsFile, int32_t lwLine, schar *pszsMsg, void *pv, int32_t cb);
void WarnProc(schar *pszsFile, int32_t lwLine, schar *pszsMsg);
#define ASSERTNAME static schar __szsFile[] = __FILE__;

#define AssertCore(f, szs, pv, cb)                                                                                     \
    if (!(f) && FAssertProc(__szsFile, __LINE__, (schar *)szs, pv, cb))                                                \
        Debugger();                                                                                                    \
    else                                                                                                               \
        (void)(0)

#define Warn(szs) WarnProc(__szsFile, __LINE__, (schar *)szs)
#define Bug(szs) AssertCore(fFalse, szs, 0, 0)
#define Assert(f, szs) AssertCore(f, szs, 0, 0)
#define AssertVar(f, szs, pvar) AssertCore(f, szs, pvar, SIZEOF(*(pvar)))
#define BugVar(szs, pvar) AssertVar(fFalse, szs, pvar)
#define AssertDo(f, szs) Assert(f, szs)
#define AssertDoVar(f, szs, pvar) AssertVar(f, szs, pvar)
#define AssertDoCore(f, szs, pv, cb) AssertCore(f, szs, pv, cb)
#define Debug(foo) foo
#define DebugShip(dbg, shp) dbg

// these Asserts are for use in a header file
#define AssertH(f)                                                                                                     \
    if (!(f) && FAssertProc(pvNil, __LINE__, pvNil, pvNil, 0))                                                         \
        Debugger();                                                                                                    \
    else                                                                                                               \
        (void)(0)
#define BugH() AssertH(fFalse)

#else //! DEBUG

#define ASSERTNAME
#define Debugger()
#define Warn(szs)
#define Bug(szs)
#define Assert(f, szs)
#define AssertVar(f, szs, pvar)
#define BugVar(szs, pvar)
#define AssertCore(f, szs, pv, cb)
#define AssertDo(f, szs) (f)
#define AssertDoVar(f, szs, pvar) (f)
#define AssertDoCore(f, szs, pv, cb) (f)
#define Debug(foo)
#define DebugShip(dbg, shp) shp
#define AssertH(f)

#endif //! DEBUG

#define RawRtn() Bug("Unimplemented Code")
#define NewCode() Bug("Untested Code")

// Use this static assert to ensure structures that are part of the file format do not change in size
//#define VERIFY_STRUCT_SIZE(STRUCT_NAME, STRUCT_SIZE)                                                                   \
//    static_assert(sizeof(STRUCT_NAME) == STRUCT_SIZE, "Size of " #STRUCT_NAME " does not match file format");
#define VERIFY_STRUCT_SIZE(X, Y)

#endif //! DEBUG_H
