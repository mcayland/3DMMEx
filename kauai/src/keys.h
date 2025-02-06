/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Reviewed:
    Copyright (c) Microsoft Corporation

    Declarations related to key handling

***************************************************************************/
#ifndef KEYS_H
#define KEYS_H

#define vkNil 0

#ifdef WIN

#define kvkLeft VK_LEFT
#define kvkRight VK_RIGHT
#define kvkUp VK_UP
#define kvkDown VK_DOWN
#define kvkHome VK_HOME
#define kvkEnd VK_END
#define kvkPageUp VK_PRIOR
#define kvkPageDown VK_NEXT

#define kvkBack VK_BACK
#define kvkDelete VK_DELETE
#define kvkReturn VK_RETURN

#else

#define kvkLeft 0x1C
#define kvkRight 0x1D
#define kvkUp 0x1E
#define kvkDown 0x1F
#define kvkHome 0x01
#define kvkEnd 0x04
#define kvkPageUp 0x0B
#define kvkPageDown 0x0C

#define kvkBack 0x08
#define kvkDelete 0x7F
#define kvkReturn 0x0D

#endif

#endif //! KEYS_H
