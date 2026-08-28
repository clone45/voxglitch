// scrollbar_test.cpp — the editor's scrollbar geometry, mirrored (the editor
// itself needs Rack). Copy of TimelineEditorWidget's viewSpan/contentEnd/
// contentSpan/maxScroll/thumbW/thumbX/scrollFromThumbX plus the drag latch —
// re-sync if those change.
#include <cstdio>
#include <cmath>
#include "TimelineEngine.hpp"

static int fails = 0;
static void check(const char* w, bool ok, double g = 0, double n = 0)
{
    if (ok) { printf("  ok    %s\n", w); return; }
    printf("  FAIL  %s   (got %.6f want %.6f)\n", w, g, n); fails++;
}
static double tlClamp(double x, double lo, double hi)
{ return x < lo ? lo : (x > hi ? hi : x); }

static const float THUMB_MIN = 24.f;

struct Bar
{
    double tpp = 0.05, scroll = 0.0;
    float boxW = 700.f;
    double contentLast = 0.0;          // furthest node / loop end / playhead
    bool dragging = false;             // DRAG_SCROLL in the editor
    double latchedSpan = 0.0;

    double viewSpan() { return (double)boxW * tpp; }
    double contentEnd() { return contentLast; }

    // ELASTIC: always one screen of future beyond content-or-view, latched
    // while the thumb is dragged.
    double contentSpan()
    {
        if (dragging && latchedSpan > 0.0) return latchedSpan;
        double v = viewSpan();
        // NB: not `far` — MinGW still defines it as a DOS memory-model keyword.
        double reach = contentEnd();
        double viewEnd = scroll + v;
        if (viewEnd > reach) reach = viewEnd;
        double span = reach + v;
        return span > v ? span : v;
    }
    double maxScroll() { double m = contentSpan() - viewSpan(); return m > 0 ? m : 0; }
    void clampScroll() { scroll = tlClamp(scroll, 0.0, maxScroll()); }
    float thumbW()
    {
        float w = (float)(viewSpan() / contentSpan()) * boxW;
        if (w < THUMB_MIN) w = THUMB_MIN;
        if (w > boxW) w = boxW;
        return w;
    }
    float thumbX()
    {
        double m = maxScroll();
        if (m <= 0.0) return 0.f;
        return (float)(scroll / m) * (boxW - thumbW());
    }
    void scrollFromThumbX(float x)
    {
        float travel = boxW - thumbW();
        if (travel <= 0.f) { scroll = 0.0; return; }
        scroll = tlClamp((double)(x / travel) * maxScroll(), 0.0, maxScroll());
    }
    void grab() { latchedSpan = contentSpan(); dragging = true; }
    void release() { dragging = false; latchedSpan = 0.0; }
};

int main()
{
    printf("Empty timeline: the future is still reachable\n");
    {
        Bar b;                                  // 35 beats visible, nothing drawn
        check("scroll range exists even with no content", b.maxScroll() > 0.0,
              b.maxScroll(), 1.0);
        b.scroll = b.maxScroll();
        check("scrolling to the end opens MORE range (no wall)",
              b.maxScroll() > b.scroll, b.maxScroll(), b.scroll);
        check("thumb stays inside the track",
              b.thumbX() + b.thumbW() <= b.boxW + 0.5f,
              b.thumbX() + b.thumbW(), b.boxW);
    }

    printf("\nThe wall is gone: repeated scrolling always advances\n");
    {
        Bar b;
        double prev = -1.0;
        bool alwaysGrew = true;
        for (int i = 0; i < 40; i++)
        {
            b.scroll = b.maxScroll();          // jam right, repeatedly
            b.clampScroll();
            if (b.scroll <= prev) alwaysGrew = false;
            prev = b.scroll;
        }
        check("40 jams right keep advancing", alwaysGrew, b.scroll, 0);
        check("reached far past any content", b.scroll > 500.0, b.scroll, 500.0);
    }

    printf("\nContent longer than the view\n");
    {
        Bar b; b.contentLast = 350.0;
        float tw = b.thumbW();
        check("thumb shrinks in proportion", tw > THUMB_MIN && tw < b.boxW * 0.25f, tw, 0);
        check("content is reachable", b.maxScroll() >= 350.0, b.maxScroll(), 350.0);
        b.scrollFromThumbX(-50.f);
        check("dragging past the start clamps at 0", b.scroll == 0.0, b.scroll, 0.0);
    }

    printf("\nDrag latch keeps the thumb under the pointer\n");
    {
        Bar b; b.contentLast = 350.0;
        b.grab();
        double spanAtGrab = b.contentSpan();
        float tw = b.thumbW();
        bool stable = true, roundTrip = true;
        for (float x = 0.f; x <= b.boxW - tw; x += 5.f)
        {
            b.scrollFromThumbX(x);
            if (b.contentSpan() != spanAtGrab) stable = false;      // latched
            if (b.thumbW() != tw) stable = false;                   // size fixed
            if (std::fabs(b.thumbX() - x) > 0.6f) roundTrip = false; // no rubber-band
        }
        check("extent stays latched through the drag", stable);
        check("thumb tracks the pointer 1:1", roundTrip);
        b.release();
        check("after release the extent grows again", b.contentSpan() > spanAtGrab,
              b.contentSpan(), spanAtGrab);
    }

    printf("\nThumb never vanishes on a very long song\n");
    {
        Bar b; b.contentLast = 86400.0;
        check("thumb keeps its minimum", b.thumbW() >= THUMB_MIN, b.thumbW(), THUMB_MIN);
        b.scroll = b.maxScroll();
        check("still inside the track at the far end",
              b.thumbX() + b.thumbW() <= b.boxW + 0.5f,
              b.thumbX() + b.thumbW(), b.boxW);
    }

    printf("\nPaging\n");
    {
        Bar b; b.contentLast = 350.0;
        double page = b.viewSpan() * 0.9;
        b.scroll = 0.0;
        b.scroll += page; b.clampScroll();
        check("a page forward moves ~90%% of a screen",
              std::fabs(b.scroll - page) < 1e-9, b.scroll, page);
        b.scroll -= page; b.clampScroll();
        check("a page back returns to the start", b.scroll == 0.0, b.scroll, 0.0);
    }

    printf("\nZoom keeps scroll legal\n");
    {
        Bar b; b.contentLast = 350.0;
        b.scroll = b.maxScroll();
        b.tpp *= 8.0;
        b.clampScroll();
        check("scroll stays within range after zooming out",
              b.scroll <= b.maxScroll() + 1e-9, b.scroll, b.maxScroll());
        check("thumb stays inside the track",
              b.thumbX() + b.thumbW() <= b.boxW + 0.5f, b.thumbX() + b.thumbW(), b.boxW);
    }

    printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "ALL PASS", fails, fails == 1 ? "" : "s");
    return fails ? 1 : 0;
}
