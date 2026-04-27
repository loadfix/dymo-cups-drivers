// -*- C++ -*-
// $Id: raster2dymolw.cpp 15043 2011-05-05 17:38:38Z aleksandr $

// DYMO LabelWriter Drivers
// Copyright (C) 2008 Sanford L.P.

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include <cups/cups.h>
#include <cups/raster.h>
#include <cups/ppd.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
//#include <signal.h>
#include <memory>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "LabelWriterDriver.h"
#include "LabelWriterLanguageMonitor.h"
#include "DummyLanguageMonitor.h"
#include "CupsPrintEnvironment.h"
#include "CupsFilter.h"
#include "CupsFilterLabelWriter.h"

using namespace DymoPrinterDriver;


//#define CUPS_12 1
//
// IsBackchannelSupported()
// ------------------------
// Historically this returned `true` unconditionally (see git history of this
// file at upstream DYMO 1.4.0.5). The effect is that raster2dymolw always
// instantiates CLabelWriterLanguageMonitor, which during every EndPage() and
// on the first StartPage() calls CheckStatusAndReprint() — a loop that
// injects ESC-A status-request bytes into the print stdout stream and then
// blocks inside cupsBackChannelRead() waiting for the printer's reply.
//
// Under modern CUPS (>= 1.6) on Linux with USB DYMO LabelWriter 450 / 450
// Turbo / 450 DUO Label hardware, this back-channel reply path is unreliable.
// The read frequently times out; the filter exits but leaves pending status
// bytes in the USB backend's write pipe, and the CUPS scheduler never sees
// the job complete. Subsequent jobs stack up forever behind a zombie
// "now printing" entry.
//
// The original commented-out probe (fstat on fd 3) would have returned false
// when there is no back-channel — which is in fact how current CUPS delivers
// jobs to this filter: the back-channel is not connected to the USB device's
// status-read endpoint in a way this driver's polling semantics rely on.
//
// The safest, smallest fix is to return false so the filter selects
// CDummyLanguageMonitor instead. The dummy monitor does nothing per page; all
// printer-essential commands (reset, label length, short form feed at end of
// page, final form feed at end of document, resolution and density setup)
// are emitted by CLabelWriterDriver / CLabelWriterDriver400 regardless. What
// we lose is:
//   * State reporting to CUPS (STATE: com.dymo.out-of-paper-error et al).
//   * Automatic reprint of a label when the printer reports an error
//     mid-page. Neither is essential for a working print of a good job.
//
// For a detailed rationale, see CHANGES.md entry "Fix 1".
static bool
IsBackchannelSupported()
{
  return false;
}

int
main(int argc, char* argv[])
{
  fputs("DEBUG: starting (raster2dymolw)\n", stderr);
 
  ppd_file_t* ppd = ppdOpenFile(getenv("PPD"));
  if (!ppd)
  {
    perror("WARNING: Unable to open ppd file, use default settings - ");

    if (IsBackchannelSupported())
    {
      CCupsFilter<CLabelWriterDriver, CDriverInitializerLabelWriterWithLM, CLabelWriterLanguageMonitor> Filter;
      return Filter.Run(argc, argv);
    }
    else
    {
      CCupsFilter<CLabelWriterDriver, CDriverInitializerLabelWriter, CDummyLanguageMonitor> Filter;
      return Filter.Run(argc, argv);
    }
  }
  else
  {
    if (!strcasecmp(ppd->modelname, "DYMO LabelWriter Twin Turbo")
        || !strcasecmp(ppd->modelname, "DYMO LabelWriter 450 Twin Turbo"))
    {
      if (IsBackchannelSupported())
      {
        CCupsFilter<CLabelWriterDriverTwinTurbo, CDriverInitializerLabelWriterTwinTurboWithLM, CLabelWriterLanguageMonitor> Filter;
        return Filter.Run(argc, argv);      
      }
      else
      {
        CCupsFilter<CLabelWriterDriverTwinTurbo, CDriverInitializerLabelWriterTwinTurbo, CDummyLanguageMonitor> Filter;
        return Filter.Run(argc, argv);
      }
    }   
    else if (!strcasecmp(ppd->modelname, "DYMO LabelWriter 400")
    || !strcasecmp(ppd->modelname, "DYMO LabelWriter 400 Turbo")
    || !strcasecmp(ppd->modelname, "DYMO LabelWriter DUO Label")
    || !strcasecmp(ppd->modelname, "DYMO LabelWriter 4XL")
    || !strcasecmp(ppd->modelname, "DYMO LabelWriter 450")
    || !strcasecmp(ppd->modelname, "DYMO LabelWriter 450 Turbo")
    || !strcasecmp(ppd->modelname, "DYMO LabelWriter 450 DUO Label"))
    {
      if (IsBackchannelSupported())
      {
        CCupsFilter<CLabelWriterDriver400, CDriverInitializerLabelWriterWithLM, CLabelWriterLanguageMonitor> Filter;
        return Filter.Run(argc, argv);
      }
      else
      {
        CCupsFilter<CLabelWriterDriver400, CDriverInitializerLabelWriter, CDummyLanguageMonitor> Filter;
        return Filter.Run(argc, argv);
      }
    }
    else
    {
      if (IsBackchannelSupported())
      {
        CCupsFilter<CLabelWriterDriver, CDriverInitializerLabelWriterWithLM, CLabelWriterLanguageMonitor> Filter;
        return Filter.Run(argc, argv);
      }
      else
      {
        CCupsFilter<CLabelWriterDriver, CDriverInitializerLabelWriter, CDummyLanguageMonitor> Filter;
        return Filter.Run(argc, argv);
      }
    }    
  }
}

/*
 * End of "$Id: raster2dymolw.cpp 15043 2011-05-05 17:38:38Z aleksandr $".
 */
