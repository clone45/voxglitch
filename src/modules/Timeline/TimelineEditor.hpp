#pragma once
// TimelineEditor.hpp — the automation editor, ported from vxsynth's
// Canvas2D TimelineEditor (~/vxsynth/app/studio/lib/automation/timeline-editor.js)
// to a NanoVG Rack widget. Same interaction math:
//   time<->pixel   t2x(t) = (t - scroll) / tpp ; x2t(x) = scroll + x*tpp
//                  (tpp = BEATS per pixel = zoom; scroll = pan, in beats)
//   wheel zoom     x0.8 / x1.25, re-anchored so the beat under the pointer
//                  stays put
//   scrollbar      a conventional bottom scrollbar: the thumb's length shows
//                  how much of the content is on screen, drag it to pan,
//                  click the track to page. (The web original used Tracks'
//                  pan strip — drag-anywhere plus a rewind button; Bret asked
//                  for a real scrollbar 2026-08-27. Rewind still lives on the
//                  panel's RWND button.) Middle-drag the body also pans.
//   ruler scrub    drag the top ruler to seek
//   node edit      click empty = add (and keep dragging it in the same
//                  gesture); drag a node = move; right-click a node = delete
// One committed gesture = ONE undo step.
//
// It owns no data: it reads the module's lane store and pushes edits back
// through laneCopy()/publishLane(), which copy ONE lane, mutate the copy
// and swap it in atomically for the audio thread.

#include "Timeline.hpp"
#include <cstdio>
#include <cmath>
#include <vector>

namespace timeline_ui
{

static const float RULER_H   = 18.f;   // top ruler / scrub strip
static const float PANSTRIP_H = 12.f;  // bottom pan strip
static const float PAD_V     = 8.f;    // padding inside the voltage plane
static const float NODE_R    = 3.5f;
static const float HANDLE_R  = 4.5f;   // the bend handle diamond
static const float HIT_PX    = 8.f;
static const float THUMB_MIN = 24.f;   // shortest draggable scrollbar thumb
static const float LOOP_HIT  = 5.f;
static const double LOOP_MIN = 1.0;
static const double V_MAX    = 10.0;
static const double TPP_MIN  = 0.002;  // beats/px, most zoomed in
static const double TPP_MAX  = 256.0;
static const double BAR      = 4.0;

// Rack's math::clamp takes floats; beats are doubles (86,400 of them, with
// sub-beat precision), so clamping through float would quantise node times.
inline double tlClamp(double x, double lo, double hi)
{
    return x < lo ? lo : (x > hi ? hi : x);
}

inline NVGcolor tcol(int hex, float a = 1.f)
{
    return nvgRGBA((hex >> 16) & 0xff, (hex >> 8) & 0xff, hex & 0xff, (unsigned char)(a * 255.f));
}

// ── undo: the touched lanes' nodes before and after a gesture ───────────────
// A mouse gesture touches one lane; a recording take touches every lane it
// recorded, and is still ONE undo step.
struct LaneEditAction : history::ModuleAction
{
    struct Entry
    {
        int lane = 0;
        timeline_dsp::LaneData before;
        timeline_dsp::LaneData after;
    };
    std::vector<Entry> entries;

    void apply(bool forward)
    {
        Module* m = APP->engine->getModule(moduleId);
        Timeline* tl = dynamic_cast<Timeline*>(m);
        if (!tl) return;
        for (size_t i = 0; i < entries.size(); i++)
            tl->publishLane(entries[i].lane, forward ? entries[i].after : entries[i].before);
    }
    void undo() override { apply(false); }
    void redo() override { apply(true); }
};

struct TimelineEditorWidget : OpaqueWidget
{
    Timeline* module = NULL;

    double tpp = 0.05;        // beats per pixel
    double scroll = 0.0;      // leftmost beat
    Vec mousePos;

    // drag state
    enum DragMode { DRAG_NONE, DRAG_NODE, DRAG_PAN, DRAG_SCRUB, DRAG_LOOP, DRAG_SCROLL, DRAG_BEND };
    int dragMode = DRAG_NONE;
    int dragNode = -1;
    int dragSeg = -1;             // segment being bent (DRAG_BEND)
    int hoverSeg = -1;            // segment whose bend handle shows
    Vec dragPos;
    timeline_dsp::LaneData gestureBefore;   // the lane BEFORE the gesture
    int hoverNode = -1;
    float grabDX = 0.f;          // pointer offset inside the thumb when grabbed
    bool hoverThumb = false;

    // ── chase (design: timeline-chase-design.md) ───────────────────────────
    // Two kinds of suspension. `chaseHeld` lasts one gesture and clears on
    // release (a node drag must not have the scroll move under the pointer,
    // or the node slides away from the cursor). `chaseBroken` is the user
    // deliberately moving the view; it clears on a seek, a loop wrap, or when
    // the playhead comes back into view.
    bool chaseHeld = false;
    bool chaseBroken = false;
    bool chaseLeftView = false;   // the playhead has been off screen since the break
    int lastSeenSeek = 0;

    bool chaseArmed()
    {
        return module && module->chaseOn() && !chaseHeld && !chaseBroken;
    }

    double lastPlayhead = 0.0;
    int lastChaseLane = -1;       // chase follows the SELECTED lane; switching
                                  // tabs must not read as a rewind

    // ── recording drain (the PianoRoll idiom) ──────────────────────────────
    // The audio thread pushes captures into the module's ring; this widget
    // drains it once per frame, on the UI thread, which is the only thread
    // allowed to publish lanes. Each capture replaces the nodes the
    // playhead passed over since the previous one and inserts the new
    // node, committed through the pointer-swap path so playback and the
    // drawing follow within a frame. When every recorded lane has its END,
    // the whole take becomes ONE undo step.
    timeline_dsp::TakeAssembler take;

