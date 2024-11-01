
# CueResearch: Implementation Deep Dive

## Observer Pattern Implementation

### CueResearch as the Subject

CueResearch is our source of truth for playback position. It maintains a vector of callbacks that want to know when the playback position changes:

```cpp
std::vector<std::function<void(unsigned int)>> playback_position_observers;
```

Any component that needs to know about playback position changes can register a callback:

```cpp
void registerPlaybackObserver(std::function<void(unsigned int)> callback);
```

### Update Flow

There are two ways the playback position can change:

1. **During Normal Playback**
   - The `process()` method updates playback position.
   - If the position changed, `broadcastPlaybackPosition()` is called.
   - All registered observers are notified.

2. **During User Interaction**
   - User drags the playhead in a widget.
   - Widget tells its model via `onDragPlayhead()`.
   - Model tells CueResearch via `onDragPlayhead()`.
   - CueResearch broadcasts to all observers.

## Component Communication

### Models → CueResearch

Models need to tell CueResearch when the user drags the playhead. They do this through a pointer to CueResearch that's set up during initialization.

### Widgets → Models

Widgets communicate with their models through direct pointer access. For example, TrackWidget has a pointer to TrackModel:

```cpp
TrackModel* track_model;
```

### Models → Widgets

Models notify their widgets of updates through callbacks. Each model maintains a callback that its widget sets during initialization:

```cpp
std::function<void(unsigned int)> on_playhead_changed;
```

## Initialization Chain

1. **CueResearch Creation**
   - CueResearch is created.
   - Creates TrackModel and WaveformModel.

2. **Widget Setup**
   - CueResearchWidget is created with a pointer to CueResearch.
   - Creates TrackWidget and WaveformWidget.
   - Sets up observer registrations.

3. **Callback Registration**
   - Models register with CueResearch for updates.
   - Widgets register with their models for updates.

## Important Considerations

### Memory Management
- Widgets are owned by CueResearchWidget.
- Models are owned by CueResearch.
- Raw pointers are used since the lifetime is managed by VCV Rack.

### Thread Safety
- Updates from `process()` happen in the audio thread.
- UI updates happen in the main thread.
- Ensure thread-safe updates of shared state.

### Callback Lifetime
- Callbacks are registered during initialization.
- No explicit cleanup needed as components have matching lifetimes.
- Be careful about capturing `this` in lambdas.

## Example Flow: User Drags Playhead

Let's follow what happens when a user drags the playhead:

1. Mouse event in TrackWidget triggers `onDragPlayhead()`.
2. Widget calculates the new position and calls `track_model->onDragPlayhead(position)`.
3. TrackModel calls `module->onDragPlayhead(position)`.
4. CueResearch updates the position and calls `broadcastPlaybackPosition()`.
5. Both models receive the update via their registered callbacks.
6. Models update their state and notify their widgets.
7. Widgets update their visual displays.

## Tips for Modifications

- Need a new component to watch playback position? Register an observer with CueResearch.
- Adding new interaction? Follow the existing pattern up through the model to CueResearch.
- Modifying visual feedback? Work in the widget's `updatePlayheadIndicator()`.
- Adding a new broadcast type? Follow the playback position observer pattern.
