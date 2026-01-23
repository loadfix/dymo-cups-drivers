# Allow GNU make extensions (wildcard function and pattern rules)
# This is the proper way to use these features in automake
AUTOMAKE_OPTIONS = -Wno-portability

# PPDC compiler
PPDC = ppdc
PPDC_FLAGS = -I . -I ./shared -I ./shared/sizes

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
# Use automake's SUFFIXES to register .ppd as a valid suffix
# The suffix rule syntax is POSIX-compliant
SUFFIXES = .drv .ppd
.drv.ppd: $(SHARED_DEF_FILES) $(SIZE_DEF_FILES)
	$(PPDC) -d . $(PPDC_FLAGS) $<

# Extra distribution files (source files)
EXTRA_DIST = $(DRV_FILES) $(SHARED_DEF_FILES) $(SIZE_DEF_FILES) Makefile

tests = testppd
check_PROGRAMS = testppd

testppd_SOURCES=

testppd$(EXEEXT): $(PPD_FILES)
	cupstestppd $(dist_cupsmodel_DATA)
