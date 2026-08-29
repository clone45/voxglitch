# Timeline — the chase feature

2026-08-27, branch `dev/timeline`. Design only. No code written yet.

Bret's request: the playhead starts at the left edge and travels right. When
it reaches the centre of the view, the view begins to scroll and the playhead
stays centred. This replaces paging, which is what a piano roll usually does.
The feature can be switched on and off.

## 1. The core rule

The whole feature is one line:

```
desiredScroll = max(0, playhead - viewSpan / 2)
```

Everything Bret described falls out of that `max(0, ...)`:

- At the start, `playhead - viewSpan/2` is negative. The scroll stays at 0,
  so the playhead travels right across the first half of the view.
- Once the playhead passes the centre, the term goes positive. The scroll
  tracks it exactly, so the playhead stays centred.

There is no state machine and no second mode. The transition Bret described
is not coded anywhere. It is a consequence of clamping at zero.

## 2. Scroll becomes derived, not stored

This is the key design decision, and it removes most of the edge cases.

Today `scroll` is a stored value that the user changes by panning. While
chase is active, `scroll` is instead **derived** from the playhead every
frame. The stored value is ignored.

That single change resolves the zoom problem for free. Zooming only changes
`tpp` (beats per pixel). The next frame recomputes `desiredScroll` with the
new `viewSpan`, and the playhead is centred again. No zoom anchoring code is
needed, and no jump can accumulate.

When chase is suspended, `scroll` goes back to being stored, and everything
behaves as it does today.

## 3. Two flags, not one

- **`chaseEnabled`** — the user's toggle. Persisted with the patch.
- **`chaseSuspended`** — runtime only. Never persisted.

Chase acts when `chaseEnabled && !chaseSuspended`. Call that state *armed*.

Suspension is what stops chase from fighting the user. Without it, any pan is
undone on the next frame, which feels broken.

### When chase suspends

There are two kinds of suspension, and they resume differently.

**Persistent suspension** happens when the user deliberately moves the view:

- dragging the scrollbar thumb
- clicking the scrollbar track to page
- middle-dragging the body to pan
- edge auto-scroll firing during a node drag
- the menu items "Fit view to content" and "Reset zoom"

**Temporary suspension** happens for the length of a drag gesture that does
not move the view:

- dragging a node
- dragging the loop handle
- scrubbing the ruler

Temporary suspension matters more than it looks. A node's beat is computed
from the pointer position plus the scroll. If the scroll moves under the
pointer during a drag, the node slides away from the cursor. Suspending for
the gesture keeps the arithmetic stable.

### When chase resumes

From temporary suspension: on mouse release.

From persistent suspension, on whichever comes first:

- the transport seeks or the loop wraps (rewind, RESET, loop end)
- the playhead **leaves the visible window and then comes back**
- the user toggles chase off and on again

The second rule makes it self-healing. If the user pans ahead to look at
something, chase picks up again when the music arrives there.

> **Correction, found during implementation.** The second rule was first
> written as "the playhead re-enters the visible window", and implemented as
> a level test: *is the playhead visible?* That re-armed the chase on the very
> next frame whenever the user nudged the view while the playhead was on
> screen, which is the commonest case. Breaking the chase did nothing.
>
> The rule must be **edge triggered**. The playhead has to actually leave the
> view and return. A flag records that it has been outside since the break.
> `tests/timeline/chase_test.cpp` caught this, and now guards it.
>
> A consequence worth knowing: after a small nudge, chase stays broken. The
> view is static, so the playhead runs off the right edge and never returns.
> It re-arms on the next rewind, loop wrap or seek. That is predictable, and
> it matches how most DAWs treat a manual scroll.

## 4. Chase is not tied to playback

An earlier draft only chased while playing. That turned out worse.

Chase is simply "the scroll follows the playhead". If the transport is
stopped and nothing moves, chase does nothing, so the play-only rule bought
nothing. It also broke scrubbing: after dragging the ruler, the view would
not recentre on the new position.

So chase is active whenever it is armed, regardless of transport state.

## 5. Edge cases

