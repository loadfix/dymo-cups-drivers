
AUTOMAKE_OPTIONS = foreign

SUBDIRS = src ppd docs samples
EXTRA_DIST = dymo-cups-drivers.spec LICENSE README.md CHANGELOG

.PHONY: format
format:
	find src -name "*.cpp" -o -name "*.h" | xargs clang-format -i

.PHONY: check-format
check-format:
	find src -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror

