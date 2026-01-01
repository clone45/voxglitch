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

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard src/modules/*.cpp)

# Add OSC library sources (for Kaiseki module)
SOURCES += $(wildcard src/osc/*.cpp)

# Add IP common sources
SOURCES += $(wildcard src/ip/*.cpp)

# Add IP library sources (platform-specific, for Kaiseki OSC support)
# Use $(OS) for early Windows detection (ARCH_WIN is set later by plugin.mk)
ifeq ($(OS),Windows_NT)
    SOURCES += $(wildcard src/ip/win32/*.cpp)
    LDFLAGS += -lws2_32 -lwinmm
else
    SOURCES += $(wildcard src/ip/posix/*.cpp)
endif

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
