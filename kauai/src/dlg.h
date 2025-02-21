/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Dialog header file.

***************************************************************************/
#ifndef DLG_H
#define DLG_H

#ifdef MAC
typedef DialogPtr HDLG;
#endif // MAC
#ifdef WIN
typedef HWND HDLG;
#endif // WIN

// type of dialog item
enum
{
    ditkButton,     // no value
    ditkCheckBox,   // long (bool)
    ditkRadioGroup, // long (index)
    ditkEditText,   // streamed stn
    ditkCombo,      // streamed stn, followed by a list of streamed stn's.
    ditkLim
};

// dialog item
struct DIT
{
    int32_t sitMin; // first system item number (for this DIT)
    int32_t sitLim; // lim of system item numbers (for this DIT)
    int32_t ditk;   // kind of item
};

typedef class DLG *PDLG;

// callback to notify of an item change (while the dialog is active)
typedef bool (*PFNDLG)(PDLG pdlg, int32_t *pidit, void *pv);

// dialog class - a DLG is a GG of DITs
#define DLG_PAR GG
#define kclsDLG 'DLG'
class DLG : public DLG_PAR
{
    RTCLASS_DEC

  private:
    PGOB _pgob;
    int32_t _rid;
    PFNDLG _pfn;
    void *_pv;

#ifdef WIN
    friend BOOL CALLBACK _FDlgCore(HWND hdlg, UINT msg, WPARAM w, LPARAM lw);
#endif // WIN

    DLG(int32_t rid);
    bool _FInit(void);

    int32_t _LwGetRadioGroup(int32_t idit);
    void _SetRadioGroup(int32_t idit, int32_t lw);
    bool _FGetCheckBox(int32_t idit);
    void _InvertCheckBox(int32_t idit);
    void _SetCheckBox(int32_t idit, bool fOn);
    void _GetEditText(int32_t idit, PSTN pstn);
    void _SetEditText(int32_t idit, PSTN pstn);
    bool _FDitChange(int32_t *pidit);
    bool _FAddToList(int32_t idit, PSTN pstn);
    void _ClearList(int32_t idit);

  public:
    static PDLG PdlgNew(int32_t rid, PFNDLG pfn = pvNil, void *pv = pvNil);

    int32_t IditDo(int32_t iditFocus = ivNil);

    // these are only valid while the dialog is up
    bool FGetValues(int32_t iditMin, int32_t iditLim);
    void SetValues(int32_t iditMin, int32_t iditLim);
    void SelectDit(int32_t idit);

    // argument access
    int32_t IditFromSit(int32_t sit);
    void GetDit(int32_t idit, DIT *pdit)
    {
        GetFixed(idit, pdit);
    }
    void PutDit(int32_t idit, DIT *pdit)
    {
        PutFixed(idit, pdit);
    }

    void GetStn(int32_t idit, PSTN pstn);
    bool FPutStn(int32_t idit, PSTN pstn);
    int32_t LwGetRadio(int32_t idit);
    void PutRadio(int32_t idit, int32_t lw);
    bool FGetCheck(int32_t idit);
    void PutCheck(int32_t idit, bool fOn);

    bool FGetLwFromEdit(int32_t idit, int32_t *plw, bool *pfEmpty = pvNil);
    bool FPutLwInEdit(int32_t idit, int32_t lw);

    bool FAddToList(int32_t idit, PSTN pstn);
    void ClearList(int32_t idit);
};

#endif //! DLG_H
