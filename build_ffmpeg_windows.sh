#!/bin/bash
# Build FFmpeg for Windows (run this in MSYS2/MinGW terminal)

set -e

echo "Building FFmpeg for Windows..."

# Set paths
DEP_PATH="$(pwd)/dep"
FFMPEG_SRC="${DEP_PATH}/ffmpeg"

# Clean previous build if exists
if [ -d "$FFMPEG_SRC" ]; then
    echo "Removing existing FFmpeg source..."
    rm -rf "$FFMPEG_SRC"
fi

if [ -d "${DEP_PATH}/lib" ]; then
    echo "Removing existing FFmpeg libraries..."
    rm -rf "${DEP_PATH}/lib"
    rm -rf "${DEP_PATH}/include/libav*"
fi

# Create dep directory if needed
mkdir -p "${DEP_PATH}"

# Clone FFmpeg
echo "Cloning FFmpeg..."
cd "${DEP_PATH}"
git clone --depth 1 --branch n6.1 https://github.com/FFmpeg/FFmpeg.git ffmpeg

# Configure FFmpeg
echo "Configuring FFmpeg..."
cd ffmpeg
./configure \
  --prefix="${DEP_PATH}" \
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

# Build FFmpeg
echo "Building FFmpeg (this will take 5-10 minutes)..."
make -j$(nproc)

# Install FFmpeg
echo "Installing FFmpeg libraries..."
make install

echo ""
echo "✅ FFmpeg build complete!"
echo ""
echo "Libraries installed:"
ls -lh "${DEP_PATH}/lib/libav*.a"
echo ""
echo "Now run: make clean && make install -j10"
