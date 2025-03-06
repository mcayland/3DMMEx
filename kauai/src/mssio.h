/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/***************************************************************************
    Author: ShonK
    Project: Kauai
    Copyright (c) Microsoft Corporation

    A message sink (MSNK) wrapper around a stdio file.
    Include <stdio.h> before including this file.

***************************************************************************/
#ifndef MSSIO_H
#define MSSIO_H

/***************************************************************************
    Standard i/o message sink.
***************************************************************************/
typedef class MSSIO *PMSSIO;
#define MSSIO_PAR MSNK
class MSSIO : public MSSIO_PAR
{
  protected:
    bool _fError;
    FILE *_pfile;

  public:
    MSSIO(FILE *pfile);
    virtual void ReportLine(const PCSZ psz) override;
    virtual void Report(const PCSZ psz) override;
    virtual bool FError(void) override;
};

#endif //! MSSIO_H
