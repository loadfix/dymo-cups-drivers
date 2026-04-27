# $Id: Makefile.am 4760 2008-06-19 22:21:35Z vbuzuev $

# DYMO LabelWriter Drivers
# Copyright (C) 2008 Sanford L.P.

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

# src_v2/ is DYMO's unfinished "version 2" tree. It uses std::auto_ptr
# (removed from the standard library in C++17) and deprecated Boost
# posix_time APIs, so it does not compile on GCC >= 11 with default flags.
# The production filter binaries are built from src/; src_v2/ was never
# wired up to install anywhere. Exclude it from SUBDIRS so `make` on a
# modern toolchain succeeds.
SUBDIRS = src ppd docs samples
EXTRA_DIST = dymo-cups-drivers.spec LICENSE

#
#  $Id: Makefile.am 4760 2008-06-19 22:21:35Z vbuzuev $
#
