/**
 * KauaiGui tests
 **/
#include <gtest/gtest.h>

#include "util.h"
#include "frame.h"
ASSERTNAME

TEST(RegionScannerTests, OverlappingRectangles)
{
    RC rc;

    // Create an empty region
    PREGN pregn = REGN::PregnNew(pvNil);
    AssertPo(pregn, 0);

    // Create two overlapping rectangles at (2, 2)-(8, 8) and (4, 4)-(10, 10)
    rc.Set(2, 2, 8, 8);
    pregn->FUnionRc(&rc);

    rc.Offset(2, 2);
    pregn->FUnionRc(&rc);

    RC rcScreen;
    rcScreen.Set(0, 0, 640, 480);

    // Create the region scanner
    REGSC regsc;
    regsc.Init(pregn, &rcScreen);

    int32_t dyp = regsc.DypCur();
    ASSERT_EQ(dyp, 2) << "DypCur() should be the first line containing a rectangle";

    int32_t xpStart = 0, xpEnd = 0;
    bool fFound = fFalse;

    // Scan to find the first line
    while (!fFound)
    {
        xpStart = regsc.XpCur();

        if (xpStart == klwMax)
        {
            // Keep going until we find part of a region
            regsc.ScanNext(1);
        }
        else
        {
            // Found it
            fFound = fTrue;
        }
    }

    // List of starting pixels
    int32_t rgxpStart[9] = {2, 2, 2, 2, 2, 2, 4, 4, klwMax};
    // List of ending pixels
    int32_t rgxpEnd[9] = {8, 8, 10, 10, 10, 10, 10, 10, klwMax};

    for (int32_t i = 0; i < CvFromRgv(rgxpStart); i++)
    {
        ASSERT_EQ(xpStart, rgxpStart[i]) << "X coordinate of start of run does not match expected value";

        int32_t xpEnd = regsc.XpFetch();
        ASSERT_EQ(xpEnd, rgxpEnd[i]) << "X coordinate of end of run does not match expected value";

        regsc.ScanNext(1);
        xpStart = regsc.XpCur();
    }

    ReleasePpo(&pregn);
}

TEST(RegionScannerTests, AdjacentRectangles)
{
    RC rc;

    // Create an empty region
    PREGN pregn = REGN::PregnNew(pvNil);
    AssertPo(pregn, 0);

    // Create two adjacent rectangles
    rc.Set(2, 2, 8, 8);
    pregn->FUnionRc(&rc);

    rc.Offset(16, 0);
    pregn->FUnionRc(&rc);

    RC rcScreen;
    rcScreen.Set(0, 0, 640, 480);

    // Create the region scanner
    REGSC regsc;
    regsc.Init(pregn, &rcScreen);

    int32_t dyp = regsc.DypCur();
    ASSERT_EQ(dyp, 2) << "DypCur() should be the first line containing a rectangle";

    int32_t xp = 0;
    bool fFound = fFalse;

    // Scan to find the first line
    while (!fFound)
    {
        xp = regsc.XpCur();

        if (xp == klwMax)
        {
            // Keep going until we find part of a region
            regsc.ScanNext(1);
        }
        else
        {
            // Found it
            fFound = fTrue;
        }
    }

    ASSERT_TRUE(regsc.FOn());
    ASSERT_EQ(xp, 2) << "X coordinate of start of run incorrect";

    // Find the end of the first line
    xp = regsc.XpFetch();
    ASSERT_FALSE(regsc.FOn());
    ASSERT_EQ(xp, 8);

    // Find the start of the second line
    xp = regsc.XpFetch();
    ASSERT_TRUE(regsc.FOn());
    ASSERT_EQ(xp, 18);

    // Find the end of the second line
    xp = regsc.XpFetch();
    ASSERT_FALSE(regsc.FOn());
    ASSERT_EQ(xp, 24);

    // There should be no more lines
    xp = regsc.XpFetch();
    ASSERT_EQ(xp, klwMax);

    ReleasePpo(&pregn);
}