    // A finished take becomes one undo action. Called BEFORE a START is
    // applied as well as after the drain: when a take's END and the next
    // take's START arrive in the same frame, applying the START first would
    // reset that lane's record and lose the finished take's `before`.
    void finishTake()
    {
        using namespace timeline_dsp;
        if (!module || !take.complete()) return;
        LaneEditAction* a = new LaneEditAction;
        a->moduleId = module->id;
        a->name = "record automation";
        for (int L = 0; L < TL_LANES; L++)
        {
            if (!take.lanes[L].open) continue;
            LaneEditAction::Entry e;
            e.lane = L;
            e.before = take.lanes[L].before;
            e.after = module->laneCopy(L);
            a->entries.push_back(e);
        }
        APP->history->push(a);
        take.reset();
    }

    void drainCaptures()
    {
        using namespace timeline_dsp;
        if (!module) return;
        Capture c;
        while (module->captureRing.pop(c))
        {
            if (c.lane < 0 || c.lane >= TL_LANES) continue;
            if (c.kind == CAP_START) finishTake();
            LaneData d = module->laneCopy(c.lane);
            if (!take.apply(c, d)) continue;
            module->publishLane(c.lane, d);                 // clears laneFull
            module->laneFull[c.lane] = take.lanes[c.lane].full != 0;
        }
        finishTake();
    }

    void step() override
    {
        drainCaptures();
        OpaqueWidget::step();
    }

    // The lane the editor is showing — its playhead is the one the view, the
    // readout and chase all follow (Bret's call, 2026-08-28).
    timeline_dsp::Lane& selLane() { return module->engine.lanes[laneIdx()]; }

    // Called once per frame, before anything reads `scroll`.
    //
    // While chase is armed the scroll is DERIVED, not stored. That is what
    // makes zoom free: the wheel only changes tpp, and the next frame
    // recentres with the new viewSpan. No zoom anchoring, no accumulated
    // drift.
    void updateChase()
    {
        if (!module) return;
        // Switching tabs swaps which playhead we watch; re-prime the memories
        // so the jump is not mistaken for a rewind.
        if (laneIdx() != lastChaseLane)
        {
            lastChaseLane = laneIdx();
            lastPlayhead = selLane().playhead;
            lastSeenSeek = selLane().seekSerial;
        }
        double ph = selLane().playhead;

        // A backward jump is a rewind, a RESET or a loop wrap; a serial bump
        // is any seek. Both re-arm a broken chase. (The engine's `wrapped`
        // flag lasts one SAMPLE, so the UI thread would nearly always miss
        // it — the backward comparison is what actually works here.)
        int serial = selLane().seekSerial;
        if (ph < lastPlayhead || serial != lastSeenSeek) chaseBroken = false;
        lastSeenSeek = serial;
        lastPlayhead = ph;

        // Self-healing, EDGE triggered: the playhead must have actually left
        // the view and come back. A level test ("is it visible?") re-armed on
        // the very next frame whenever the user nudged the view while the
        // playhead was on screen, which made breaking the chase useless in
        // the commonest case. Caught by tests/timeline/chase_test.cpp.
        if (chaseBroken)
        {
            bool inside = (ph >= scroll && ph <= scroll + viewSpan());
            if (!inside) chaseLeftView = true;
            else if (chaseLeftView) { chaseBroken = false; chaseLeftView = false; }
        }

        if (!chaseArmed()) return;

        double want = ph - viewSpan() * 0.5;
        scroll = (want > 0.0) ? want : 0.0;   // the whole feature is this line
    }

    // Any deliberate view move breaks the chase until it re-arms.
    void breakChase() { chaseBroken = true; chaseLeftView = false; }

    float bodyTop() { return RULER_H; }
    float bodyBot() { return box.size.y - PANSTRIP_H; }

    float t2x(double t) { return (float)((t - scroll) / tpp); }
    double x2t(float x) { return scroll + (double)x * tpp; }

    float v2y(double v)
    {
        float top = bodyTop() + PAD_V, bot = bodyBot() - PAD_V;
        float mid = (top + bot) * 0.5f, half = (bot - top) * 0.5f;
        return mid - (float)(v / V_MAX) * half;
    }
    double y2v(float y)
    {
        float top = bodyTop() + PAD_V, bot = bodyBot() - PAD_V;
        float mid = (top + bot) * 0.5f, half = (bot - top) * 0.5f;
        if (half <= 0.f) return 0.0;
        double v = (double)((mid - y) / half) * V_MAX;
        return tlClamp(v, -V_MAX, V_MAX);
    }

    double applySnap(double beat)
    {
        if (!module) return beat;
        double s = module->snapBeats();
        if (s <= 0.0) return beat;
        return std::round(beat / s) * s;
    }
    double clampT(double beat)
    {
        return tlClamp(beat, 0.0, timeline_dsp::TL_MAX_BEAT);
    }

    int laneIdx() { return module ? module->currentLane() : 0; }

    // ── scrollbar geometry ─────────────────────────────────────────────────
    // The scrollable extent is the CONTENT, not the engine's 86,400-beat
    // ceiling — a thumb sized against the ceiling would be a pixel wide.
    double viewSpan() { return (double)box.size.x * tpp; }

    // The furthest thing that actually exists: nodes, the loop end, the
    // playhead.
    double contentEnd()
    {
        double last = 0.0;
        if (module)
        {
            last = module->lastBeat();
            if (module->params[Timeline::LOOP_PARAM].getValue() > 0.5f)
            {
                double le = module->effectiveLoopEnd();
                if (le > last) last = le;
            }
            if (selLane().playhead > last) last = selLane().playhead;
        }
        return last;
    }

