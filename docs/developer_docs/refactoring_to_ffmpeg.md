# Refactoring Guide: Migrating Sample-Based Modules from AudioFile.h to FFmpeg

## Overview

This guide provides step-by-step instructions for refactoring existing Voxglitch sample-based modules to use FFmpeg-based audio loading instead of AudioFile.h. The migration enables support for additional audio formats (FLAC, MP3, ALAC) while maintaining backward compatibility.

**Supported Formats After Migration:**
- WAV (existing)
- AIFF (existing)
- FLAC (new)
- MP3 (new)
- ALAC/M4A (new)

**Modules Currently Using AudioFile.h:**
- Looper
- WavBank
- ArpSeq
- SamplerX8
- Groovebox
- Hazumi
- Repeater
- And others...

---

## Prerequisites

Before starting refactoring:

1. ✅ Ensure FFmpeg is built (should already be done via `make dep`)
2. ✅ Verify `src/modules/Netrunner/AudioLoader.hpp` and `.cpp` exist as reference
3. ✅ Create a git branch for the module you're refactoring
4. ✅ Have test audio files in various formats ready

---

## Step 1: Update sample.hpp

The core change is in `src/vgLib-2.0/sample.hpp`, specifically the `Sample::load()` method.

### 1.1 Add AudioLoader Include

**Location:** `src/vgLib-2.0/sample.hpp` (top of file)

**Before:**
```cpp
#pragma once

#include "AudioFile.h"
```

**After:**
```cpp
#pragma once

#include "AudioFile.h"
#include "../modules/Netrunner/AudioLoader.hpp"  // Add FFmpeg loader
```

### 1.2 Replace Sample::load() Method

**Location:** `src/vgLib-2.0/sample.hpp:102-164`

**Before:**
```cpp
bool load(std::string path)
{
  // Set loading flags
  this->loading = true;
  this->loaded = false;

  // initialize some transient variables
  float left = 0;
  float right = 0;

  // Load the audio file
  if(! audioFile.load(path))
  {
    this->loading = false;
    this->loaded = false;
    return(false);
  }

  // Read details about the loaded sample
  uint32_t sampleRate = audioFile.getSampleRate();
  int numSamples = audioFile.getNumSamplesPerChannel();
  int numChannels = audioFile.getNumChannels();

  this->channels = numChannels;
  this->sample_rate = sampleRate;
  sample_audio_buffer.clear();

  //
  // Copy the sample data from the AudioFile object to floating point vectors
  //
  for(int i = 0; i < numSamples; i++)
  {
    if(numChannels == 2)
    {
      left = audioFile.samples[0][i];  // [channel][sample_index]
      right = audioFile.samples[1][i];
    }
    else if(numChannels == 1)
    {
      left = audioFile.samples[0][i];
      right = left;
    }

    sample_audio_buffer.push_back(left, right);
  }

  // Store sample length and file information
  this->sample_length = sample_audio_buffer.size();
  this->filename = system::getFilename(path);
  this->display_name = filename;
  this->display_name.erase(this->display_name.length()-4); // remove extension
  this->path = path;

  this->loading = false;
  this->loaded = true;

  // Now that the audioFile has been read into memory, clear it out
  audioFile.samples[0].resize(0);
  audioFile.samples[1].resize(0);

  return(true);
};
```

