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

# Include Rack SDK's arch.mk early so we can use ARCH_WIN / ARCH_LIN / ARCH_MAC
# for platform-specific source selection (arch.mk is safe to include before plugin.mk)
include $(RACK_DIR)/arch.mk

# Kaiseki opens its own UDP socket, so Windows still needs the winsock libs.
ifdef ARCH_WIN
    LDFLAGS += -lws2_32 -lwinmm
endif

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
