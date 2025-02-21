/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Data movement declarations

***************************************************************************/
#ifndef UTILCOPY_H
#define UTILCOPY_H

void BltPb(const void *pv1, void *pv2, int32_t cb);
void CopyPb(const void *pv1, void *pv2, int32_t cb);
void ClearPb(void *pv, int32_t cb);
void FillPb(void *pv1, int32_t cb, uint8_t b);
void ReversePb(void *pv, int32_t cb);
void ReverseRgsw(void *pv, int32_t csw);
void ReverseRglw(void *pv, int32_t clw);
void SwapBlocks(void *pv, int32_t cb1, int32_t cb2);
void SwapPb(void *pv1, void *pv2, int32_t cb);
void MoveElement(void *prgv, int32_t cbElement, int32_t ivSrc, int32_t ivTarget);
bool FEqualRgb(const void *pv1, const void *pv2, int32_t cb);
int32_t CbEqualRgb(const void *pv1, const void *pv2, int32_t cbMax);
uint32_t FcmpCompareRgb(const void *pv1, const void *pv2, int32_t cb);

#ifdef DEBUG
#define SwapVars(pv1, pv2)                                                                                             \
    if (SIZEOF(*pv1) != SIZEOF(*pv2))                                                                                  \
        Bug("sizes don't match");                                                                                      \
    else                                                                                                               \
        SwapPb(pv1, pv2, SIZEOF(*pv1))
#else //! DEBUG
#define SwapVars(pv1, pv2) SwapPb(pv1, pv2, SIZEOF(*pv1))
#endif //! DEBUG

#endif // UTILCOPY_H
