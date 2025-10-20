# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=

# This points to voglitch/src/
CXXFLAGS += -Isrc

# FFmpeg dependency configuration
DEP_PATH := $(abspath dep)
ffmpeg := dep/lib/libavcodec.a

# Add FFmpeg to build dependencies
DEPS += $(ffmpeg)

# Link FFmpeg libraries in correct order (order matters!)
OBJECTS += dep/lib/libavformat.a
OBJECTS += dep/lib/libavcodec.a
OBJECTS += dep/lib/libavutil.a

# Add FFmpeg include path
FLAGS += -Idep/include

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS +=

# Platform-specific linker flags for FFmpeg
ifdef ARCH_LIN
	# Recommended by FFmpeg for symbolic linking
	LDFLAGS += -Wl,-Bsymbolic
	LDFLAGS += -lpthread -lm
endif

ifdef ARCH_MAC
	# macOS SDK version flag (updated to match VCV Rack requirements)
	MAC_SDK_FLAGS = -mmacosx-version-min=10.13
	FLAGS += $(MAC_SDK_FLAGS)
	# macOS needs pthread for FFmpeg threading
	LDFLAGS += -lpthread
endif

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard src/modules/*.cpp)
SOURCES += $(wildcard src/modules/Netrunner/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)

# Dependency target for toolchain compatibility
.PHONY: dep
dep: $(ffmpeg)

# FFmpeg build target
$(ffmpeg):
	# Clone FFmpeg from official repo
	cd dep && git clone --depth 1 --branch n6.1 https://github.com/FFmpeg/FFmpeg.git ffmpeg

	# Configure FFmpeg with minimal decoder-only build
	cd dep/ffmpeg && ./configure \
		--prefix="$(DEP_PATH)" \
		$(if $(ARCH_MAC),--extra-cflags="$(MAC_SDK_FLAGS)" --extra-ldflags="$(MAC_SDK_FLAGS)",) \
		$(if $(ARCH_WIN),--cross-prefix=x86_64-w64-mingw32- --arch=x86_64 --target-os=mingw32,) \
		--enable-pic \
		--enable-gpl \
		--disable-programs \
		--disable-doc \
		--disable-avdevice \
		--disable-swresample \
		--disable-swscale \
		--disable-postproc \
		--disable-avfilter \
		--disable-network \
		--disable-iconv \
		--disable-autodetect \
		--disable-asm \
		--disable-x86asm \
		--disable-everything \
		--enable-protocol=file \
		--enable-demuxer=wav \
		--enable-demuxer=aiff \
		--enable-demuxer=flac \
		--enable-demuxer=mov \
		--enable-demuxer=mp3 \
		--enable-decoder=pcm_s16le \
		--enable-decoder=pcm_s24le \
		--enable-decoder=pcm_f32le \
		--enable-decoder=pcm_s16be \
		--enable-decoder=pcm_s24be \
		--enable-decoder=pcm_f32be \
		--enable-decoder=flac \
		--enable-decoder=alac \
		--enable-decoder=mp3float \
		--enable-parser=mpegaudio

	# Build FFmpeg (use all CPU cores)
	cd dep/ffmpeg && $(MAKE) -j$(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

	# Install to dep/ directory
	cd dep/ffmpeg && $(MAKE) install

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

# Add platform-specific system libraries after plugin.mk (required by FFmpeg)
# These must come after the Rack SDK linker flags
ifdef ARCH_WIN
	LDFLAGS := $(LDFLAGS) -static-libgcc -static-libstdc++ -lbcrypt -lws2_32
endif
