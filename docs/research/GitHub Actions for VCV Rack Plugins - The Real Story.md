# GitHub Actions for VCV Rack Plugins: The Real Story

GitHub Actions **does provide a genuine shortcut** that sidesteps most toolchain complexity—but with important caveats about build consistency. The community has converged on a hybrid Docker + native macOS approach that works remarkably well, though it's not quite as simple as "just use the cloud."

## Does GitHub Actions eliminate the macOS SDK requirement?

**Yes—sort of.** This requires understanding what "the SDK" means in two different contexts:

**The macOS System SDK (what you DON'T need):**
GitHub's macOS runners come with **complete Xcode installations pre-installed**, including all Apple platform SDKs (macOS 14.0-15.2, iOS, tvOS, watchOS). You get multiple Xcode versions (14.3.1 through 16.4) with full compilation toolchains ready to go. **No MacOSX12.3.sdk.tar.xz download needed** because Apple's native SDK is already there.

**The VCV Rack SDK (what you DO need):**
Your workflow still downloads the **Rack SDK** (the VCV-specific headers and libraries) directly from vcvrack.com during each build. This is a quick wget/curl operation (~50MB), not the problematic multi-gigabyte Xcode SDK extraction that plagues local Docker setup.

The critical distinction: The **macOS system SDK licensing problem only affects the Docker cross-compilation toolchain**. When using GitHub's native macOS runners, you're compiling on actual macOS machines with legitimately licensed Xcode, so Apple's restrictions don't apply.

## Setup process: dramatically simpler

**GitHub Actions setup takes 15-30 minutes vs 3+ hours for local Docker:**

The standard workflow from qno's community example (the de facto reference):

```yaml
name: Build VCV Rack Plugin
on: [push, pull_request]

jobs:
  # Windows + Linux: Docker-based
  build:
    runs-on: ubuntu-latest
    container:
      image: ghcr.io/qno/rack-plugin-toolchain-win-linux
    strategy:
      matrix:
        platform: [win-x64, lin-x64]
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - name: Build plugin
        run: |
          export PLUGIN_DIR=$GITHUB_WORKSPACE
          pushd /home/build/rack-plugin-toolchain
          make plugin-build-${{ matrix.platform }}
      - uses: actions/upload-artifact@v4
        with:
          path: /home/build/rack-plugin-toolchain/plugin-build
          name: ${{ matrix.platform }}

  # macOS: Native compilation
  build-mac:
    runs-on: macos-latest
    strategy:
      matrix:
        platform: [x64, arm64]
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - name: Get Rack-SDK
        run: |
          wget -O Rack-SDK.zip https://vcvrack.com/downloads/Rack-SDK-latest-mac-x64+arm64.zip
          unzip Rack-SDK.zip
      - name: Build plugin
        run: |
          export RACK_DIR=$HOME/Rack-SDK
          export CROSS_COMPILE=x86_64-apple-darwin  # or arm64-apple-darwin
          make dep
          make dist
      - uses: actions/upload-artifact@v4
        with:
          path: dist/*.vcvplugin
          name: mac-${{ matrix.platform }}
```

**Complexity comparison:**
- **Local Docker**: Extract macOS SDK from Xcode → package with osxcross → build toolchain Docker image (3+ hours) → configure paths → run builds
- **GitHub Actions**: Copy workflow YAML → commit → push → builds start automatically

The workflow handles version management, artifact storage, and release creation automatically. Tag your repository with `v1.0.0` and it builds all four platform variants and creates a GitHub release with downloadable `.vcvplugin` files.

## How builds work under the hood

**Hybrid architecture** solving Apple's licensing constraints:

### Windows + Linux: Docker cross-compilation
Runs on `ubuntu-latest` with the `ghcr.io/qno/rack-plugin-toolchain-win-linux` container, which contains:
- The official VCV rack-plugin-toolchain (based on VCV's Arch Linux build system)
- MinGW-w64 cross-compiler for Windows
- Native GCC for Linux
- Pre-configured build environments at `/home/build/rack-plugin-toolchain`

This matches the **exact toolchain VCV uses for Library builds**, ensuring consistency for Windows/Linux.

### macOS: Native runners with Xcode
Runs on actual macOS VMs (`macos-latest` = macOS 14) with:
- **Pre-installed Xcode** (14.3.1, 15.x, 16.x available)
- **All macOS SDKs** already present in `/Applications/Xcode.app/Contents/Developer/Platforms/`
- Apple Clang compiler, Swift toolchain, build tools
- Downloads only the **Rack SDK** from vcvrack.com
- Cross-compiles both x64 and ARM64 from the same runner using `CROSS_COMPILE` environment variable

**Why this split approach?** As community maintainer qno explained: "I was made aware of legal issues for the usage of the MacOS SDK" in publicly distributed Docker images. Apple's Xcode license prohibits redistributing the SDK or using it on non-Apple hardware. Docker toolchains work around this for Windows/Linux cross-compilation, but for macOS you need either:
1. Access to a Mac to extract the SDK (painful)
2. Use of GitHub's legitimately licensed macOS infrastructure (elegant)

The community chose option 2.

## Trade-offs: the complete picture

### Build time
**GitHub Actions**: 5-20 minutes per build (parallel across platforms)
- Cold start overhead each time
- Network latency for Docker pulls
- But completely automated

**Local Docker**: 2-10 minutes per build after initial setup
- Benefits from persistent Docker layer cache
- Local storage access faster
- But 3+ hour initial toolchain build

**Winner for iteration speed: Local Docker** (after setup)  
**Winner for getting started: GitHub Actions** (by miles)

### Reliability: the critical gotcha

**This is where it gets interesting.** Multiple developers report plugins that build successfully in GitHub Actions but **crash when built by VCV Library**. 

Community member Squinky documented a real case: "I was finding that my own build worked fine, but sometimes the library builds would crash. The only way to replicate the crash was to use the rack toolchain." The root cause was undefined behavior (modulo by zero) that manifested differently in different compiler environments.

**The issue**: 
- **macOS in GitHub Actions**: Uses Apple's native compiler on macOS 14
- **macOS in official toolchain**: Uses osxcross with specific GCC version cross-compiling from Linux
- **Different compilers handle undefined behavior differently**

Community consensus from the 79+ post discussion thread: *"These github builds are not 100% the same as a real toolchain build."*

**Practical impact**: GitHub Actions is excellent for rapid iteration and testing, but you should **validate with the official toolchain before VCV Library submission**. The hybrid development approach most professionals use:
1. GitHub Actions for every commit (continuous testing)
2. Local Docker toolchain for final validation before library submission

### Cost (private repositories)

**Public repos: Completely free** with unlimited builds

**Private repos:**
- 2,000 free minutes/month (Linux only)
- Platform multipliers:
  - Linux: 1x ($0.008/min)
  - Windows: 2x ($0.016/min)
  - **macOS: 10x ($0.08/min)**

**Real-world math:**
- 20 builds/month, 10 minutes each = 200 minutes = **$0** (within free tier)
- 100 macOS builds/month, 10 minutes each = 10,000 billed minutes = **~$800/month**

The **macOS multiplier is brutal** for heavy private repo usage. But most developers stay within free tiers by:
- Using Linux for Windows/Linux builds (cheaper)
- Only running macOS builds on important commits
- Keeping builds optimized and cached

### Debugging: local wins decisively

**GitHub Actions debugging: frustrating**
- Debug via logs only
- 5-20 minute feedback cycles
- No interactive access
- "Debugging via CI" universally hated

**Local Docker debugging: excellent**
- Full GDB/LLDB access
- IDE integration (VSCode, CLion)
- Real-time variable inspection
- Immediate feedback
- Can shell into container for investigation

If you're troubleshooting a complex build issue, the **10-100x faster feedback loop** of local debugging makes it dramatically superior.

### Compatibility with VCV Library requirements

VCV Library doesn't mandate a specific build method—they build plugins themselves. The submission process:
1. Developer creates GitHub issue with plugin slug
2. Provides repository URL, version, and exact commit hash
3. **VCV builds using official rack-plugin-toolchain**
4. VCV publishes if build succeeds

**Critical**: Your plugin must build successfully with the official toolchain. GitHub Actions gets you 90% there, but **the compiler differences mean you should test with official toolchain before submission** to avoid discovering crashes after users download from the Library.

## Community adoption and real-world examples

**qno/vcv-plugin-github-actions-example** is the **de facto standard**, referenced in virtually every discussion:
- Maintained through 8+ major updates tracking Rack SDK evolution
- Complete, copy-paste-ready workflow
- Handles version management, artifact storage, release creation
- ~200 lines of well-commented YAML

**Actively used by:**
- **Surge XT modules** (baconpaul): "His applying [GitHub Actions] to Surge XT this summer was the impetus which led to us finishing the modules!"
- **stoermelder PackOne** (50+ modules, very active development)
- **DaveBenham/VenomModules** (587+ workflow runs)
- **Paul-Dempsey/GenericBlank** (template repository with GitHub Actions built-in)
- **Dewb/monome-rack** (complex firmware emulation with custom Docker build)

**Community thread**: 79+ posts on "Automated building and releasing Plugins on Github with Github Actions" with ongoing troubleshooting, updates, and success stories.

**Common workflow:**
1. Fork qno's example
2. Update plugin.json with your details
3. Push to GitHub
4. Tag with `v1.0.0` to trigger release build
5. Get .vcvplugin files for all platforms in GitHub Releases

## The macOS compilation question answered

**Does GitHub Actions use native macOS compilation?** 

**Yes, completely native.** The macOS portion runs on actual macOS virtual machines with:
- Real Intel or Apple M1 hardware (you choose via `macos-13`, `macos-14`, `macos-latest`)
- Full Xcode installation (multiple versions, switch with `xcode-select`)
- **All Apple platform SDKs pre-installed** in standard locations
- Apple Clang compiler as default
- 14 GB RAM, 14 GB storage
- 4 cores (Intel) or 3 cores (M1)

**The only download** is the Rack SDK (~50MB) from vcvrack.com—a simple wget operation that takes seconds.

**Cross-compilation details:**
From the same macOS runner, you build both x64 and ARM64 variants using the `CROSS_COMPILE` environment variable:
- `x86_64-apple-darwin` for Intel Macs
- `arm64-apple-darwin` for Apple Silicon

This works because the Rack SDK (as of version 2.2.1+) includes proper cross-compilation support. Earlier versions required community patches, but it's now built into the official SDK.

**Comparison to Docker approach:**
- **Docker toolchain**: Cross-compiles macOS binaries from Linux using osxcross, requires MacOSX12.3.sdk.tar.xz extracted from Xcode (painful)
- **GitHub Actions**: Native compilation on macOS with pre-installed Xcode (trivial)

## Final verdict: is it truly a shortcut?

**Yes, but with asterisks.**

GitHub Actions **genuinely eliminates**:
- ✅ The macOS SDK extraction nightmare (biggest pain point)
- ✅ 3+ hour toolchain build time
- ✅ 8GB RAM and 15GB disk space requirements
- ✅ Docker configuration complexity
- ✅ Manual multi-platform build orchestration
- ✅ Platform-specific development machine requirements

GitHub Actions **does not eliminate**:
- ❌ The need to understand your build system
- ❌ Compiler differences from official Library builds (use Docker toolchain for validation)
- ❌ Debugging complexity (logs only, slow feedback)
- ❌ Costs for heavy private repo usage

**The complexity is moved, not eliminated.** But it's moved in a very favorable direction:
- From "3-hour Docker setup requiring Mac access for SDK"
- To "copy 200 lines of YAML and push to GitHub"

**Recommended approach for most developers:**

**Development phase**: Use GitHub Actions exclusively
- Zero setup time to start building plugins
- Automatic builds on every commit
- All platforms tested continuously
- Free for open source

**Pre-submission validation**: Build locally with official Docker toolchain
- Catches compiler-specific issues
- Validates exact Library build conditions
- Superior debugging when issues arise
- Confidence before submission

**Best of both worlds**: Fast iteration with GitHub Actions, rigorous validation with local toolchain, automated releases and team collaboration through CI/CD. The community has figured out a workflow that genuinely works, and qno's example repository makes it accessible to everyone.

The macOS SDK problem—historically the biggest barrier—is completely solved by GitHub's native macOS runners. For open-source VCV Rack plugin development, GitHub Actions is transformative.