**After:**
```cpp
bool load(std::string path)
{
  // Set loading flags
  this->loading = true;
  this->loaded = false;

  // Use FFmpeg AudioLoader
  vgLib_v2::AudioLoader loader;
  vgLib_v2::AudioData audioData;
  std::string error;

  bool success = loader.loadFile(path, audioData, error);

  if (!success)
  {
    WARN("Failed to load sample from %s - %s", path.c_str(), error.c_str());
    this->loading = false;
    this->loaded = false;
    return false;
  }

  // Store audio properties
  this->channels = audioData.channels;
  this->sample_rate = audioData.sampleRate;
  sample_audio_buffer.clear();

  // Convert AudioData (interleaved) to Sample format (separate L/R buffers)
  if (audioData.channels == 1)
  {
    // Mono: duplicate to both channels
    for (int64_t i = 0; i < audioData.totalSamples; i++)
    {
      float sample = audioData.samples[i];
      sample_audio_buffer.push_back(sample, sample);
    }
  }
  else if (audioData.channels == 2)
  {
    // Stereo: de-interleave
    for (int64_t i = 0; i < audioData.totalSamples; i++)
    {
      float left = audioData.samples[i * 2];
      float right = audioData.samples[i * 2 + 1];
      sample_audio_buffer.push_back(left, right);
    }
  }
  else
  {
    // Multi-channel: use first two channels
    for (int64_t i = 0; i < audioData.totalSamples; i++)
    {
      float left = audioData.samples[i * audioData.channels];
      float right = audioData.samples[i * audioData.channels + 1];
      sample_audio_buffer.push_back(left, right);
    }
  }

  // Store sample length and file information
  this->sample_length = sample_audio_buffer.size();
  this->filename = system::getFilename(path);
  this->display_name = filename;

  // Remove file extension from display name (handle various extensions)
  size_t lastDot = this->display_name.find_last_of('.');
  if (lastDot != std::string::npos)
  {
    this->display_name = this->display_name.substr(0, lastDot);
  }

  this->path = path;

  this->loading = false;
  this->loaded = true;

  return true;
};
```

### Key Changes Explained:

1. **FFmpeg Integration:** Uses `AudioLoader` instead of `AudioFile`
2. **Interleaved Format:** AudioData uses interleaved samples, so we de-interleave them
3. **Error Handling:** More explicit error reporting with FFmpeg error messages
4. **Multi-channel Support:** Handles >2 channels by taking first two
5. **Extension Removal:** Uses `find_last_of('.')` to handle all extensions (.wav, .flac, .mp3, etc.)

---

## Step 2: Update File Filter in Module UI (Optional)

Many modules have file browser dialogs that filter for `.wav` files only. Update these to accept multiple formats.

### 2.1 Locate File Dialog Code

**Common Locations:**
- `MenuItemLoadSample` classes
- Widget `onButton()` methods
- Context menu items

**Example Location:** `src/modules/Looper/LooperLoadSample.hpp` or similar

### 2.2 Update File Filters

**Before:**
```cpp
char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, NULL);
```

**After:**
```cpp
// Updated filter to accept multiple audio formats
osdialog_filters* filters = osdialog_filters_parse(
  "Audio Files:wav,aiff,aif,flac,mp3,m4a"
);
char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, filters);
osdialog_filters_free(filters);
```

### 2.3 Update Directory Scanning (For WavBank-style modules)

**Location:** Modules that scan directories for samples (e.g., `WavBank::load_samples_from_path()`)

**Before:**
```cpp
for (auto path : dirList)
{
  if (
    (rack::string::lowercase(system::getExtension(path)) == "wav") ||
    (rack::string::lowercase(system::getExtension(path)) == ".wav"))
  {
    SamplePlayer sample_player;
    sample_player.loadSample(path);
    this->sample_players.push_back(sample_player);
  }
}
```

**After:**
```cpp
// List of supported audio extensions
const std::vector<std::string> supportedExtensions = {
  "wav", ".wav",
  "aiff", ".aiff", "aif", ".aif",
  "flac", ".flac",
  "mp3", ".mp3",
  "m4a", ".m4a", "alac", ".alac"
};

for (auto path : dirList)
{
  std::string ext = rack::string::lowercase(system::getExtension(path));

  // Check if extension is in supported list
  bool isSupported = std::find(
    supportedExtensions.begin(),
    supportedExtensions.end(),
    ext
  ) != supportedExtensions.end();

  if (isSupported)
  {
    SamplePlayer sample_player;
    sample_player.loadSample(path);
    this->sample_players.push_back(sample_player);
  }
}
```

---

## Step 3: Update Comments and TODO Items

Search for comments that reference WAV-only support:

```bash
grep -r "\.wav" src/modules/YourModule/
grep -r "TODO.*MP3" src/modules/YourModule/
```

**Update comments like:**
```cpp
// TODO: Consider supporting MP3.
```

