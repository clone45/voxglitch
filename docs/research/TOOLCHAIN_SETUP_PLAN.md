# VCV Rack Official Toolchain Setup Plan

**Date:** 2025-10-28 (Updated: 2025-10-28)
**Status:** ✅ COMPLETED - Toolchain setup successful, all platforms building
**Repository:** /mnt/c/Code/voxglitch

---

## Introduction: Why We're Doing This

Hi! If you're reading this document, you're either:
- Future me/us picking up where we left off
- Someone trying to understand what this whole toolchain thing is about
- Wondering why we can't just use GitHub Actions like we have been

Here's the story: The voxglitch plugin has been building beautifully with GitHub Actions. Every commit, every push - green checkmarks all around. Everything seemed great! But then the VCV Rack team reached out to let us know that while our GitHub Actions builds work perfectly, **the plugin fails when built with their official toolchain**.

This is actually a well-known issue in the VCV Rack community. Different compilers handle edge cases differently - things like uninitialized variables, division by zero, or floating-point quirks might work fine in one compiler but crash in another. GitHub Actions uses Apple's native compilers on actual Macs, while the official VCV toolchain cross-compiles everything from Linux using a carefully controlled set of tools. Same code, different compilers, different results.

**The challenge:** To fix this, we need to reproduce the VCV team's build environment locally. That means setting up the official Docker-based toolchain.

**The complication:** The toolchain requires a macOS SDK file that traditionally could only be extracted from Xcode on an actual Mac computer. Since we're on Windows/WSL2, we don't have a Mac readily available.

**The solution:** After researching and reaching out to the VCV community, we found a pre-packaged version of the exact SDK file we need, eliminating the need to rent an AWS Mac ($30+ for 24 hours) just to extract one file.

**What success looks like:** Once we have the official toolchain set up, we'll be able to build our plugin exactly the way VCV Library does. We can then reproduce the issue they're seeing, fix it, and validate that the fix works - all before submitting to the Library. After that, we'll continue using GitHub Actions for day-to-day development (it's convenient and free), but we'll do a final validation build with the official toolchain before any Library submission.

This document captures everything we learned during our research and provides a step-by-step plan to get the official toolchain up and running. The initial setup takes about 3 hours (mostly automated Docker builds), but it's a one-time investment that will save us from mysterious crashes and failed Library submissions down the road.

Let's do this!

---

## Problem Statement

The VCV Rack team has informed us that the voxglitch plugin builds successfully in GitHub Actions but **fails when built with the official rack-plugin-toolchain**. This is a known issue in the community - different compilers handle undefined behavior differently, causing plugins to work in one environment but crash in another.

To fix this issue, we need to:
1. Set up the official Docker-based toolchain locally
2. Build the plugin with it to reproduce the VCV team's failure
3. Fix the issues that appear
4. Continue using GitHub Actions for CI/CD, but validate with official toolchain before submissions

## Environment

- **System:** WSL2 (Linux 5.15.167.4-microsoft-standard-WSL2)
- **Platform:** Ubuntu on Windows
- **Working Directory:** /mnt/c/Code/voxglitch
- **Git Branch:** master
- **Main Branch:** master

## Research Summary

### Document 1: GitHub Actions for VCV Rack Plugins

**Key Points:**
- GitHub Actions provides a convenient build system but has compiler differences from the official toolchain
- **Critical Issue:** Plugins that build successfully in GitHub Actions can crash in official Library builds
- **Why:** GitHub Actions macOS uses Apple's native compiler on macOS 14, while the official toolchain uses osxcross with specific GCC/Clang versions cross-compiling from Linux
- **Community Consensus:** "These github builds are not 100% the same as a real toolchain build"
- **Best Practice:** Use GitHub Actions for rapid iteration, but validate with official toolchain before VCV Library submission

**Hybrid Development Approach:**
1. GitHub Actions for every commit (continuous testing)
2. Local Docker toolchain for final validation before library submission

**Technical Details:**
- Windows + Linux: Docker cross-compilation using `ghcr.io/qno/rack-plugin-toolchain-win-linux`
- macOS: GitHub Actions uses native runners with Xcode (legitimately licensed)
- Official toolchain uses osxcross to cross-compile macOS binaries from Linux

