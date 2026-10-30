# gameWE - AMX Mod X plugin
#   make build | package | compiler | clean
#
# Needs amxxpc from AMX Mod X 1.9 or newer: set AMXXPC=/path/to/amxxpc
# (amxxpc32.so must be next to it) and AMXX_INCLUDE=/path/to/include.
# "make compiler" builds both from the AMX Mod X sources into build/amxx-<AMXX>
# (needs git, gcc-multilib and g++-multilib) and uses them.
#
# The version comes from the git tag on HEAD (v1.2.3 -> 1.2.3), otherwise
# from the VERSION file. Override with: make package VERSION=1.2.3-rc1

AMXX        ?= 1.10
TOOLDIR     := build/amxx-$(AMXX)
AMXXPC      ?= $(TOOLDIR)/amxxpc
AMXX_INCLUDE ?= $(TOOLDIR)/include
GIT_TAG     := $(shell git describe --tags --exact-match --match 'v*' 2>/dev/null)
VERSION     ?= $(if $(GIT_TAG),$(patsubst v%,%,$(GIT_TAG)),$(shell cat VERSION))

MODS     := cstrike dod tfc ns hlmp
SRC      := addons/amxmodx/scripting
GEN      := build/gen
STAGE    := build/stage
PLUGINS  := $(STAGE)/addons/amxmodx/plugins
PKG      := gamewe-amxmodx-$(VERSION)
DIST     := dist

AMXXPC_ABS := $(abspath $(AMXXPC))

.PHONY: all build package compiler clean

all: build

compiler:
	sh tools/build-amxxpc.sh $(AMXX) $(abspath $(TOOLDIR))

# warnings are errors, so the code stays warning free
build:
	@test -x "$(AMXXPC)" || { echo "amxxpc not found at $(AMXXPC): run 'make compiler' or set AMXXPC=..."; exit 1; }
	rm -rf $(STAGE) $(GEN)
	install -d $(PLUGINS) $(GEN)
	printf '#if defined _gamewe_version_included\n\t#endinput\n#endif\n#define _gamewe_version_included\n#define GAMEWE_PLUGIN_VERSION "%s"\n' "$(VERSION)" > $(GEN)/gamewe_version.inc
	@set -e; for mod in $(MODS); do \
		echo "amxxpc gamewe_$$mod.sma"; \
		LD_LIBRARY_PATH="$(dir $(AMXXPC_ABS))$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH}" \
			"$(AMXXPC_ABS)" $(SRC)/gamewe_$$mod.sma -i$(GEN) -i$(SRC)/include -i$(AMXX_INCLUDE) \
			-o$(PLUGINS)/gamewe_$$mod.amxx > build/amxxpc.log 2>&1 || { cat build/amxxpc.log; exit 1; }; \
		if grep -qiE 'warning|error' build/amxxpc.log; then cat build/amxxpc.log; exit 1; fi; \
		test -s $(PLUGINS)/gamewe_$$mod.amxx; \
	done

package: build
	install -d $(DIST)
	rm -f $(DIST)/$(PKG).tar.gz $(DIST)/$(PKG).sha256 $(DIST)/$(PKG).tar.gz.sha256
	tar --sort=name --owner=0 --group=0 --numeric-owner --mtime='2000-01-01 00:00Z' \
		-C $(STAGE) -czf $(DIST)/$(PKG).tar.gz addons
	cd $(DIST) && sha256sum $(PKG).tar.gz > $(PKG).tar.gz.sha256
	@echo "built $(DIST)/$(PKG).tar.gz"

clean:
	rm -rf build $(DIST)