**To:**
```cpp
// Now supports WAV, AIFF, FLAC, MP3, and ALAC formats via FFmpeg
```

---

## Step 4: Testing

### 4.1 Create Test Files

Generate test files in all supported formats:

```bash
# Assuming you have ffmpeg binary installed
cd test_samples/

# Generate test tone in various formats
ffmpeg -f lavfi -i "sine=frequency=440:duration=2" test_stereo.wav
ffmpeg -f lavfi -i "sine=frequency=440:duration=2" test_stereo.aiff
ffmpeg -f lavfi -i "sine=frequency=440:duration=2" -c:a flac test_stereo.flac
ffmpeg -f lavfi -i "sine=frequency=440:duration=2" -c:a libmp3lame test_stereo.mp3
ffmpeg -f lavfi -i "sine=frequency=440:duration=2" -c:a alac test_stereo.m4a

# Mono versions
ffmpeg -f lavfi -i "sine=frequency=440:duration=2" -ac 1 test_mono.wav
ffmpeg -f lavfi -i "sine=frequency=440:duration=2" -ac 1 test_mono.flac
```

### 4.2 Test Checklist

For each module you refactor:

- [ ] **WAV files** - Verify existing patches still load correctly
- [ ] **AIFF files** - Test stereo and mono AIFF
- [ ] **FLAC files** - Test 16-bit and 24-bit FLAC
- [ ] **MP3 files** - Test various bitrates (128k, 320k)
- [ ] **ALAC files** - Test M4A containers
- [ ] **Sample rate handling** - Test 44.1kHz, 48kHz, 96kHz files
- [ ] **Pitch/playback** - Verify pitch control still works correctly
- [ ] **Looping** - Test loop functionality
- [ ] **Save/Load patches** - Confirm patches save and reload correctly
- [ ] **Error handling** - Try loading an invalid file (should fail gracefully)
- [ ] **Memory usage** - Load large files (>100MB) and monitor for leaks

### 4.3 Verify Build

```bash
make clean
make dep    # Rebuild FFmpeg if needed
make        # Build the plugin
```

### 4.4 Runtime Testing

1. Open VCV Rack
2. Add your refactored module
3. Load samples in each format
4. Check VCV log for warnings/errors
5. Verify audio output is clean (no clicks, pops, artifacts)

---

## Step 5: Optional - Add Async Loading Support

For modules that might load large files or many files at once, consider adding async loading.

### 5.1 When to Add Async Loading

**Consider async loading if:**
- Module loads many samples at once (like WavBank scanning a folder)
- Module might load large files (>50MB)
- Loading blocks the UI noticeably

**Skip async loading if:**
- Module loads one sample at a time on user action (like Looper)
- Files are typically small
- Complexity outweighs benefit

### 5.2 Example Pattern (Based on Netrunner)

See `src/modules/Netrunner/AsyncSampleLoader.hpp` for full implementation.

**Basic Pattern:**
```cpp
// In your module struct:
std::vector<AsyncSampleLoader> async_loaders;
std::vector<SamplePlayer> sample_players;

// To start loading:
for (size_t i = 0; i < paths.size(); i++)
{
  sample_players.push_back(SamplePlayer());
  async_loaders.push_back(AsyncSampleLoader());
  async_loaders[i].startLoad(paths[i], 120.0f);
}

// In process() method:
for (size_t i = 0; i < async_loaders.size(); i++)
{
  if (async_loaders[i].isReady())
  {
    std::unique_ptr<Sample> loaded_sample = async_loaders[i].getLoadedSample();
    if (loaded_sample && i < sample_players.size())
    {
      sample_players[i].applySample(std::move(loaded_sample));
    }
  }
}
```

**Note:** Async loading requires additional consideration for thread safety and UI updates. Only add if genuinely needed.

---

## Step 6: Documentation and Cleanup

### 6.1 Update Module README (if exists)

Add information about supported formats:

```markdown
## Supported Audio Formats

- WAV (PCM, 8/16/24/32-bit)
- AIFF (8/16/24/32-bit)
- FLAC (lossless)
- MP3 (lossy, all bitrates)
- ALAC/M4A (Apple lossless)
```

