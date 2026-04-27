// -*- C++ -*-
// $Id: CupsFilterLabelWriter.h 7049 2009-02-06 23:24:54Z vbuzuev $

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

#ifndef hd8574b83_b264_47b2_8d33_a46ae75691d2
#define hd8574b83_b264_47b2_8d33_a46ae75691d2

#include <cups/cups.h>
#include <cups/raster.h>
#include <cups/ppd.h>
#include "LabelWriterDriver.h"
#include "LabelWriterLanguageMonitor.h"
#include "DummyLanguageMonitor.h"
#include "PrinterDriver.h"  // ILanguageMonitor

namespace DymoPrinterDriver
{

// The "no-LM" Process*Options variants take the language monitor by
// ILanguageMonitor& rather than CDummyLanguageMonitor&.
//
// Upstream declared them to take CDummyLanguageMonitor& but left the
// parameter unused in the body. The WithLM subclasses then invoked the
// no-LM versions with `(CDummyLanguageMonitor&)LM` C-casts — where LM
// is actually a CLabelWriterLanguageMonitor (a sibling class, not a
// derived class). That cast is reinterpret_cast and the reference is
// undefined per [basic.reference]; any use of the parameter inside the
// callee would be UB. It happens to be safe today only because the
// callees don't touch LM.
//
// Hoisting the parameter type to the common base ILanguageMonitor&
// removes the UB cleanly: the WithLM callers pass their concrete LM
// reference (which IS-A ILanguageMonitor) with no cast, and the no-LM
// callees still don't use it. See STATIC_ANALYSIS.md §S-1.
class CDriverInitializerLabelWriter
{
public:
  static void ProcessPPDOptions (CLabelWriterDriver& Driver, ILanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(CLabelWriterDriver& Driver, ILanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

class CDriverInitializerLabelWriterTwinTurbo
{
public:
  static void ProcessPPDOptions (CLabelWriterDriverTwinTurbo& Driver, ILanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(CLabelWriterDriverTwinTurbo& Driver, ILanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

class CDriverInitializerLabelWriterWithLM
{
public:
  static void ProcessPPDOptions (CLabelWriterDriver& Driver, CLabelWriterLanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(CLabelWriterDriver& Driver, CLabelWriterLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};


class CDriverInitializerLabelWriterTwinTurboWithLM
{
public:
  static void ProcessPPDOptions (CLabelWriterDriverTwinTurbo& Driver, CLabelWriterLanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(CLabelWriterDriverTwinTurbo& Driver, CLabelWriterLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

}

#endif

/*
 * End of "$Id: CupsFilterLabelWriter.h 7049 2009-02-06 23:24:54Z vbuzuev $".
 */
