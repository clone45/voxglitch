![Image of Note Detector](/docs/images/note-detector/note-detector_v2.jpg "Title")

# Table of Contents

1. [Introduction](#1-introduction)
   - [Overview of the Note Detector Module](#overview-of-the-note-detector-module)
   - [Features](#features)
   - [Quick-Start Guide](#quick-start-guide)

2. [Interface and Controls](#2-interface-and-controls)
   - [Front Panel Overview](#front-panel-overview)
   - [Input Ports](#input-ports)
   - [Output Ports](#output-ports)
   - [Controls](#controls)
   - [Display](#display)

3. [Operational Modes](#3-operational-modes)
   - [Trigger Mode](#trigger-mode)
   - [Gate Mode](#gate-mode)

4. [Clocked vs. Freeform Modes](#4-clocked-vs-freeform-modes)
   - [Clocked Mode](#clocked-mode)
   - [Freeform Mode](#freeform-mode)

5. [Customizing Settings](#5-customizing-settings)
   - [Output Mode Selection](#output-mode-selection)
   - [Trigger Length Adjustment](#trigger-length-adjustment)
   - [Tolerance Level Configuration](#tolerance-level-configuration)
   - [Notation Type Selection](#notation-type-selection)

6. [Technical Specifications](#6-technical-specifications)
   - [Input and Output Voltage Ranges](#input-and-output-voltage-ranges)
   - [Frequency Control](#frequency-control)

7. [Support](#7-support)



# 1. Introduction

## Overview of the Note Detector Module

The Note Detector module is an innovative tool designed for the VCV Rack platform, catering to enthusiasts and professionals in the field of modular synthesis. This module primarily serves as a pitch detection device, accurately identifying the note and octave from incoming control voltage (CV) signals.

The Note Detector is a collaboration between Voxglitch and Omri Cohen.

### Features:
- Detection of notes and octaves from CV signals.
- Configurable output for generating either triggers or gates corresponding to pitch detection.
- Fine-grained tolerance settings for pitch detection
- A user-friendly display providing instant visual feedback of the target note.
- Clock input for detecting repeated notes, allowing multiple detections of the same note within a sequence.

### Quick-Start Guide

To quickly integrate the Note Detector into your workflow:

1. Patch a CV source (e.g., a sequencer or keyboard module that outputs CV pitch information) to the CV IN port of the Note Detector.
2. Connect a gate or trigger source from the same sequencer to the CLOCK IN port. This will enable the Note Detector to register repeated notes in the sequence.
3. Select the target note using the NOTE and OCTAVE knobs
4. Connect the OUTPUT to the desired module (e.g., an envelope generator or drum module)

#### Sample Patch:
- Sequencer Note CV Output > CV IN on Note Detector.
- Sequencer Gate Output > CLOCK IN on Note Detector (to handle repeated notes).
- Note Detector OUTPUT > CV Input to a kick drum module or ADSR

This setup enables the Note Detector to track and output each instance of a note, including repeated notes.  

You can omit the clock input cable, which will cause the module to run in "Freeform Mode" (see below), which will still output a trigger or gate when the note is detected, but will be unable to detect when the same note repeats in a sequence since the pitch doesn't change.

# 2. Interface and Controls

## Front Panel Overview

The front panel of the Note Detector is elegantly laid out to ensure immediate accessibility and ease of use. It consists of input and output jacks, control knobs for precise adjustments, and a digital display for instant note verification.

### Input Ports
- **CV IN**: Plug the output from your CV source—like a sequencer or keyboard—into this jack. The module will scrutinize the incoming voltage to discern the note being played.
- **CLOCK IN**: This jack accepts a gate or trigger signal that matches the rhythm of your notes. It's crucial for recognizing consecutive occurrences of the same note, ensuring each is accurately detected and articulated.  If you do not connect a cable to the CLOCK INPUT, the module is considered to be in "freeform mode" (see the section below on Clocked vs. Freeform modes.)

### Output Ports
- **OUTPUT**: This jack emits a trigger or gate signal in response to the detected notes. The character of this signal, whether a succinct trigger or an elongated gate, is contingent on the module's Output Mode, which is configured through the context menu.

### Controls
- **NOTE Knob**: This control allows you to dial in the specific note that the module will detect. When combined with the OCTAVE knob, it enables you to target the exact pitch for the module to track.
- **OCTAVE Knob**: This control is used to set the octave for the note you're aiming to detect. It can be adjusted to isolate a single octave or configured to the "All" setting, which commands the module to detect the chosen note across all octaves.

### Display
- **Note Readout**: Displays the note and (optionally) octave that you wish to detect.  It can be configured to display notes with sharps or flats through the "Notation" context menu.


# 3. Operational Modes

The Note Detector module offers two primary operational modes to suit different musical applications and preferences: Trigger Mode and Gate Mode. These modes determine how the module responds when it detects the specified note.

## Trigger Mode

**Description:**  
Trigger Mode is designed to output a short, percussive pulse when the module detects the chosen note. This mode is ideal for triggering events with precision, such as starting an envelope, advancing a sequencer, or activating a drum hit.  The trigger length, which defaults to 0.01 seconds, is configured through the Trigger Length context menu.

**How to Use:**

1. Connect your CV and Clock sources to the CV IN and CLOCK IN jacks, respectively.
2. Select the desired note and octave using the NOTE and OCTAVE knobs.
3. Engage Trigger Mode by selecting it from the context menu (right-click on the module to access it).
4. The OUTPUT will emit a brief trigger pulse each time the selected note is detected, useful for triggering envelope generators, clocking sequencers, or most all other common applications.

**Application Example:**  
Set the sequencer to loop a melody and the Note Detector to trigger a drum module every time a C# note is played, regardless of the octave.

## Gate Mode

**Description:**  
Gate Mode outputs a continuous voltage signal as long as the detected note matches the selected parameters. This mode is useful for sustaining notes or effects, like holding a note on a synthesizer or maintaining a gate signal for the duration of a note.

**How to Use:**

1. Ensure that your CV and Clock sources are patched into the CV IN and CLOCK IN jacks.
2. Set the NOTE and OCTAVE knobs to define the pitch that will hold the gate open.
3. Switch to Gate Mode via the context menu.
4. The OUTPUT will produce a sustained 10V signal for as long as the specified note is present at the input.

**Application Example:**  
Use the Note Detector to sustain a pad sound on a polyphonic synthesizer module every time an F# in the 3rd octave is detected, creating a layered texture over a melody.

# 4. Clocked vs. Freeform Modes

## Clocked Mode
When a cable is connected to the clock input, evaluation of the note input will only occur when the clock signal transitions from high to low.  This is valuable for identifying instances of the same note that are adjacent in a sequence, that would be impossible to detect based on pitch change alone.

## Freeform Mode
The Note Detector is considered in "freeform" mode if a cable is not present in the CLOCK input.  In Freeform Mode, the Note Detector operates independently of an external clock signal. This mode allows the module to continuously and autonomously analyze the incoming CV signal for note detection. It's particularly effective in scenarios where the sequence of notes varies dynamically or does not require recognition of adjacent, repeated notes.


# 5. Customizing Settings

The Note Detector module provides a variety of settings that can be customized to fit your musical needs. These settings allow you to fine-tune how the module detects notes and interacts with the rest of your patch. The following subsections detail how to adjust these settings.

## Output Mode Selection

The Note Detector can output either triggers or gates based on the incoming CV signal. Here's how to select the output mode:

1. Right-click on the module to open the context menu.
2. Locate the Output Mode option.
3. Choose Trigger to output a short pulse when a note is detected.
4. Choose Gate to output a sustained voltage as long as the detected note is held.

## Trigger Length Adjustment

When in Trigger Mode, you can adjust the length of the output pulse:

1. Right-click on the module to access the context menu.
2. Select the Trigger Length submenu.
3. Choose from predefined trigger length options, which determine the duration of the pulse sent from the OUTPUT port.

## Tolerance Level Configuration

Adjust the tolerance level to control the sensitivity of the note detection:

1. Right-click on the module to bring up the context menu.
2. Navigate to the Tolerance Level submenu.
3. Select a tolerance level that matches your requirements, from exact note matching to more lenient detection that allows for microtonal variations.

## Notation Type Selection

You can choose between sharp and flat notation for the note display:

1. Right-click on the module to open the context menu.
2. Find the Notation option.
3. Select Sharp to display notes using sharp notation (e.g., C#, D#, etc.).
4. Select Flat to display notes using flat notation (e.g., Db, Eb, etc.).

# 6. Technical Specifications

The Note Detector module adheres to standard voltage ranges and behaviors expected in modular synthesis environments like VCV Rack. Here are its technical specifications:

## Input and Output Voltage Ranges
- **CV Input Range**: Typically 0 to 10 V (unipolar CV) or ±5 V (bipolar CV).
- **Output Voltage**: 
  - Triggers and Gates output at 10 V.
  - Default Trigger duration: Standard 1 ms, but customizable

## Frequency Control
- **Voltage Standard**: Follows the 1 V/oct standard.
- **Frequency Relationship**: The frequency \( f \) is defined as \( f = f_0 \times 2^V \), where \( f_0 \) is the baseline frequency. For audio-rate oscillators, \( f_0 \) is typically set to C4 (261.6256 Hz).

These specifications ensure that the Note Detector module performs reliably within the VCV Rack ecosystem, providing consistent and expected results in various synthesis applications.

# 7. Support

For questions, issues, or feature requests, please contact voxglitch@gmail.com, or post to [Voxglitch Community Feedback](https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&cad=rja&uact=8&ved=2ahUKEwjT48a2192CAxVdIUQIHSFOCg0QFnoECBkQAQ&url=https%3A%2F%2Fcommunity.vcvrack.com%2Ft%2Fvoxglitch-community-feedback%2F14290%3Fpage%3D44&usg=AOvVaw1j87Wokz8sJct5AudvwU_7&opi=89978449)

The Note Detector is open source: https://github.com/clone45/voxglitch/tree/master/src/NoteDetector