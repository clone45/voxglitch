#!/bin/bash
# FFmpeg configuration for Windows build without NASM

./configure \
  --prefix="$(pwd)/.." \
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
