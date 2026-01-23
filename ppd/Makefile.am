# $Id: Makefile.am 15958 2011-09-02 14:37:33Z pineichen $

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

# PPDC compiler
PPDC = ppdc
PPDC_FLAGS = -I. -I./shared -I./shared/sizes

# Source files for PPD generation
DRV_FILES = $(wildcard *.drv)
SHARED_DEF_FILES = $(wildcard shared/*.def)
SIZE_DEF_FILES = $(wildcard shared/sizes/*.def)

# Generated PPD files
PPD_FILES = \
    lm400.ppd \
    lm450.ppd \
    lmpc.ppd \
    lmpc2.ppd \
    lmpnp.ppd \
    lmwpnp.ppd \
    lp350.ppd \
    lw300.ppd \
    lw310.ppd \
    lw315.ppd \
    lw320.ppd \
    lw330.ppd \
    lw330t.ppd \
    lw400.ppd \
    lw400t.ppd \
    lwduol.ppd \
    lwduot.ppd \
    lwduot2.ppd \
    lwtt.ppd \
    lw4xl.ppd \
    lw450.ppd \
    lw450t.ppd \
    lw450tt.ppd \
    lw450dl.ppd \
    lw450dt.ppd \
    se450.ppd \
    lmmls.ppd \
    lw550.ppd \
    lw550p.ppd \
    lw550t.ppd \
    lw550tp.ppd \
    lw5xl.ppd \
    lw5xlp.ppd \
    lww.ppd

dist_cupsmodel_DATA = $(PPD_FILES)

# Pattern rule: compile .drv to .ppd
%.ppd: %.drv $(SHARED_DEF_FILES) $(SIZE_DEF_FILES)
	$(PPDC) $(PPDC_FLAGS) $<

# Extra distribution files (source files)
EXTRA_DIST = $(DRV_FILES) $(SHARED_DEF_FILES) $(SIZE_DEF_FILES) Makefile

tests = testppd
check_PROGRAMS = testppd

testppd_SOURCES=

testppd$(EXEEXT): $(PPD_FILES)
	cupstestppd $(dist_cupsmodel_DATA)