### Document 2: VCV Rack Plugin Toolchain - Complete Cross-Compilation Guide

**Overview:**
- Enables cross-compilation of VCV Rack plugins for all platforms (Linux x64, Windows x64, macOS x64, macOS ARM64) from a single Linux environment
- Uses Docker for cross-platform support
- Requires ~3.7 GB disk space after setup
- Build time: ~3 hours for initial Docker container build

**Architecture:**
- Linux builds: Native GCC compilation
- Windows builds: MinGW-w64 cross-compiler via crosstool-ng
- macOS builds: osxcross with LLVM/Clang and Apple's cctools-port

**Critical Requirements:**
- Repository path must have **no spaces** (Makefile limitation)
- Requires **MacOSX12.3.sdk.tar.xz** from Xcode 14.0.1
- Docker build needs 8 GB RAM and 15 GB disk space during construction
- Do NOT use sudo with Docker commands

**Build Commands:**
```bash
# Build Docker container (one-time, ~3 hours)
JOBS=$(nproc) make docker-build

# Build plugin
make -j$(nproc) docker-plugin-build PLUGIN_DIR=/path/to/plugin

# Output location
# Built packages placed in plugin-build/ directory
```

### Document 3: Official Toolchain Documentation

**Supported Platforms:**
- GNU/Linux x64
- Windows x64
- macOS x64 and arm64

**Official Build System:** Based on Arch Linux

**Docker Build Process:**
```bash
# Clone repo (path without spaces!)
# Place MacOSX12.3.sdk.tar.xz in repo root

# Build container
make docker-build
# or with parallel jobs:
JOBS=$(nproc) make docker-build

# Build plugin
make -j$(nproc) docker-plugin-build PLUGIN_DIR=...

# Results in plugin-build/ directory
```

**Important Notes:**
- Do NOT use sudo with Docker
- Follow Docker post-install instructions to add user to docker group
- Docker Desktop users need 8 GB RAM, 20 GB disk minimum

## The Solution: Pre-Packaged SDK

### Problem We Solved
Originally, extracting MacOSX12.3.sdk.tar.xz required:
- Access to a Mac computer
- Downloading Xcode 14.0.1 (~10 GB)
- Using osxcross tools to extract SDK
- Cost: ~$30 for AWS Mac rental (24-hour minimum)

### Solution Found
Community member discovered: https://github.com/joseluisq/macosx-sdks

This repository provides pre-packaged macOS SDKs extracted using osxcross.

**Direct Download URL:**
```
https://github.com/joseluisq/macosx-sdks/releases/download/12.3/MacOSX12.3.sdk.tar.xz
```

**SHA256 Checksum (for verification):**
```
3abd261ceb483c44295a6623fdffe5d44fc4ac2c872526576ec5ab5ad0f6e26c
```

