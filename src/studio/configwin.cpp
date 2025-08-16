/***************************************************************************

    configwin.cpp: Store app configuration in the Windows registry

    This module implements FGetSetRegKey, which was previously part of the
    APP class.

***************************************************************************/

#include "studio.h"

ASSERTNAME

bool FGetSetRegKey(PCSZ pszValueName, void *pvData, int32_t cbData, uint32_t grfreg, bool *pfNoValue)
{
    AssertSz(pszValueName);
    AssertPvCb(pvData, cbData);
    AssertNilOrVarMem(pfNoValue);

    bool fRet = fFalse;
    bool fSetKey, fSetDefault, fString, fBinary;

    fSetKey = grfreg & fregSetKey;
    fSetDefault = grfreg & fregSetDefault;
    fString = grfreg & fregString;
    fBinary = grfreg & fregBinary;

#ifdef WIN
    DWORD dwDisposition;
    DWORD dwCbData = cbData;
    DWORD dwType;
    int32_t lwRet;
    HKEY hkey = 0;
    REGSAM samDesired = (fSetKey || fSetDefault) ? KEY_ALL_ACCESS : KEY_READ;

    lwRet = RegCreateKeyEx((grfreg & fregMachine) ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, kszSocratesKey, 0, NULL,
                           REG_OPTION_NON_VOLATILE, samDesired, NULL, &hkey, &dwDisposition);
    if (lwRet != ERROR_SUCCESS)
    {
#ifdef DEBUG
        STN stnErr;
        stnErr.FFormatSz(PszLit("Could not open Socrates key: lwRet=0x%x"), lwRet);
        Warn(stnErr.Psz());
#endif // DEBUG
        goto LFail;
    }

    if ((dwDisposition == REG_CREATED_NEW_KEY && fSetDefault) || fSetKey)
    {
    LWriteValue:
        if (fBinary)
        {
            dwType = REG_BINARY;
        }
        else if (fString)
        {
            if (!fSetKey)
                dwCbData = CchSz((PSZ)pvData) + kcchExtraSz;
            else
                Assert(CchSz((PSZ)pvData) < cbData, "Invalid string for reg key");
            dwType = REG_SZ;
        }
        else
        {
            Assert(cbData == SIZEOF(DWORD), "Unknown reg key type");
            dwType = REG_DWORD;
        }

        lwRet = RegSetValueEx(hkey, pszValueName, NULL, dwType, (uint8_t *)pvData, dwCbData);
        if (lwRet != ERROR_SUCCESS)
        {
#ifdef DEBUG
            STN stnErr;
            stnErr.FFormatSz(PszLit("Could not set value %z: lwRet=0x%x"), pszValueName, lwRet);
            Warn(stnErr.Psz());
#endif // DEBUG
            goto LFail;
        }
    }
    else
    {
        lwRet = RegQueryValueEx(hkey, pszValueName, NULL, &dwType, (uint8_t *)pvData, &dwCbData);
        if (lwRet != ERROR_SUCCESS)
        {
            if (lwRet == ERROR_FILE_NOT_FOUND && fSetDefault)
                goto LWriteValue;
            /* If the caller gave us a way to differentiate a genuine registry
                failure from simply not having set the value yet, do so */
            if (pfNoValue != pvNil)
                fRet = *pfNoValue = (lwRet == ERROR_FILE_NOT_FOUND);
            goto LFail;
        }
        Assert(dwType == (DWORD)(fString ? REG_SZ : (fBinary ? REG_BINARY : REG_DWORD)), "Invalid key type");
    }
    if (pfNoValue != pvNil)
        *pfNoValue = fFalse;
    fRet = fTrue;
LFail:
    if (hkey != 0)
        RegCloseKey(hkey);
#else  // WIN
    RawRtn();
#endif // !WIN

    return fRet;
}
