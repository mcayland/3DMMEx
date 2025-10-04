/***************************************************************************
    Author: Ben Stone
    Project: Kauai
    Reviewed:

    SDL font enumeration for Windows

***************************************************************************/
#include "frame.h"
ASSERTNAME

#include "gfx.h"
#include "fontsdl.h"

#include <shlobj.h>

/***************************************************************************
    Get the path to the system font directory
***************************************************************************/
static bool FFindSystemFontDir(PFNI pfniSystemFontDir)
{
    AssertPo(pfniSystemFontDir, 0);

    pfniSystemFontDir->SetNil();

    STN stnSystemFontDir;
    achar szSystemFontDir[MAX_PATH];
    HRESULT hr;

    hr = SHGetFolderPath(NULL, CSIDL_FONTS, NULL, 0, szSystemFontDir);
    if (FAILED(hr))
        return fFalse;

    Assert(CchSz(szSystemFontDir) < kcchMaxSz, "System font path does not fit in a STN");
    if (CchSz(szSystemFontDir) >= kcchMaxSz)
        return fFalse;
    stnSystemFontDir.SetSz(szSystemFontDir);

    return pfniSystemFontDir->FBuildFromPath(&stnSystemFontDir, kftgDir);
}

/***************************************************************************
    Find a font to use as the default system font
***************************************************************************/
static bool FFindDefaultFontFile(PFNI pfniFontDir, PFNI pfniDefaultFont)
{
    AssertPo(pfniFontDir, 0);
    AssertPo(pfniDefaultFont, 0);

    STN stn;
    FNE fne;
    FTG ftgTtf = kftgTtf;

    // Check for any of these default font files:
    PCSZ rgpszDefaultFontFiles[] = {
        PszLit("vgasys.fon"), // System (Windows)
        PszLit("comic.ttf"),  // Comic Sans MS
    };

    for (int32_t ipsz = 0; ipsz < CvFromRgv(rgpszDefaultFontFiles); ipsz++)
    {
        stn = rgpszDefaultFontFiles[ipsz];
        FNI fniFont = *pfniFontDir;
        if (!fniFont.FSetLeaf(&stn))
            continue;

        if (fniFont.TExists() == tYes)
        {
            *pfniDefaultFont = fniFont;
            return fTrue;
        }
    }

    // Return the first TrueType font file in the font directory
    if (!fne.FInit(pfniFontDir, &ftgTtf, 1, 0))
        return fFalse;

    return fne.FNextFni(pfniDefaultFont);
}

bool NTL::_FLoadFontTable()
{
    AssertThis(0);
    Assert(fInitTtf, "TTF_Init should have been called");

    bool fRet = fFalse;
    PGL pglsdlfont = pvNil;
    FNI fniFontDir, fniDefaultFont;

    // FUTURE: Add support for per-user fonts
    if (!FFindSystemFontDir(&fniFontDir))
    {
        Bug("Could not find system fonts directory");
        goto LFail;
    }

    if (!FAddAllFontsInDir(&fniFontDir))
    {
        Bug("Could not add fonts from system fonts directory");
        goto LFail;
    }

    // Ensure we have at least one font
    if (_pgst->IvMac() == 0)
    {
        goto LFail;
    }

    if (FFindDefaultFontFile(&fniFontDir, &fniDefaultFont))
    {
        int32_t onnSystem = 0;
        STN stnT = PszLit("System Default");
        if (FAddFontFile(&fniDefaultFont, &stnT, &onnSystem))
        {
            _onnSystem = onnSystem;
        }
    }
    else
    {
        Bug("Could not find a default font file");
        goto LFail;
    }

    fRet = fTrue;

LFail:
    return fRet;
}
