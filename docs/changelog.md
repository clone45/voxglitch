## Change Log

## 2.24.1
* Fixed issue with expander panning controls
* Various back-end improvements that won't be noticeable to most users
* Fixed the "pop" that could occur when changing memory locations
* Added feature to align all knobs to one position

## 2.24.0
* Brand new design for the Groovebox, and tons of updates

## 2.23.2
* Added defensive code in case theme layer files don't exist
* Fixed potential array index out-of-bounds errors detected by cppcheck

## 2.23.1
* Fixed link to Autobreak Studio in documentation
* Updated Groovebox so that it recalls the selected memory slot upon load
* Changed incorrect label on programmer module from "poly audio out" to "poly out"

## 2.23.0
* In Autobreak Studio, when adjusting a position slider, the position is highlighted on the waveform display
* The Groovebox track and sample selection area has been converted to an LCD screen and the ability to increment/decrement sample has been added

### 2.22.0

* Added new module: Autobreak Studio
* Fixed bug where the groovebox expander pitch controls weren't working

### 2.21.1

* Fixed bug which crashed the library view on some Windows machines
* Fixed memory leak caused when theme is missing

### 2.20.0
General:
* Code clean-up:
  - Consolidated all DSP helpers into the dsp folder instead of using submodules.hpp
  - Added namespacing to Digital Sequencer and XP version to avoid conflicts.  I may do this of all modules eventually.

Ghosts:
* Added a new "mode" knob which offers different ranges for most of the knobs (Optional - for more inventive, but less consistent controls.)
* Now conforms to the 1v/octave standard for pitch response
* Adjusted how the math is done, which is now more "correct"

Grain Engine MK2:
* Now conforms to the 1v/octave standard for pitch response

Looper Module:
* Double-clicking in the middle of the looper module replaces single-click for quickly loading samples.
* Looper module no longer has visualization of playback, but it's scheduled to return again eventually.

Groovebox:
* Stripped out declicking code from Groovebox until I can implement a proper solution

Digital Sequencer / Digital Sequencer XP
* Added menu for updating all sequencer properties (snap and output range) at once

Wavbank:
* Now conforms to the 1v/octave standard for pitch response
* Added sorting code to wavbank modules to (hopefully) solve a problem where samples were being loaded out of order for some users.

Looper:
* Replaced visualizer with a volume slider
* Updated code to use common SamplePlayer class

### 2.19.0

* For Groovebox, added ability to use `SHIFT` + `CTRL` + `scroll wheel` when mousing-over a track label to quickly load the next or previous sample in the same folder.
* Replaced rack::settings::sampleRate with APP->engine->getSampleRate() globally for better plugin and Cardinal compatibility.  (Thanks again Filipe!)
* Possibly fixed a bug where outputs would display "NAN" instead of outputting the correct values

### 2.18.0

* Fixed a bug where the right-drag-handle didn't highlight because of overlapping visualizer widgets
* Improved performance.  Should take up 1% less CPU when expander is disconnected
* Moved de-clicking filter into a shared class in /Common/dsp.  This will make it easier to update in the future.
* Added de-clicking code to 8xSampler
* Added new feature to Groovebox: Copy/Paste step data
* Updated documentation

### 2.17.1

* Fixed bug where it was not possible to shift only 1 track in groovebox
* Added de-click filter to groovebox
* Compatibility with Cardinal restored (thanks Filipe Coelho)

### 2.17.0

* Added this change log file!
* Added new parameter lock: Sample End
* Added visualizer that appears when adjust sample start or sample end positions
* Reorganized parameter lock positions on the front panel and color coded them
* Fixed a bug which could crash VCV when loading larger samples
* When loading samples, the last accessed directory should now be the default
* Added ability to shift tracks forwards and back by holding SHIFT while dragging a step button left or right
* Same as above, but when holding SHIFT+CONTROL, shifts all tracks
* Fixed issue where playback wasn't ending when unsoloing a muted track in the Groovebox Expander
* Improved how the Groovebox uses data from the expander, which avoids a lot of edge cases.
