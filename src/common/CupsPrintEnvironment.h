// -*- C++ -*-
// $Id: CupsPrintEnvironment.h 14901 2011-04-06 10:46:22Z aleksandr $

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

#ifndef h952b1c81_8931_433a_8479_7ae6d8e85a86
#define h952b1c81_8931_433a_8479_7ae6d8e85a86

#include <cups/backend.h>
#include <cups/sidechannel.h>
#include "PrinterDriver.h"

namespace DymoPrinterDriver
{

// this is environment for a driver
// this env will be forward output to LM also
class CCupsPrintEnvironmentForDriver: public IPrintEnvironment
{
public:
  CCupsPrintEnvironmentForDriver(ILanguageMonitor& LanguageMonitor);
  virtual ~CCupsPrintEnvironmentForDriver();

  // Rule of Three: this class owns a raw FILE* and closes it in its
  // destructor. If a copy were ever made (by accident — e.g. a caller
  // writes `auto env = originalEnv;`) the compiler-generated copy
  // constructor would shallow-copy PRNFile_ and the destructor of the
  // second instance would fclose() an already-closed stream. Delete
  // both the copy ctor and the copy-assignment operator so such
  // accidents produce a compile error instead.
  //
  // A move-based alternative (e.g. std::unique_ptr<FILE, decltype(&fclose)>)
  // would be cleaner still but changes more of the surface area than
  // warranted. See STATIC_ANALYSIS.md §S-5.
  CCupsPrintEnvironmentForDriver(const CCupsPrintEnvironmentForDriver&) = delete;
  CCupsPrintEnvironmentForDriver& operator=(const CCupsPrintEnvironmentForDriver&) = delete;

  virtual void WriteData(const buffer_t& DataBuffer);
  virtual void ReadData(buffer_t& DataBuffer);
  virtual job_status_t GetJobStatus();
  virtual void SetJobStatus(job_status_t JobStatus);

private:
  FILE* PRNFile_;
  ILanguageMonitor& LanguageMonitor_;
};

// this is environment for a language monitor
// it simple output it is data to CUPS file descriptor
class CCupsPrintEnvironmentForLM: public IPrintEnvironment
{
public:
  CCupsPrintEnvironmentForLM();
  virtual ~CCupsPrintEnvironmentForLM();

  // See the Rule-of-Three comment on CCupsPrintEnvironmentForDriver.
  // This variant never opens PRNFile_ in the current code, but it
  // holds the handle as a member and a future edit could. Being
  // non-copyable is the right default for a resource-owning type.
  CCupsPrintEnvironmentForLM(const CCupsPrintEnvironmentForLM&) = delete;
  CCupsPrintEnvironmentForLM& operator=(const CCupsPrintEnvironmentForLM&) = delete;

  virtual void WriteData(const buffer_t& DataBuffer);
  virtual void ReadData(buffer_t& DataBuffer);
  virtual job_status_t GetJobStatus();
  virtual void SetJobStatus(job_status_t JobStatus);

private:
  FILE* PRNFile_;
  IPrintEnvironment::job_status_t JobStatus_;
};

};

#endif

/*
 * End of "$Id: CupsPrintEnvironment.h 14901 2011-04-06 10:46:22Z aleksandr $".
 */
