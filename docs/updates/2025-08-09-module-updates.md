# Module Updates - August 9, 2025

This document summarizes the module improvements and new features added on August 9, 2025.

## Ghosts Module

### Added: Remove Sample Context Menu Item

- **Location**: `src/modules/Ghosts/GhostsWidget.hpp`
- **Change**: Added "Remove Sample" menu item to the right-click context menu
- **Functionality**: 
  - Clears the currently loaded sample using `sample.unload()`
  - Updates filename display to show "[ EMPTY ]"
  - Purges any currently playing ghosts for clean state
- **User Impact**: Users can now easily clear loaded samples without having to load a different sample

## Looper Module  

### Added: Output Voltage Range Selector

- **Location**: `src/modules/Looper/Looper.hpp` and `src/modules/Looper/LooperWidget.hpp`
- **Changes**:
  - Added voltage range data structures with 8 selectable ranges matching Digital Sequencer XP
  - Implemented save/load functionality for range settings
  - Added context menu with "Output Range" submenu
  - Modified audio output processing to scale from -1V/+1V to selected voltage range
- **Available Ranges**:
  - 0.0 to 10.0V
  - -10.0 to 10.0V  
  - 0.0 to 5.0V
  - **-5.0 to 5.0V** (default)
  - 0.0 to 3.0V
  - -3.0 to 3.0V
  - 0.0 to 1.0V
  - -1.0 to 1.0V
- **User Impact**: Users can now select appropriate output voltage ranges for their patches, with settings persisting across patch saves/loads

## GrainEngine MK2 Module

### Updated: Sample Knob Snap Behavior

- **Location**: `src/modules/GrainEngineMK2/GrainEngineMK2.hpp`
- **Changes**:
  - Enabled `snapEnabled = true` for SAMPLE_KNOB parameter
  - Changed knob range from 0.0-1.0 to 0.0-4.0 to match 5 sample slots
  - Updated sample selection logic to work with discrete 0-4 values
  - Maintained CV input compatibility with proper scaling
- **User Impact**: Sample knob now snaps to exact positions (0, 1, 2, 3, 4) corresponding to the 5 sample slots, eliminating ambiguity when manually selecting samples

## Technical Details

### File Modifications Summary:
- `src/modules/Ghosts/GhostsWidget.hpp`: Added context menu item
- `src/modules/Looper/Looper.hpp`: Added voltage range system and scaling
- `src/modules/Looper/LooperWidget.hpp`: Added range selector menu
- `src/modules/GrainEngineMK2/GrainEngineMK2.hpp`: Enabled sample knob snapping

### Development Notes:
- All changes maintain backward compatibility with existing patches
- New features follow established UI patterns from other modules in the codebase
- Proper bounds checking and validation implemented throughout
- Save/load functionality tested for data persistence

---

*Generated on August 9, 2025*