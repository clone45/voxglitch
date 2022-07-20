## Change Log

### 2.19.0

* For Groovebox, added ability to use scroll wheel when mousing-over a track label to quickly load the next or previous sample in the same folder.
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