    // ELASTIC extent: always ONE SCREEN of empty future beyond whichever is
    // further, the content or where you are already looking. A timeline is
    // composed forward, so a scrollbar clamped to existing content walls you
    // out of the very place you want to put the next node. The extent grows
    // as you travel, so there is never a wall — the thumb just shrinks and
    // approaches, without reaching, the right end.
    //
    // While the thumb is being dragged the extent is LATCHED: letting it grow
    // mid-drag makes scroll-per-pixel change under the pointer, and the thumb
    // rubber-bands away from the cursor.
    double latchedSpan = 0.0;

    double contentSpan()
    {
        if (dragMode == DRAG_SCROLL && latchedSpan > 0.0) return latchedSpan;
        double v = viewSpan();
        // NB: not `far` — MinGW still defines it as a DOS memory-model keyword.
        double reach = contentEnd();
        double viewEnd = scroll + v;
        if (viewEnd > reach) reach = viewEnd;
        double span = reach + v;                       // one screen of future
        return span > v ? span : v;
    }

    double maxScroll()
    {
        double m = contentSpan() - viewSpan();
        return m > 0.0 ? m : 0.0;
    }

    void clampScroll() { scroll = tlClamp(scroll, 0.0, maxScroll()); }

    float thumbW()
    {
        double total = contentSpan(), view = viewSpan();
        float w = (float)(view / total) * box.size.x;
        if (w < THUMB_MIN) w = THUMB_MIN;
        if (w > box.size.x) w = box.size.x;
        return w;
    }

    float thumbX()
    {
        double m = maxScroll();
        if (m <= 0.0) return 0.f;
        float travel = box.size.x - thumbW();
        return (float)(scroll / m) * travel;
    }

    void scrollFromThumbX(float x)
    {
        float travel = box.size.x - thumbW();
        if (travel <= 0.f) { scroll = 0.0; return; }
        scroll = tlClamp((double)(x / travel) * maxScroll(), 0.0, maxScroll());
    }

    // Nearest node to a pixel position, or -1.
    int nodeAt(Vec p)
    {
        if (!module) return -1;
        const timeline_dsp::LaneData& ld = module->lane(laneIdx());
        int best = -1;
        float bestD = HIT_PX;
        int n = ld.count();
        for (int i = 0; i < n; i++)
        {
            float dx = t2x(ld.t[i]) - p.x;
            float dy = v2y(ld.v[i]) - p.y;
            float d = std::sqrt(dx * dx + dy * dy);
            if (d < bestD) { bestD = d; best = i; }
        }
        return best;
    }

    // ── segment bend (curves between nodes) ────────────────────────────────
    // One parameter per segment, the exponential-approach shape:
    // value = v0 + (v1-v0) * (1-e^(-b*frac))/(1-e^(-b)). The handle is a
    // diamond at the segment's midpoint, ON the curve. While dragging, the
    // bend is solved so the curve passes through the POINTER — both axes, so
    // it feels like pulling a rubber band, not riding a vertical slider
    // (Bret's feel note: vertical-only read as a constraint). The drag has a
    // straight detent, and right-clicking the handle straightens.
    static double bendFrac(double frac, double bend)
    {
        if (bend < 1e-9 && bend > -1e-9) return frac;
        return (1.0 - std::exp(-bend * frac)) / (1.0 - std::exp(-bend));
    }

    // The curve's y at pixel x inside segment i.
    float segCurveY(const timeline_dsp::LaneData& ld, int i, float x)
    {
        double t0 = ld.t[i], t1 = ld.t[i + 1];
        double v0 = ld.v[i], v1 = ld.v[i + 1];
        double t = x2t(x);
        double frac = (t1 > t0) ? (t - t0) / (t1 - t0) : 0.0;
        frac = bendFrac(tlClamp(frac, 0.0, 1.0), ld.b[i]);
        return v2y(v0 + frac * (v1 - v0));
    }

    Vec bendHandlePos(int i)
    {
        const timeline_dsp::LaneData& ld = module->lane(laneIdx());
        double tm = ((double)ld.t[i] + (double)ld.t[i + 1]) * 0.5;
        double v0 = ld.v[i], v1 = ld.v[i + 1];
        double frac = bendFrac(0.5, ld.b[i]);
        return Vec(t2x(tm), v2y(v0 + frac * (v1 - v0)));
    }

    // The handle under the pointer, or -1. Only the hovered (or dragged)
    // segment shows a handle, so only that one is hittable — clicking the
    // curve anywhere else still adds a node, as it always did.
    int bendHandleSeg(Vec p)
    {
        int seg = (dragMode == DRAG_BEND) ? dragSeg : hoverSeg;
        if (!module || seg < 0) return -1;
        if (seg + 1 >= module->lane(laneIdx()).count()) return -1;
        Vec h = bendHandlePos(seg);
        return (std::fabs(h.x - p.x) <= HANDLE_R + 3.f
                && std::fabs(h.y - p.y) <= HANDLE_R + 3.f) ? seg : -1;
    }

    // The segment whose curve passes near the pointer (for hover).
    int segAt(Vec p)
    {
        if (!module) return -1;
        const timeline_dsp::LaneData& ld = module->lane(laneIdx());
        int n = ld.count();
        double t = x2t(p.x);
        for (int i = 0; i + 1 < n; i++)
        {
            if (t < ld.t[i] || t > ld.t[i + 1]) continue;
            return (std::fabs(segCurveY(ld, i, p.x) - p.y) <= 8.f) ? i : -1;
        }
        return -1;
    }

    // Snapshot the current lane (for the undo record).
    void snapshot() { gestureBefore = module ? module->laneCopy(laneIdx()) : timeline_dsp::LaneData(); }

    void pushGesture(const char* name)
    {
        if (!module) return;
        LaneEditAction* a = new LaneEditAction;
        a->moduleId = module->id;
        a->name = name;
        LaneEditAction::Entry e;
        e.lane = laneIdx();
        e.before = gestureBefore;
        e.after = module->laneCopy(e.lane);
        a->entries.push_back(e);
        APP->history->push(a);
    }

