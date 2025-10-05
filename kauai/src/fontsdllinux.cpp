/***************************************************************************
    Author: Ben Stone
    Project: Kauai
    Reviewed:

    SDL font enumeration for Linux/Unix platforms using Fontconfig

***************************************************************************/
#include "frame.h"
ASSERTNAME

#include "gfx.h"
#include "fontsdl.h"

#include <fontconfig/fontconfig.h>

// Find a default font to use when the requested font is unavailable
static bool FFindSystemFont(PFNI pfniSystemFont)
{
    AssertPo(pfniSystemFont, 0);

    bool fRet = fFalse;
    FcPattern *ppattern = pvNil, *pmatch = pvNil;
    FcResult result;

    ppattern = FcNameParse((FcChar8 *)"sans");
    if (ppattern == pvNil)
        goto LFail;

    FcConfigSubstitute(pvNil, ppattern, FcMatchPattern);
    FcDefaultSubstitute(ppattern);

    pmatch = FcFontMatch(pvNil, ppattern, &result);
    if (pmatch != pvNil && result == FcResultMatch)
    {
        FcChar8 *pu8szFontFile = pvNil;
        if (FcPatternGetString(pmatch, FC_FILE, 0, &pu8szFontFile) == FcResultMatch)
        {
            STN stnFontFile;
            stnFontFile.SetUtf8Sz((PU8SZ)pu8szFontFile);
            fRet = pfniSystemFont->FBuildFromPath(&stnFontFile);
        }
    }

LFail:
    FcPatternDestroy(pmatch);
    FcPatternDestroy(ppattern);
    return fRet;
}

bool NTL::_FLoadFontTable()
{
    AssertThis(0);
    Assert(fInitTtf, "TTF_Init should have been called");

    bool fRet = fFalse;
    FcConfig *pconfig = pvNil;
    FcPattern *ppattern = pvNil;
    FcObjectSet *pobjectset = pvNil;
    FcFontSet *pfontset = pvNil;
    FcResult result;
    FNI fniSystemFont;

    // Initialise Fontconfig
    AssertDo(pconfig = FcInitLoadConfigAndFonts(), "FcInitLoadConfigAndFonts failed");
    if (pvNil == pconfig)
        goto LFail;

    // Find all available fonts
    AssertDo(ppattern = FcPatternCreate(), "Could not create pattern");
    if (pvNil == ppattern)
        goto LFail;

    AssertDo(pobjectset = FcObjectSetBuild(FC_FAMILY, FC_FILE, (void *)0), "Could not build object set");
    if (pvNil == pobjectset)
        goto LFail;

    AssertDo(pfontset = FcFontList(pconfig, ppattern, pobjectset), "Could not get font list");
    if (pvNil == pfontset)
        goto LFail;

    // Add each font to the font list
    for (int32_t ifont = 0; ifont < pfontset->nfont; ifont++)
    {
        FcPattern *pfont = pfontset->fonts[ifont];
        FcChar8 *pu8szFontFile = pvNil;

        if (FcPatternGetString(pfont, FC_FILE, 0, &pu8szFontFile) == FcResultMatch)
        {
            STN stnFontFile;
            stnFontFile.SetUtf8Sz((PU8SZ)pu8szFontFile);

            FNI fniFontFile;
            if (fniFontFile.FBuildFromPath(&stnFontFile))
            {
                AssertDo(FAddFontFile(&fniFontFile), "Could not add font file");
            }
        }
    }

    if (FFindSystemFont(&fniSystemFont))
    {
        STN stnSystemFont = PszLit("System");
        AssertDo(FAddFontFile(&fniSystemFont, &stnSystemFont, &_onnSystem), "Could not add system font");
    }
    else
    {
        Bug("Could not find a default font");
        goto LFail;
    }

    fRet = fTrue;

LFail:
    FcFontSetDestroy(pfontset);
    FcObjectSetDestroy(pobjectset);
    FcPatternDestroy(ppattern);
    FcConfigDestroy(pconfig);
    FcFini();

    return fRet;
}