| # | Case | Behaviour | Why it is right |
|---|------|-----------|-----------------|
| 1 | Zoom while chasing | Playhead stays centred, content spreads or contracts around it | Scroll is derived, so the next frame recentres |
| 2 | Zoom in so far that the playhead was already past the new centre | View recentres in one frame | A jump the user caused by their own action |
| 3 | Zoom out until the whole song fits | No scrolling at all | `playhead - viewSpan/2` stays negative, so scroll stays 0 |
| 4 | Loop wrap | View snaps back to the start | The playhead really did jump. Also clears persistent suspension |
| 5 | Rewind or RESET | Same as a loop wrap | Same reason |
| 6 | Chase switched on with the playhead off screen | View jumps to centre it | The user asked for it just now |
| 7 | Chase switched on with the playhead before the centre | Nothing moves until it passes the centre | Matches the requested behaviour |
| 8 | User pans while chasing | Chase suspends, the pan holds | Chase must never fight the pointer |
| 9 | User drags a node while chasing | Chase suspends for the gesture, resumes on release | Keeps pointer-to-beat arithmetic stable |
| 10 | Edge auto-scroll fires during a node drag | Promoted to persistent suspension | The user moved the view, even if indirectly |
| 11 | Ruler scrub | Suspends for the drag, recentres on release | The playhead moved, so centring it is correct |
| 12 | Elastic scroll extent | No interaction | Chase's `viewEnd` is `playhead + viewSpan/2`, always inside the extent, so `clampScroll` never bites |
| 13 | Scrollbar thumb while chasing | Moves on its own, still grabbable | Grabbing it is a pan, so it suspends chase |
| 14 | Editor locked | Chase still works | The lock blocks mouse events. Chase is not one |
| 15 | Very high zoom | Playhead appears still, content flows past | Correct, and it is the point of the feature |
| 16 | Playhead at 0, stopped, chase on | Scroll pinned at 0 | The user can still pan, which suspends chase |
| 17 | Two Timelines in one rack | Independent, if the toggle is per module | See the open question below |

## 6. Reading the playhead

Chase runs in `draw()` on the UI thread. It reads `engine.playhead`, which
the audio thread writes every sample.

This race is benign and already exists: the playhead line and the bar/beat
readout read the same value. A torn read would mis-place the view by one
frame, which is invisible. No lock is needed, and a lock here would be worse
than the problem.

## 7. Where the toggle lives

Timeline follows Tracks, which puts "Chase Playhead" in the module context
menu. One difference needs a decision.

Tracks stores its chase flag in `voxglitchSettings.chasePlayhead`, which is
**global to the plugin**. Timeline could either:

- **(a) Share that global flag.** Consistent with Tracks. One preference
  covers every module that follows a playhead. But switching it in Timeline
  also switches it in Tracks, which may surprise.
- **(b) Keep a per-module flag**, saved in `dataToJson` next to `locked`.
  Two Timelines can then behave differently, and a patch remembers what it
  was set to.

**Recommendation: (b).** Timeline instances are independent, and a patch that
opens with the view where you left it is worth more than cross-module
consistency. Bret's call.

**Decided: (b), and promoted to a panel switch.** Bret asked for front-panel
access, so chase is a real param (`CHASE_PARAM`) rather than a menu bool.
That gives MIDI mapping, undo and preset support for free, and params persist
on their own, so the hand-written JSON entry was removed. The duplicate menu
item is gone too: one control per thing. `locked` stays a menu bool, matching
Tracks.

## 8. Should the centre be visible?

An armed chase changes what the view does, so it should be visible somehow.
Options, cheapest first:

- The menu item's tick is the only indicator. No panel change.
- A short tick mark in the ruler at the centre, drawn only while armed.
- The playhead line brightens while armed.

**Recommendation: the ruler tick.** It shows both that chase is on and where
the anchor is, and it disappears when chase suspends, which tells the user
why the view stopped following.

## 9. Test plan

The scroll maths is testable outside Rack, in the style of
`tests/timeline/scrollbar_test.cpp`. A `chase_test.cpp` mirroring
`desiredScroll` and the suspend rules should check:

1. The playhead travels from the left edge to the centre with no scrolling.
2. Past the centre, the playhead's screen position stays fixed within a
   pixel, across many samples.
3. Zooming in and out at any playhead position keeps it centred.
4. A loop wrap returns the scroll to 0.
5. Panning suspends, and the pan holds for at least one frame.
6. The playhead re-entering the view clears a persistent suspension.
7. A temporary suspension clears on release.
8. Chase never drives the scroll outside the elastic extent.

## 10. Open questions for Bret

1. Per-module toggle or the shared `voxglitchSettings.chasePlayhead`?
   (Recommend per-module.)
2. Ruler tick as the indicator, or menu tick only?
   (Recommend the ruler tick.)
3. Should the anchor always be the exact centre, or would an offset such as
   40% ever be wanted, so more of the future is visible than the past?
   (Recommend a fixed centre. It can be widened later without breaking
   anything, since the anchor is one constant.)