    // ── events ─────────────────────────────────────────────────────────────
    void onHover(const HoverEvent& e) override
    {
        mousePos = e.pos;
        hoverNode = (module && module->locked) ? -1 : nodeAt(e.pos);
        hoverSeg = (hoverNode < 0 && module && !module->locked
                    && e.pos.y > bodyTop() && e.pos.y < bodyBot())
                   ? segAt(e.pos) : -1;
        if (e.pos.y >= bodyBot())
        {
            float tx = thumbX(), tw = thumbW();
            hoverThumb = (e.pos.x >= tx && e.pos.x <= tx + tw);
        }
        else hoverThumb = false;
        OpaqueWidget::onHover(e);
        e.consume(this);
    }

    void onHoverScroll(const HoverScrollEvent& e) override
    {
        if (e.scrollDelta.y == 0.f) return;
        if (module && module->locked) return;      // Tracks blocks the wheel too
        // Let the wheel pass through to Rack when the rack is zoomed out:
        // at that size the editor is too small to aim at, and swallowing the
        // wheel traps the user's view scroll. Returning WITHOUT consuming is
        // what hands the event back to Rack. (The Tracks threshold, 0.95.)
        if (getAbsoluteZoom() < 0.95f) return;
        double anchor = x2t(e.pos.x);                 // beat under the pointer
        double factor = (e.scrollDelta.y > 0.f) ? 0.8 : 1.25;
        tpp = tlClamp(tpp * factor, TPP_MIN, TPP_MAX);
        scroll = anchor - (double)e.pos.x * tpp;      // keep that beat fixed
        clampScroll();
        e.consume(this);
    }

    void onButton(const ButtonEvent& e) override
    {
        if (!module) return;
        // Locked: freeze the editor against every mouse event (the Tracks
        // rule, and its position — first line, before anything else runs).
        // Deliberately NOT consumed, so the press falls through to the
        // module's own context menu, where Lock Editor can be switched off.
        if (module->locked) return;

        mousePos = e.pos;
        if (e.action != GLFW_PRESS) { OpaqueWidget::onButton(e); return; }

        bool inRuler = e.pos.y < bodyTop();
        bool inPan = e.pos.y >= bodyBot();

        if (e.button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            int hit = (!inRuler && !inPan) ? nodeAt(e.pos) : -1;
            if (hit >= 0)
            {
                snapshot();
                timeline_dsp::LaneData ld = module->laneCopy(laneIdx());
                ld.erase(hit);
                module->publishLane(laneIdx(), ld);
                pushGesture("delete automation node");
            }
            else if (!inRuler && !inPan && bendHandleSeg(e.pos) >= 0)
            {
                // Right-click the bend handle: back to a straight line.
                snapshot();
                int seg = bendHandleSeg(e.pos);
                timeline_dsp::LaneData ld = module->laneCopy(laneIdx());
                if (seg < ld.count()) ld.b[seg] = 0.f;
                module->publishLane(laneIdx(), ld);
                pushGesture("straighten automation segment");
            }
            else
            {
                createContextMenu();
            }
            e.consume(this);
            return;
        }

        if (e.button == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            breakChase();                      // panning is a deliberate move
            dragMode = DRAG_PAN;
            dragPos = e.pos;
            e.consume(this);
            return;
        }
        if (e.button != GLFW_MOUSE_BUTTON_LEFT) { OpaqueWidget::onButton(e); return; }

        if (inPan)
        {
            // Grab the thumb, or page toward a click in the track.
            float tx = thumbX(), tw = thumbW();
            breakChase();                      // both branches move the view
            if (e.pos.x >= tx && e.pos.x <= tx + tw)
            {
                latchedSpan = contentSpan();   // freeze BEFORE the mode change
                dragMode = DRAG_SCROLL;
                grabDX = e.pos.x - tx;
                dragPos = e.pos;
            }
            else
            {
                double page = viewSpan() * 0.9;
                scroll += (e.pos.x < tx) ? -page : page;
                clampScroll();
            }
            e.consume(this);
            return;
        }

        if (inRuler)
        {
            // The loop handle lives at the loop end when looping is on.
            if (module->params[Timeline::LOOP_PARAM].getValue() > 0.5f)
            {
                float lx = t2x(module->effectiveLoopEnd());
                if (std::fabs(lx - e.pos.x) < LOOP_HIT)
                {
                    chaseHeld = true;          // hold for the gesture only
                    dragMode = DRAG_LOOP;
                    dragPos = e.pos;
                    e.consume(this);
                    return;
                }
            }
            chaseHeld = true;                  // hold for the gesture only
            dragMode = DRAG_SCRUB;
            dragPos = e.pos;
            module->engine.seekAll(clampT(x2t(e.pos.x)));   // scrub = global
            e.consume(this);
            return;
        }

        // The bend handle wins over add-node: it is a small, visible target
        // sitting ON the curve, and stealing this click is the whole point.
        {
            int seg = bendHandleSeg(e.pos);
            if (seg >= 0)
            {
                snapshot();
                chaseHeld = true;
                dragMode = DRAG_BEND;
                dragSeg = seg;
                dragPos = e.pos;
                e.consume(this);
                return;
            }
        }

        // Body: grab an existing node, or add one and keep dragging it.
        snapshot();
        int hit = nodeAt(e.pos);
        if (hit < 0)
        {
            double beat = clampT(applySnap(x2t(e.pos.x)));
            double volt = y2v(e.pos.y);
            timeline_dsp::LaneData ld = module->laneCopy(laneIdx());
            hit = ld.insert(beat, volt);
            if (hit < 0) { e.consume(this); return; }   // lane full
            module->publishLane(laneIdx(), ld);
        }
        chaseHeld = true;                      // hold for the gesture only:
                                               // a scroll moving under the
                                               // pointer slides the node away
        dragMode = DRAG_NODE;
        dragNode = hit;
        dragPos = e.pos;
        e.consume(this);
    }