**Legal Note:** This is technically a gray area (Apple's license prohibits SDK redistribution), but:
- Repository has been public since 2018
- Widely used in open-source community
- No documented legal actions
- Generally accepted for non-commercial/educational use

## Battle Plan: Step-by-Step Setup

### Phase 1: Prepare Environment (15 minutes)

1. **Verify Docker Installation**
   ```bash
   docker --version
   docker ps
   ```

2. **Check Docker Permissions**
   ```bash
   # Should work WITHOUT sudo
   docker run hello-world

   # If it requires sudo, fix permissions:
   sudo usermod -aG docker $USER
   # Then log out and back in
   ```

3. **Verify Disk Space**
   ```bash
   df -h
   # Need: 15 GB free during build, 3.7 GB after
   ```

4. **Check RAM**
   ```bash
   free -h
   # Need: 8 GB available
   ```

### Phase 2: Clone Toolchain Repository (5 minutes)

1. **Choose Location (NO SPACES in path)**
   ```bash
   cd /mnt/c/Code  # Or another location without spaces
   ```

2. **Clone Official Toolchain**
   ```bash
   git clone https://github.com/VCVRack/rack-plugin-toolchain.git
   cd rack-plugin-toolchain
   ```

### Phase 3: Download macOS SDK (5 minutes)

1. **Download SDK**
   ```bash
   cd rack-plugin-toolchain
   wget https://github.com/joseluisq/macosx-sdks/releases/download/12.3/MacOSX12.3.sdk.tar.xz
   ```

2. **Verify Download**
   ```bash
   sha256sum MacOSX12.3.sdk.tar.xz
   # Should match: 3abd261ceb483c44295a6623fdffe5d44fc4ac2c872526576ec5ab5ad0f6e26c
   ```

3. **Confirm File Placement**
   ```bash
   ls -lh MacOSX12.3.sdk.tar.xz
   # Should be in the rack-plugin-toolchain root directory
   ```

### Phase 4: Build Docker Toolchain Container (3 hours, mostly automated)

1. **Start Build**
   ```bash
   cd /mnt/c/Code/rack-plugin-toolchain
   JOBS=$(nproc) make docker-build
   ```

2. **What Happens:**
   - Installs Ubuntu dependencies
   - Builds crosstool-ng for Windows cross-compilation
   - Builds osxcross for macOS cross-compilation
   - Downloads Rack SDKs for all platforms
   - Takes ~3 hours total
   - Creates Docker image tagged as `rack-plugin-toolchain:19`

3. **Monitor Progress**
   - Build outputs progress to terminal
   - Can safely leave running in background
   - No interaction needed once started

4. **Verify Success**
   ```bash
   docker images | grep rack-plugin-toolchain
   # Should see: rack-plugin-toolchain:19
   ```

### Phase 5: Build voxglitch Plugin with Official Toolchain (10 minutes)

1. **Run Build**
   ```bash
   cd /mnt/c/Code/rack-plugin-toolchain
   make -j$(nproc) docker-plugin-build PLUGIN_DIR=/mnt/c/Code/voxglitch
   ```

2. **What Gets Built:**
   - Linux x64 build
   - Windows x64 build
   - macOS x64 build
   - macOS ARM64 build

3. **Check Output**
   ```bash
   ls -lh plugin-build/
   # Should see .vcvplugin files for all platforms
   ```

### Phase 6: Reproduce VCV Team's Issue (variable time)

1. **Test Locally**
   - Load built plugin in VCV Rack
   - Test functionality that VCV team reported as failing
   - Compare behavior to GitHub Actions builds

2. **Debug Issues**
   - Check build logs for warnings/errors
   - Compare compiler flags between environments
   - Look for undefined behavior patterns

3. **Common Issues to Check:**
   - Uninitialized variables
   - Division by zero
   - Modulo by zero
   - Platform-specific assumptions
   - Floating-point precision differences

### Phase 7: Fix and Validate (variable time)

1. **Fix Issues in Source Code**
   - Address any undefined behavior
   - Add defensive checks
   - Ensure cross-platform compatibility

2. **Rebuild with Official Toolchain**
   ```bash
   make -j$(nproc) docker-plugin-build PLUGIN_DIR=/mnt/c/Code/voxglitch
   ```

3. **Test Again**
   - Verify fixes work in official toolchain builds
   - Ensure GitHub Actions still builds successfully

4. **Submit to VCV Library**
   - Once validated with official toolchain
   - Should now match VCV team's build environment

## Issue Discovered & Resolved: FFmpeg Cross-Compilation

During the initial build with the official toolchain, we discovered that the voxglitch plugin's FFmpeg dependency failed to build for macOS. This was **NOT** the original issue the VCV team reported, but rather a toolchain compatibility problem specific to the voxglitch plugin's build system.

### The Problem

**Error encountered:**
```
gcc: error: unrecognized command-line option '-mmacosx-version-min=10.9'
C compiler test failed.
```

**Root cause:**
The voxglitch Makefile was passing macOS-specific compiler flags (`-mmacosx-version-min=10.9`) to FFmpeg's configure script, but FFmpeg was attempting to use the regular Linux `gcc` compiler instead of the osxcross cross-compiler (`x86_64-apple-darwin21.4-clang`).

The Makefile had logic to set `MAC_SDK_FLAGS` when building for macOS, but it wasn't detecting the cross-compilation environment and therefore wasn't telling FFmpeg to use the correct cross-compiler.

### The Solution

Modified `/mnt/c/Code/voxglitch/Makefile` (lines 59-76) to detect osxcross cross-compilation and properly configure FFmpeg:

```makefile
# Detect if we're cross-compiling for macOS (osxcross)
ifneq ($(findstring darwin,$(CC)),)
    FFMPEG_CROSS_COMPILE := yes
    FFMPEG_CC := $(CC)
    FFMPEG_CXX := $(CXX)
    FFMPEG_AR := $(AR)
    FFMPEG_RANLIB := $(RANLIB)
endif

# FFmpeg build target
$(ffmpeg):
	# Clone FFmpeg from official repo
	cd dep && git clone --depth 1 --branch n6.1 https://github.com/FFmpeg/FFmpeg.git ffmpeg

	# Configure FFmpeg with minimal decoder-only build
	cd dep/ffmpeg && ./configure \
		--prefix="$(DEP_PATH)" \
		$(if $(FFMPEG_CROSS_COMPILE),--enable-cross-compile --cc="$(FFMPEG_CC)" --cxx="$(FFMPEG_CXX)" --ar="$(FFMPEG_AR)" --ranlib="$(FFMPEG_RANLIB)" --target-os=darwin --arch=x86_64 --extra-cflags="$(MAC_SDK_FLAGS)" --extra-ldflags="$(MAC_SDK_FLAGS)",) \
		$(if $(ARCH_MAC),$(if $(FFMPEG_CROSS_COMPILE),,--extra-cflags="$(MAC_SDK_FLAGS)" --extra-ldflags="$(MAC_SDK_FLAGS)"),) \
		$(if $(ARCH_WIN),--cross-prefix=x86_64-w64-mingw32- --arch=x86_64 --target-os=mingw32,) \
		--enable-pic \
		--enable-gpl \
		[... rest of FFmpeg configuration flags ...]
```

**Key changes:**
1. Detection logic checks if `CC` environment variable contains "darwin" to identify osxcross cross-compilation
2. When cross-compiling is detected, captures the cross-compiler toolchain variables
3. Passes `--enable-cross-compile` with the correct compilers to FFmpeg's configure:
   - `--cc="x86_64-apple-darwin21.4-clang"`
   - `--cxx="x86_64-apple-darwin21.4-clang++-libc++"`
   - `--target-os=darwin --arch=x86_64`
4. Maintains backwards compatibility with native macOS builds (GitHub Actions)

### Verification

After applying the fix, FFmpeg successfully configured with the osxcross compiler:

```
C compiler                x86_64-apple-darwin21.4-clang
target-os                 darwin
arch                      x86_64
```

All four platform builds completed successfully:
- ✅ **voxglitch-2.36.2-mac-x64.vcvplugin** (942 KB)
- ✅ **voxglitch-2.36.2-mac-arm64.vcvplugin** (889 KB)
- ✅ **voxglitch-2.36.2-win-x64.vcvplugin** (1.6 MB)
- ✅ **voxglitch-2.36.2-lin-x64.vcvplugin** (1.5 MB)

### Lessons Learned

1. **Cross-compilation detection is critical**: Build systems must detect when they're being cross-compiled and adjust accordingly
2. **FFmpeg needs explicit cross-compiler configuration**: FFmpeg's configure script requires `--enable-cross-compile` and explicit compiler paths
3. **Environment variables are key**: The official toolchain sets `CC`, `CXX`, `AR`, `RANLIB` to the appropriate cross-compilers
4. **Test early with clean builds**: Always do a clean build (`rm -rf build dist dep`) when testing toolchain changes

### Impact

This fix ensures that:
- The voxglitch plugin builds successfully with the official VCV Rack toolchain
- Future builds will work seamlessly across all platforms
- The build process is compatible with both GitHub Actions (native macOS) and official toolchain (cross-compiled macOS)

## Ongoing Workflow (Post-Setup)

### For Development:
- Continue using GitHub Actions for rapid iteration
- Every commit gets built and tested automatically

### Before Library Submission:
```bash
cd /mnt/c/Code/rack-plugin-toolchain
make -j$(nproc) docker-plugin-build PLUGIN_DIR=/mnt/c/Code/voxglitch
# Test the resulting builds thoroughly
```

### Updating Rack SDKs (without rebuilding toolchain):
```bash
cd /mnt/c/Code/rack-plugin-toolchain
make rack-sdk-clean
make rack-sdk-all
```

## Key Community Contacts

If additional help needed:
- **qno** - Maintains `qno/vcv-plugin-github-actions-example` and Docker images
- **@cschol** - Acknowledged in official docs for toolchain help
- **baconpaul** - Surge XT modules developer
- **Squinky** - Documented toolchain build differences

## Reference Links

- Official Toolchain: https://github.com/VCVRack/rack-plugin-toolchain
- macOS SDKs Repository: https://github.com/joseluisq/macosx-sdks
- GitHub Actions Example: https://github.com/qno/vcv-plugin-github-actions-example
- VCV Rack Library: https://library.vcvrack.com
- VCV Community Forum: https://community.vcvrack.com

## Current Status

- ✅ Problem identified and understood
- ✅ Research completed
- ✅ macOS SDK download URL located
- ✅ Environment verified (WSL2 on Windows)
- ✅ **Phase 1:** Environment prepared (Docker installed and configured)
- ✅ **Phase 2:** Toolchain repository cloned
- ✅ **Phase 3:** macOS SDK downloaded and verified
- ✅ **Phase 4:** Docker toolchain container built (rack-plugin-toolchain:19)
- ✅ **Phase 5:** Initial build attempted - FFmpeg cross-compilation issue discovered
- ✅ **FFmpeg Fix:** Makefile updated to detect and configure osxcross cross-compilation
- ✅ **Phase 5 (Retry):** All four platform builds completed successfully
- ✅ **Build Outputs:** All .vcvplugin files created for all platforms
- 🎯 **READY:** Toolchain is ready for VCV Library submissions

**Build Results (2025-10-28):**
- voxglitch-2.36.2-mac-x64.vcvplugin (942 KB)
- voxglitch-2.36.2-mac-arm64.vcvplugin (889 KB)
- voxglitch-2.36.2-win-x64.vcvplugin (1.6 MB)
- voxglitch-2.36.2-lin-x64.vcvplugin (1.5 MB)

## Troubleshooting Tips for Future Builds

### Common Issues and Solutions

1. **"multiple target patterns" error**
   - **Cause:** Old build dependency files from previous builds
   - **Solution:** Clean build artifacts: `cd /mnt/c/Code/voxglitch && rm -rf build dist dep`

2. **FFmpeg "C compiler test failed" error**
   - **Cause:** FFmpeg not configured for cross-compilation
   - **Solution:** Already fixed in Makefile (lines 59-76)
   - **Verification:** Check that `CC` contains "darwin" during macOS builds

3. **Docker TTY error ("input device is not a TTY")**
   - **Cause:** Makefile uses `--interactive --tty` flags
   - **Solution:** Run Docker command directly without TTY flags, or use background mode

4. **Missing .vcvplugin files after build**
   - **Cause:** Build failed but error was missed
   - **Solution:** Check `/mnt/c/Code/voxglitch/toolchain-build.log` for errors
   - **Verification:** Ensure exit code is 0

5. **Out of disk space during Docker build**
   - **Cause:** Docker needs 15 GB during build
   - **Solution:** Clean Docker images: `docker system prune -a`
   - **Check:** `df -h` before starting build

### Build Performance Tips

1. **Use parallel jobs:** Always use `-j$(nproc)` for faster builds
2. **Background builds:** Redirect output to log file for long builds: `2>&1 | tee build.log`
3. **Docker layer caching:** Keep the toolchain Docker image to avoid 3-hour rebuilds
4. **Clean selectively:** Only clean the plugin (`make clean`), not deps unless necessary

## Notes for Future Sessions

- This document created: 2025-10-28
- Last updated: 2025-10-28
- Repository path: /mnt/c/Code/voxglitch
- Working on WSL2/Ubuntu
- Toolchain: rack-plugin-toolchain:19
- Build artifacts: /mnt/c/Code/rack-plugin-toolchain/plugin-build/
- Build log: /mnt/c/Code/voxglitch/toolchain-build.log

**Achievement unlocked:** The voxglitch plugin now builds successfully with the official VCV Rack toolchain on all platforms. The FFmpeg cross-compilation issue has been resolved, and the build system is fully compatible with both GitHub Actions (native macOS) and the official toolchain (cross-compiled macOS).
