# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=

# This points to voglitch/src/
CXXFLAGS += -Isrc

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS +=

# The module-browser smoke test build (scripts/browser_smoke.py): compiles the
# hidden BrowserSmoke module in and instruments the plugin with UBSan, which
# reports `&module->x` on the browser's NULL module at its exact line on any
# compiler -- the ArpSeq crash of 2026-10-30 only showed on Xcode's clang.
# vptr is off: Rack's own classes are not instrumented.
ifdef BROWSER_SMOKE
FLAGS += -DVOXGLITCH_BROWSER_SMOKE -fsanitize=undefined -fno-sanitize=vptr
LDFLAGS += -fsanitize=undefined
endif

# The Wav Bank MC load-race test build: adds a context-menu item that reloads
# the current folder repeatedly while audio plays, to reproduce the
# load-while-playing crash on demand (WavBankMCWidget.hpp). Never in a shipped
# build. Make does not notice the flag changing, so touch the source first:
#   touch src/modules/wavbank_mc.cpp && make WAVBANK_RACE_TEST=1 install
ifdef WAVBANK_RACE_TEST
FLAGS += -DVOXGLITCH_WAVBANK_RACE_TEST
endif

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard src/modules/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