    void onDragStart(const DragStartEvent& e) override
    {
        OpaqueWidget::onDragStart(e);
    }

    void onDragMove(const DragMoveEvent& e) override
    {
        if (!module || dragMode == DRAG_NONE) return;
        float zoom = getAbsoluteZoom();
        if (zoom <= 0.f) zoom = 1.f;
        dragPos = dragPos.plus(e.mouseDelta.div(zoom));

        if (dragMode == DRAG_PAN)
        {
            scroll -= (double)(e.mouseDelta.x / zoom) * tpp;
            clampScroll();
        }
        else if (dragMode == DRAG_SCROLL)
        {
            scrollFromThumbX(dragPos.x - grabDX);
        }
        else if (dragMode == DRAG_SCRUB)
        {
            module->engine.seekAll(clampT(x2t(dragPos.x)));
        }
        else if (dragMode == DRAG_LOOP)
        {
            double end = clampT(applySnap(x2t(dragPos.x)));
            if (end < LOOP_MIN) end = LOOP_MIN;
            module->loopEndUser = end;
        }
        else if (dragMode == DRAG_BEND)
        {
            // Solve the bend so the curve passes through the POINTER: find b
            // with f_b(frac_p) = u_p. f_b is monotone in b for fixed frac, so
            // bisection is exact enough and cannot diverge.
            int L = laneIdx();
            timeline_dsp::LaneData ld = module->laneCopy(L);
            if (dragSeg >= 0 && dragSeg + 1 < ld.count())
            {
                double t0 = ld.t[dragSeg], t1 = ld.t[dragSeg + 1];
                double v0 = ld.v[dragSeg], v1 = ld.v[dragSeg + 1];
                if (std::fabs(v1 - v0) > 1e-9 && t1 > t0)
                {
                    double fp = (x2t(dragPos.x) - t0) / (t1 - t0);
                    fp = tlClamp(fp, 0.05, 0.95);
                    double u = (y2v(dragPos.y) - v0) / (v1 - v0);
                    u = tlClamp(u, 0.02, 0.98);
                    double lo = -10.0, hi = 10.0;
                    for (int it = 0; it < 40; it++)
                    {
                        double mid = 0.5 * (lo + hi);
                        if (bendFrac(fp, mid) < u) lo = mid; else hi = mid;
                    }
                    double bend = 0.5 * (lo + hi);
                    if (std::fabs(bend) < 0.4) bend = 0.0;   // the detent
                    ld.b[dragSeg] = (float)bend;
                }
            }
            module->publishLane(L, ld);
        }
        else if (dragMode == DRAG_NODE)
        {
            // Edge auto-scroll: dragging a node against either edge pulls the
            // view along, so a node can be carried into empty future space
            // without stopping to zoom out. Rate scales with how far past the
            // edge the pointer is.
            const float EDGE = 24.f;
            float over = 0.f;
            if (dragPos.x > box.size.x - EDGE) over = dragPos.x - (box.size.x - EDGE);
            else if (dragPos.x < EDGE)         over = dragPos.x - EDGE;
            if (over != 0.f)
            {
                breakChase();                  // the view moved, so promote
                                               // the hold to a full break
                double step = (double)tlClamp(over / EDGE, -3.0, 3.0) * 6.0 * tpp;
                scroll += step;
                if (scroll < 0.0) scroll = 0.0;   // no clamp to the right: the
                                                  // extent is elastic
                dragPos.x = tlClamp(dragPos.x, 0.f, box.size.x);
            }
            int L = laneIdx();
            timeline_dsp::LaneData ld = module->laneCopy(L);
            if (dragNode >= 0 && dragNode < ld.count())
            {
                double beat = clampT(applySnap(x2t(dragPos.x)));
                double volt = y2v(dragPos.y);
                ld.t[dragNode] = (float)beat;
                ld.v[dragNode] = (float)volt;
                // Keep the lane sorted; follow the node to its new index.
                ld.resort();
                int at = ld.indexOf(beat, volt);
                if (at >= 0) dragNode = at;
            }
            module->publishLane(L, ld);
        }
        OpaqueWidget::onDragMove(e);
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        if (dragMode == DRAG_NODE) pushGesture("move automation node");
        if (dragMode == DRAG_BEND) pushGesture("bend automation segment");
        dragMode = DRAG_NONE;
        latchedSpan = 0.0;
        chaseHeld = false;                     // the gesture is over
        dragNode = -1;
        dragSeg = -1;
        OpaqueWidget::onDragEnd(e);
    }

    void createContextMenu()
    {
        if (!module) return;
        ui::Menu* menu = createMenu();
        menu->addChild(createMenuLabel(string::f("Lane %d", laneIdx() + 1)));
        TimelineEditorWidget* self = this;
        menu->addChild(createMenuItem("Clear lane", "", [self]() {
            if (!self->module) return;
            self->snapshot();
            timeline_dsp::LaneData empty;
            self->module->publishLane(self->laneIdx(), empty);
            self->pushGesture("clear automation lane");
        }));
        menu->addChild(createMenuItem("Straighten lane", "", [self]() {
            if (!self->module) return;
            self->snapshot();
            int L = self->laneIdx();
            timeline_dsp::LaneData ld = self->module->laneCopy(L);
            int n = ld.count();
            for (int i = 0; i < n; i++) ld.b[i] = 0.f;
            self->module->publishLane(L, ld);
            self->pushGesture("straighten automation lane");
        }));
        // Both of these place the view deliberately, so they break the chase.
        menu->addChild(createMenuItem("Fit view to content", "", [self]() {
            if (!self->module) return;
            double last = self->module->lastBeat();
            if (last <= 0.0) last = 16.0;
            self->breakChase();
            self->scroll = 0.0;
            self->tpp = tlClamp(last * 1.05 / (double)self->box.size.x, TPP_MIN, TPP_MAX);
        }));
        menu->addChild(createMenuItem("Reset zoom", "", [self]() {
            self->breakChase();
            self->scroll = 0.0;
            self->tpp = 0.05;
        }));
    }

