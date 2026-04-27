// -*- C++ -*-
// $Id: raster2dymolm.cpp 14880 2011-03-31 16:29:05Z aleksandr $

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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <memory>
#include <string.h>

#include "LabelManagerDriver.h"
#include "LabelManagerLanguageMonitor.h"
#include "DummyLanguageMonitor.h"
#include "CupsPrintEnvironment.h"
#include "CupsFilter.h"
#include "CupsFilterLabelManager.h"

using namespace DymoPrinterDriver;

// IsBackchannelSupported() — mirror of the same fix on the LW side.
//
// The LabelManager language monitor has the same architectural problem
// as the LabelWriter one: it injects ESC-status-request bytes into the
// main stdout stream and blocks inside cupsBackChannelRead waiting for
// replies. On modern CUPS + USB the reply path is unreliable; the
// filter can exit while pending status bytes are still in the pipe,
// leaving CUPS with a zombie "now printing" job that blocks the queue
// until cancel-a + systemctl restart cups. Fix 1 disabled this on the
// LW side and the zombie-job symptom vanished.
//
// The LM side had the same upstream value `return true;` and has
// therefore been at risk of the same failure mode. Flipping it to
// `return false;` selects CDummyLanguageMonitor, consistent with the
// LW treatment.
//
// What we lose on the LM side as a consequence:
//   * Tape-cassette-present / tape-size-mismatch / head-overheat state
//     reporting to CUPS (STATE: com.dymo.*).
//   * Auto-reprint-on-recovery when the user replaces a tape cassette
//     mid-job.
//
// The LabelManager driver's outbound commands (label-length, feed,
// raster data, cut / form-feed) are emitted by the driver class
// regardless of which language monitor is wired in, so prints still
// function; what's lost is purely the status-reporting / reprint
// loop.
//
// See src/lw/raster2dymolw.cpp for the longer rationale. Reference:
// BUGS.md finding 3.11.
static bool
IsBackchannelSupported()
{
    return false;
}

int
main(int argc, char* argv[])
{
  fputs("DEBUG: starting (raster2dymolm)\n", stderr);

  if (IsBackchannelSupported())
  {
    CCupsFilter<CLabelManagerDriver, CDriverInitializerLabelManagerWithLM, CLabelManagerLanguageMonitor> Filter;
    return Filter.Run(argc, argv);
  }
  else
  {
    CCupsFilter<CLabelManagerDriver, CDriverInitializerLabelManager, CDummyLanguageMonitor> Filter;
    return Filter.Run(argc, argv);
  }
}

/*
 * End of "$Id: raster2dymolm.cpp 14880 2011-03-31 16:29:05Z aleksandr $".
 */
