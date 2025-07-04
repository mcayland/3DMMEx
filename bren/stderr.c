/*
 * Copyright (c) 1993 Argonaut Software Ltd. All rights reserved.
 *
 * $Id: stderr.c 1.3 1994/11/07 01:39:18 sam Exp $
 * $Locker: sam $
 *
 * Default error handler that reports error through stderr
 */
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#endif // WIN32

#include "brender.h"

static void BR_CALLBACK BrStdioWarning(char *message)
{
#ifdef WIN32
    MessageBoxA(0, message, "BRender Warning", MB_OK);
#else  // !WIN32
    printf("BRender Warning: %s\n", message);
#endif // WIN32
}

static void BR_CALLBACK BrStdioError(char *message)
{
#ifdef WIN32
    MessageBoxA(0, message, "BRender Fatal Error", MB_OK);
#else  // !WIN32
    printf("BRender Fatal Error: %s\n", message);
#endif // WIN32

    exit(10);
}

/*
 * ErrorHandler structure
 */
br_diaghandler BrStdioDiagHandler = {
    "Stdio DiagHandler",
    BrStdioWarning,
    BrStdioError,
};

/*
 * Override default
 */
br_diaghandler *_BrDefaultDiagHandler = &BrStdioDiagHandler;