    // ── drawing ────────────────────────────────────────────────────────────
    void draw(const DrawArgs& args) override
    {
        NVGcontext* vg = args.vg;
        float w = box.size.x, h = box.size.y;
        float top = bodyTop(), bot = bodyBot();

        updateChase();          // may move `scroll`; must run before anything
                                // maps beats to pixels

        // Ground.
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, w, h);
        nvgFillColor(vg, tcol(0x0e1117));
        nvgFill(vg);

        // Ruler + pan strip grounds.
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, w, RULER_H);
        nvgRect(vg, 0, bot, w, PANSTRIP_H);
        nvgFillColor(vg, tcol(0x14181d));
        nvgFill(vg);

        drawGrid(vg, w, top, bot);
        drawLoop(vg, w, top, bot);
        drawCurve(vg, w, top, bot);
        drawPlayhead(vg, h, top, bot);
        drawChaseTick(vg, w);
        drawScrollbar(vg, w, bot);

        if (module && module->locked)
        {
            nvgBeginPath(vg);
            nvgRect(vg, 0, 0, w, h);
            nvgFillColor(vg, nvgRGBA(180, 30, 30, 25));
            nvgFill(vg);
        }

        // Frame.
        nvgBeginPath(vg);
        nvgRect(vg, 0.5f, 0.5f, w - 1.f, h - 1.f);
        nvgStrokeColor(vg, tcol(0x2b333d));
        nvgStrokeWidth(vg, 1.f);
        nvgStroke(vg);

