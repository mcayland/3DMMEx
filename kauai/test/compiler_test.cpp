/**
 * Kauai compiler tests
 **/
#include <gtest/gtest.h>
#include <cstdint>

#include "util.h"
#include "resources.h"
#include "chcm.h"

ASSERTNAME

/**
 * Message sink class that logs compiler errors as test failures
 **/
class MessageSink : public MSNK
{
  public:
    virtual void ReportLine(const PCSZ psz) override
    {
        ADD_FAILURE() << "Compile error: " << psz;
    }
    virtual void Report(const PCSZ psz) override
    {
        ADD_FAILURE() << "Compile error: " << psz;
    }
    virtual bool FError(void) override
    {
        return fFalse;
    }
};

/**
 * Message sink class that ignores compiler errors
 */
class MessageSinkIgnoreError : public MSNK
{
  public:
    virtual void ReportLine(const PCSZ psz) override
    {
        // do nothing
    }
    virtual void Report(const PCSZ psz) override
    {
        // do nothing
    }
    virtual bool FError(void) override
    {
        return fFalse;
    }
};

TEST(KauaiCompilerTests, TestSearchPath)
{
    PCHCM pchcm = pvNil;
    MessageSink msnk;
    MessageSinkIgnoreError msnkIgnoreError;
    PCFL pcflCompiled = pvNil;

    STN stnSrc;
    FNI fniSrc, fniDst;

    // Find the source and destination files
    GetTestResource(&fniSrc, PszLit("search_path.cht"));
    AssertDo(fniDst.FGetTemp(), "Couldn't create temp file");

    // Compile without the search path set. This should fail.
    pchcm = NewObj CHCM();
    AssertPo(pchcm, 0);
    pcflCompiled = pchcm->PcflCompile(&fniSrc, &fniDst, &msnkIgnoreError);
    ASSERT_TRUE(pcflCompiled == pvNil);
    ReleasePpo(&pchcm);

    // Compile with the search path set to the test resources directory
    pchcm = NewObj CHCM();
    AssertPo(pchcm, 0);
    FNI fniTestResourcePath;
    GetTestResourcePath(&fniTestResourcePath);

    STN stnTestResourcePath;
    fniTestResourcePath.GetStnPath(&stnTestResourcePath);
    AssertDo(pchcm->FSetSearchPath(stnTestResourcePath.Psz()), "Could not set search path");

    pcflCompiled = pchcm->PcflCompile(&fniSrc, &fniDst, &msnk);
    ASSERT_TRUE(pcflCompiled != pvNil) << "Script compilation failed";
    AssertNilOrPo(pcflCompiled, 0);
    ReleasePpo(&pcflCompiled);

    if (fniDst.TExists())
    {
        fniDst.FDelete();
    }

    ReleasePpo(&pchcm);
    UnmarkAllObjs();
    UnmarkAllMem();
    AssertUnmarkedMem();
}