### 6.2 Update CHANGELOG

Add entry documenting the change:

```markdown
### Changed
- Updated audio loading to support FLAC, MP3, and ALAC formats in addition to WAV and AIFF
- Improved error handling for invalid audio files
```

### 6.3 Remove AudioFile.h Dependency (Later)

Once **all** modules are migrated:

1. Remove `#include "AudioFile.h"` from `sample.hpp`
2. Remove `AudioFile<float> audioFile;` member from `Sample` struct
3. Remove recording methods that use `audioFile` (or refactor them to use FFmpeg encoding)
4. Consider removing `src/vgLib-2.0/AudioFile.h` entirely

**Note:** Keep AudioFile.h until all modules are migrated, as some may still need it for recording/saving functionality.

---

## Common Issues and Solutions

### Issue 1: Build errors - "AudioLoader not found"

**Solution:** Ensure FFmpeg is built and AudioLoader files exist:
```bash
make dep
ls src/modules/Netrunner/AudioLoader.hpp
ls src/modules/Netrunner/AudioLoader.cpp
```

### Issue 2: Linker errors about FFmpeg symbols

**Solution:** Verify Makefile includes FFmpeg libraries:
```makefile
OBJECTS += dep/lib/libavformat.a
OBJECTS += dep/lib/libavcodec.a
OBJECTS += dep/lib/libavutil.a
```

### Issue 3: Runtime error - "Decoder not found"

**Solution:** The FFmpeg build may not include the needed decoder. Check `file_research.md` for the correct configure flags.

### Issue 4: Samples sound wrong after loading

**Solution:** Check de-interleaving logic. Ensure you're handling stereo/mono correctly:
- Mono: `audioData.samples[i]` → duplicate to L and R
- Stereo: `audioData.samples[i*2]` (left) and `audioData.samples[i*2+1]` (right)

### Issue 5: Memory leaks

**Solution:** AudioData and AudioLoader handle cleanup automatically. Ensure you're not keeping AudioData around longer than needed.

### Issue 6: File extension not recognized

**Solution:** Update file filters and extension checks to include new formats. Be case-insensitive.

---

## Verification Checklist

Before considering migration complete:

- [ ] Module builds without errors
- [ ] All supported formats load correctly
- [ ] Existing patches with WAV files still work
- [ ] Error messages are helpful (not just "failed to load")
- [ ] No memory leaks (test with large files)
- [ ] Audio quality is identical to before
- [ ] Sample rate conversion works correctly
- [ ] Pitch control works correctly
- [ ] Loop functionality works correctly
- [ ] Module appears in VCV Rack library
- [ ] Updated file filters in UI (if applicable)
- [ ] Updated comments referencing WAV-only support
- [ ] Tested on target platform (Windows/Mac/Linux if possible)

---

## Example Migration Order

Suggested order for migrating modules (easiest to hardest):

1. **Looper** - Single sample, simple interface
2. **WavBank** - Multiple samples, directory scanning
3. **SamplerX8** - 8 independent sample players
4. **ArpSeq** - Integration with sequencer
5. **Groovebox** - More complex, multiple layers
6. **Hazumi** - Granular synthesis considerations
7. **Repeater** - Real-time buffer considerations

---

## Additional Resources

- **FFmpeg Documentation:** See `docs/file_research.md` for detailed FFmpeg implementation guide
- **Reference Implementation:** `src/modules/Netrunner/` - Complete working example
- **AudioLoader API:** `src/modules/Netrunner/AudioLoader.hpp` - FFmpeg wrapper interface
- **AsyncSampleLoader:** `src/modules/Netrunner/AsyncSampleLoader.hpp` - Async loading pattern

---

## Questions?

If you encounter issues not covered in this guide:

1. Check the Netrunner implementation for reference
2. Review `file_research.md` for FFmpeg details
3. Test with simple WAV files first to isolate issues
4. Use DEBUG() logging to trace the loading process

---

**Document Version:** 1.0
**Last Updated:** 2025-01-09
**Author:** Claude Code with Bret Truchan