        OpaqueWidget::draw(args);
    }

    // Choose a bar/beat step that keeps gridlines readable at this zoom.
    double niceStep()
    {
        static const double STEPS[14] = {
            0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 4096 };
        double minPx = 48.0;
        for (int i = 0; i < 14; i++)
            if (STEPS[i] / tpp >= minPx) return STEPS[i];
        return STEPS[13];
    }

    void drawGrid(NVGcontext* vg, float w, float top, float bot)
    {
        double step = niceStep();
        double t0 = std::floor(scroll / step) * step;
        std::shared_ptr<Font> font =
            APP->window->loadFont(asset::system("res/fonts/ShareTechMono-Regular.ttf"));

        nvgBeginPath(vg);
        for (double t = t0; ; t += step)
        {
            float x = t2x(t);
            if (x > w) break;
            if (x < -1.f) continue;
            nvgMoveTo(vg, std::floor(x) + 0.5f, top);
            nvgLineTo(vg, std::floor(x) + 0.5f, bot);
        }
        nvgStrokeColor(vg, tcol(0xffffff, 0.05f));
        nvgStrokeWidth(vg, 1.f);
        nvgStroke(vg);

        // Ruler ticks + bar numbers.
        if (font)
        {
            nvgFontFaceId(vg, font->handle);
            nvgFontSize(vg, 8.f);
            nvgFillColor(vg, tcol(0x8b98a5));
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            for (double t = t0; ; t += step)
            {
                float x = t2x(t);
                if (x > w) break;
                if (x < -1.f) continue;
                char buf[32];
                double bar = t / BAR;
                if (step >= BAR) std::snprintf(buf, sizeof(buf), "%d", (int)std::floor(bar) + 1);
                else std::snprintf(buf, sizeof(buf), "%d.%d",
                                   (int)std::floor(bar) + 1,
                                   (int)std::floor(std::fmod(t, BAR)) + 1);
                nvgText(vg, x + 3.f, RULER_H * 0.5f, buf, NULL);
            }
        }

        // Voltage guides: +/-5 V hairlines, 0 V centre.
        nvgBeginPath(vg);
        nvgMoveTo(vg, 0, std::floor(v2y(5.0)) + 0.5f);
        nvgLineTo(vg, w, std::floor(v2y(5.0)) + 0.5f);
        nvgMoveTo(vg, 0, std::floor(v2y(-5.0)) + 0.5f);
        nvgLineTo(vg, w, std::floor(v2y(-5.0)) + 0.5f);
        nvgStrokeColor(vg, tcol(0xffffff, 0.05f));
        nvgStroke(vg);
        nvgBeginPath(vg);
        nvgMoveTo(vg, 0, std::floor(v2y(0.0)) + 0.5f);
        nvgLineTo(vg, w, std::floor(v2y(0.0)) + 0.5f);
        nvgStrokeColor(vg, tcol(0xffffff, 0.16f));
        nvgStroke(vg);
    }

    void drawLoop(NVGcontext* vg, float w, float top, float bot)
    {
        if (!module || module->params[Timeline::LOOP_PARAM].getValue() < 0.5f) return;
        float lx = t2x(module->effectiveLoopEnd());
        if (lx < w)
        {
            nvgBeginPath(vg);
            nvgRect(vg, lx, top, w - lx, bot - top);
            nvgFillColor(vg, tcol(0x000000, 0.30f));
            nvgFill(vg);
        }
        nvgBeginPath(vg);
        nvgMoveTo(vg, lx, 0);
        nvgLineTo(vg, lx, bot);
        nvgStrokeColor(vg, tcol(0xe3b341));
        nvgStrokeWidth(vg, 1.5f);
        nvgStroke(vg);
        // The grab handle, in the ruler.
        nvgBeginPath(vg);
        nvgRect(vg, lx - 3.f, 1.f, 6.f, RULER_H - 2.f);
        nvgFillColor(vg, tcol(0xe3b341));
        nvgFill(vg);
    }

    void drawCurve(NVGcontext* vg, float w, float top, float bot)
    {
        if (!module) return;
        const timeline_dsp::LaneData& ld = module->lane(laneIdx());
        int n = ld.count();

        nvgSave(vg);
        nvgScissor(vg, 0, top, w, bot - top);

        if (n == 0)
        {
            // An empty lane reads 0 V across the whole span.
            nvgBeginPath(vg);
            nvgMoveTo(vg, 0, v2y(0.0));
            nvgLineTo(vg, w, v2y(0.0));
            nvgStrokeColor(vg, tcol(0x39ff14, 0.45f));
            nvgStrokeWidth(vg, 1.5f);
            nvgStroke(vg);
            nvgRestore(vg);
            return;
        }

        // The curve: hold before the first node, each segment drawn with its
        // bend (straight segments as single lines, bent ones subdivided),
        // hold after the last.
        // With thousands of recorded nodes, only the visible ones need
        // stroking: skip nodes left of the view except the last one before
        // it, and stop after the first one past the right edge.
        nvgBeginPath(vg);
        nvgMoveTo(vg, 0, v2y(ld.v[0]));
        nvgLineTo(vg, t2x(ld.t[0]), v2y(ld.v[0]));
        int first = 0;
        while (first + 1 < n && t2x(ld.t[first + 1]) < 0.f) first++;
        if (first > 0) nvgLineTo(vg, t2x(ld.t[first]), v2y(ld.v[first]));
        for (int i = first; i + 1 < n; i++)
        {
            float x0 = t2x(ld.t[i]), x1 = t2x(ld.t[i + 1]);
            if (ld.b[i] == 0.f)
            {
                nvgLineTo(vg, x1, v2y(ld.v[i + 1]));
            }
            else
            {
                int steps = (int)tlClamp((x1 - x0) / 4.0, 8.0, 48.0);
                for (int s = 1; s <= steps; s++)
                {
                    double frac = (double)s / steps;
                    double fb = bendFrac(frac, ld.b[i]);
                    nvgLineTo(vg, x0 + (x1 - x0) * (float)frac,
                              v2y(ld.v[i] + fb * (ld.v[i + 1] - ld.v[i])));
                }
            }
            if (x1 > w) break;
        }
        if (t2x(ld.t[n - 1]) < w) nvgLineTo(vg, w, v2y(ld.v[n - 1]));
        nvgStrokeColor(vg, tcol(0x39ff14));
        nvgStrokeWidth(vg, 1.5f);
        nvgLineJoin(vg, NVG_ROUND);
        nvgStroke(vg);

        // Nodes.
        for (int i = 0; i < n; i++)
        {
            float x = t2x(ld.t[i]), y = v2y(ld.v[i]);
            if (x < -8.f) continue;
            if (x > w + 8.f) break;
            bool hot = (i == hoverNode) || (dragMode == DRAG_NODE && i == dragNode);
            nvgBeginPath(vg);
            nvgCircle(vg, x, y, NODE_R + (hot ? 1.5f : 0.f));
            nvgFillColor(vg, hot ? tcol(0xc8ffb0) : tcol(0x39ff14));
            nvgFill(vg);
            nvgStrokeColor(vg, tcol(0x0b0d10));
            nvgStrokeWidth(vg, 1.f);
            nvgStroke(vg);
        }

        // The bend handle: a diamond at the hovered segment's midpoint. A
        // different shape from the round nodes, so it reads as a different
        // kind of thing.
        {
            int seg = (dragMode == DRAG_BEND) ? dragSeg : hoverSeg;
            if (seg >= 0 && seg + 1 < n && !(module && module->locked))
            {
                // While dragging, the handle rides the curve AT THE POINTER,
                // since that is the point the solve pins the curve through.
                Vec hp = bendHandlePos(seg);
                if (dragMode == DRAG_BEND)
                {
                    float hx = tlClamp(dragPos.x,
                                       t2x(ld.t[seg]) + 4.f,
                                       t2x(ld.t[seg + 1]) - 4.f);
                    hp = Vec(hx, segCurveY(ld, seg, hx));
                }
                bool hot = (dragMode == DRAG_BEND) || bendHandleSeg(mousePos) >= 0;
                float r = HANDLE_R + (hot ? 1.f : 0.f);
                nvgBeginPath(vg);
                nvgMoveTo(vg, hp.x, hp.y - r);
                nvgLineTo(vg, hp.x + r, hp.y);
                nvgLineTo(vg, hp.x, hp.y + r);
                nvgLineTo(vg, hp.x - r, hp.y);
                nvgClosePath(vg);
                nvgFillColor(vg, hot ? tcol(0xc8ffb0) : tcol(0x39ff14, 0.9f));
                nvgFill(vg);
                nvgStrokeColor(vg, tcol(0x0b0d10));
                nvgStrokeWidth(vg, 1.f);
                nvgStroke(vg);
            }
        }
        nvgRestore(vg);
    }

    void drawPlayhead(NVGcontext* vg, float h, float top, float bot)
    {
        if (!module) return;
        float x = t2x(selLane().playhead);
        if (x < 0.f || x > box.size.x) return;
        nvgBeginPath(vg);
        nvgMoveTo(vg, x, 0);
        nvgLineTo(vg, x, bot);
        nvgStrokeColor(vg, tcol(0xff5d5d));
        nvgStrokeWidth(vg, 1.f);
        nvgStroke(vg);
    }

    // The chase anchor: a short tick in the ruler at the centre, drawn only
    // while chase is actually following. It disappears the moment chase
    // suspends, which is how the user sees WHY the view stopped moving.
    void drawChaseTick(NVGcontext* vg, float w)
    {
        if (!chaseArmed()) return;
        float cx = std::floor(w * 0.5f) + 0.5f;
        nvgBeginPath(vg);
        nvgMoveTo(vg, cx, 0.f);
        nvgLineTo(vg, cx, RULER_H);
        nvgStrokeColor(vg, tcol(0xff5d5d, 0.55f));
        nvgStrokeWidth(vg, 1.f);
        nvgStroke(vg);
    }

    // A conventional scrollbar: the thumb's LENGTH is how much of the content
    // fits on screen, its position is where in the content you are.
    void drawScrollbar(NVGcontext* vg, float w, float bot)
    {
        nvgBeginPath(vg);
        nvgMoveTo(vg, 0, bot + 0.5f);
        nvgLineTo(vg, w, bot + 0.5f);
        nvgStrokeColor(vg, tcol(0xffffff, 0.06f));
        nvgStrokeWidth(vg, 1.f);
        nvgStroke(vg);

        float tx = thumbX(), tw = thumbW();
        float pad = 2.f;
        float r = (PANSTRIP_H - 2.f * pad) * 0.5f;
        bool hot = hoverThumb || dragMode == DRAG_SCROLL;
        nvgBeginPath(vg);
        nvgRoundedRect(vg, tx + pad, bot + pad, tw - 2.f * pad,
                       PANSTRIP_H - 2.f * pad, r);
        nvgFillColor(vg, hot ? tcol(0x8b98a5, 0.85f) : tcol(0x8b98a5, 0.45f));
        nvgFill(vg);
    }

};

