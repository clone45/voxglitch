# CueResearch Module Architecture Guide

Hey there! 👋 Let's walk through how the CueResearch module is structured. Don't worry if it seems complex at first - we'll break it down piece by piece.

## Overview

CueResearch is a sample playback module that shows two waveform views of your audio: a main track view and a smaller overview. Both views let you see and control where you are in the sample through a playhead indicator.

## The Main Components

We've split things up into three types of components:

- **The main module (CueResearch)**
- **Models (TrackModel and WaveformModel)**
- **Widgets (TrackWidget and WaveformWidget)**

### CueResearch (The Brain)

Think of CueResearch as the conductor of our orchestra. It:

- Owns the actual audio sample
- Creates and owns both models (TrackModel and WaveformModel)
- Handles the actual audio playback
- Broadcasts playback position changes to anyone who's interested

CueResearch doesn't know anything about the visual stuff - it just manages the audio and tells everyone else where we are in the sample.

### The Models (State Managers)

We have two models: **TrackModel** and **WaveformModel**. They're owned by CueResearch and manage the state for their respective views. Think of them as the middlemen between CueResearch and the visual widgets.

Each model:

- Knows its current state (like zoom level, visible window, etc.)
- Can tell CueResearch when the user drags the playhead
- Can update its state when CueResearch broadcasts a new playback position

### The Widgets (Visual Stuff)

**TrackWidget** and **WaveformWidget** handle all the visual aspects. They're created by CueResearchWidget and each has a pointer to its respective model.

The widgets:

- Draw the waveforms
- Handle mouse interaction
- Show the playhead indicator
- Tell their models when the user drags things around

## How It All Works Together

Let's walk through two common scenarios:

### Scenario 1: Normal Playback

1. CueResearch is playing audio and updates the playback position.
2. It broadcasts this new position.
3. Both models receive this update.
4. The models tell their widgets to update their playhead indicators.
5. You see the playhead move smoothly across both waveforms.

### Scenario 2: User Drags the Playhead

1. User clicks and drags in one of the widgets.
2. Widget tells its model "hey, they're dragging the playhead!"
3. Model tells CueResearch about the new position.
4. CueResearch broadcasts this position to all models.
5. Both widgets update to show the playhead in the new spot.

## Why This Design?

We chose this structure because:

- It's clean and organized - everything has its job.
- Changes in one part won't break other parts.
- We can reuse components (like the WaveformWidget) in other projects.
- It's easy to add new features without messing up existing code.

## Tips for Developers

- **Widgets only talk to their models, never directly to CueResearch.**
- **Models don't know about widgets - they just provide and accept data.**
- **If you need to add a new feature that needs to know the playback position, just register for updates from CueResearch.**
- **When modifying the drawing code, you only need to look at the widget code.**
- **When modifying audio behavior, you only need to look at CueResearch.**

Need to make changes? Feel free to ask questions! This architecture is designed to be friendly to modify and extend.