// ── the lane tab strip ─────────────────────────────────────────────────────
// 16 tabs; a tab holding drawn nodes reads brighter, so you can see where you
// have already worked (the web version's bank/lane tabs, flattened).
struct TimelineLaneTabs : OpaqueWidget
{
    Timeline* module = NULL;

    void onButton(const ButtonEvent& e) override
    {
        if (module && module->locked) return;      // frozen with the editor
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && module)
        {
            float tw = box.size.x / (float)timeline_dsp::TL_LANES;
            int idx = (int)(e.pos.x / tw);
            if (idx >= 0 && idx < timeline_dsp::TL_LANES)
                module->paramQuantities[Timeline::LANE_PARAM]->setValue((float)idx);
            e.consume(this);
            return;
        }
        OpaqueWidget::onButton(e);
    }

    void draw(const DrawArgs& args) override
    {
        NVGcontext* vg = args.vg;
        float tw = box.size.x / (float)timeline_dsp::TL_LANES;
        int cur = module ? module->currentLane() : 0;
        std::shared_ptr<Font> font =
            APP->window->loadFont(asset::system("res/fonts/ShareTechMono-Regular.ttf"));

        for (int i = 0; i < timeline_dsp::TL_LANES; i++)
        {
            float x = (float)i * tw;
            bool drawn = module && !module->lane(i).empty();
            bool sel = (i == cur);
            nvgBeginPath(vg);
            nvgRect(vg, x + 1.f, 1.f, tw - 2.f, box.size.y - 2.f);
            nvgFillColor(vg, sel ? tcol(0x39ff14, 0.85f)
                                 : (drawn ? tcol(0x2f6b2a) : tcol(0x1b222b)));
            nvgFill(vg);
            nvgStrokeColor(vg, tcol(0x2b333d));
            nvgStrokeWidth(vg, 1.f);
            nvgStroke(vg);
            // Transport dot: with independent transports, the tabs are the
            // only place all 16 states are visible at once. Green = running,
            // red = stopped — always drawn, so the state is readable either
            // way. (On the selected tab's bright green ground, the running
            // dot is dark green rather than green-on-green.)
            if (module)
            {
                bool run = module->engine.lanes[i].playing;
                nvgBeginPath(vg);
                nvgCircle(vg, x + tw - 4.f, 4.f, 1.6f);
                nvgFillColor(vg, run ? (sel ? tcol(0x0b3d0b) : tcol(0x39ff14))
                                     : tcol(0xff5d5d));
                nvgFill(vg);
            }
            if (font)
            {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "%d", i + 1);
                nvgFontFaceId(vg, font->handle);
                nvgFontSize(vg, 8.f);
                nvgFillColor(vg, sel ? tcol(0x0b0d10) : tcol(0x8b98a5));
                nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
                nvgText(vg, x + tw * 0.5f, box.size.y * 0.5f, buf, NULL);
            }
        }
    }
};

// ── the bar/beat + time readout ────────────────────────────────────────────
struct TimelineReadout : TransparentWidget
{
    Timeline* module = NULL;

    void draw(const DrawArgs& args) override
    {
        NVGcontext* vg = args.vg;
        std::shared_ptr<Font> font =
            APP->window->loadFont(asset::system("res/fonts/ShareTechMono-Regular.ttf"));
        if (!font) return;
        double beat = module ? module->engine.lanes[module->currentLane()].playhead : 0.0;
        double bpm = module ? module->engine.bpm : 120.0;
        int bar = (int)std::floor(beat / BAR) + 1;
        int bt = (int)std::floor(std::fmod(beat, BAR)) + 1;
        double sec = beat * 60.0 / (bpm > 0.0 ? bpm : 120.0);
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%03d.%d   %d:%05.2f", bar, bt,
                      (int)(sec / 60.0), std::fmod(sec, 60.0));
        // A dim backing plate: the readout sits OVER the curve now, and green
        // on green is unreadable when a lane passes behind it.
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 2.f);
        nvgFillColor(vg, tcol(0x0e1117, 0.72f));
        nvgFill(vg);

        // Right-aligned: it sits at the editor's bottom-right corner, so the
        // digits must grow leftward and keep the right edge steady.
        nvgFontFaceId(vg, font->handle);
        nvgFontSize(vg, 11.f);
        nvgFillColor(vg, tcol(0x39ff14, 0.85f));
        nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        nvgText(vg, box.size.x - 4.f, box.size.y * 0.5f, buf, NULL);

        // The selected lane hit the node cap during a take: a small red
        // notice at the readout's left, until the next take or edit on it.
        if (module && module->laneFull[module->currentLane()])
        {
            nvgFontSize(vg, 8.f);
            nvgFillColor(vg, tcol(0xff5d5d));
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            nvgText(vg, 4.f, box.size.y * 0.5f, "LANE FULL", NULL);
        }
    }
};

} // namespace timeline_ui
