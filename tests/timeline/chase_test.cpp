// chase_test.cpp — the chase scroll maths, mirrored (the editor needs Rack).
// Copy of TimelineEditorWidget::updateChase and the suspend rules — re-sync
// if those change. Design: docs/implementation_plans/timeline-chase-design.md.
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

struct Chase
{
    double tpp = 0.05, scroll = 0.0;
    float boxW = 700.f;
    bool chaseEnabled = true, chaseHeld = false, chaseBroken = false;
    bool chaseLeftView = false;
    double lastPlayhead = 0.0;
    int lastSeenSeek = 0;

    // stand-ins for the engine
    double playhead = 0.0;
    int seekSerial = 0;

    double viewSpan() { return (double)boxW * tpp; }
    bool armed() { return chaseEnabled && !chaseHeld && !chaseBroken; }
    void breakChase() { chaseBroken = true; chaseLeftView = false; }

    void updateChase()
    {
        double ph = playhead;
        if (ph < lastPlayhead || seekSerial != lastSeenSeek) chaseBroken = false;
        lastSeenSeek = seekSerial;
        lastPlayhead = ph;
        if (chaseBroken)
        {
            bool inside = (ph >= scroll && ph <= scroll + viewSpan());
            if (!inside) chaseLeftView = true;
            else if (chaseLeftView) { chaseBroken = false; chaseLeftView = false; }
        }
        if (!armed()) return;
        double want = ph - viewSpan() * 0.5;
        scroll = (want > 0.0) ? want : 0.0;
    }

    // Where the playhead sits on screen, in pixels.
    float playheadX() { return (float)((playhead - scroll) / tpp); }
};

int main()
{
    printf("Phase 1: the playhead crosses the view before anything scrolls\n");
    {
        Chase c;                                  // 35 beats visible
        double centreBeat = c.viewSpan() * 0.5;   // 17.5
        bool scrolled = false;
        for (double ph = 0.0; ph < centreBeat - 0.01; ph += 0.05)
        {
            c.playhead = ph; c.updateChase();
            if (c.scroll != 0.0) scrolled = true;
        }
        check("no scrolling before the centre", !scrolled, c.scroll, 0.0);
        c.playhead = 0.0; c.scroll = 0.0; c.updateChase();
        check("playhead starts at the left edge", std::fabs(c.playheadX()) < 0.5f,
              c.playheadX(), 0.0);
        c.playhead = centreBeat; c.updateChase();
        check("reaches the centre of the view",
              std::fabs(c.playheadX() - c.boxW * 0.5f) < 1.f,
              c.playheadX(), c.boxW * 0.5f);
    }

    printf("\nPhase 2: past the centre the playhead is pinned\n");
    {
        Chase c;
        float want = c.boxW * 0.5f;
        bool pinned = true;
        for (double ph = c.viewSpan() * 0.5; ph < 400.0; ph += 0.037)
        {
            c.playhead = ph; c.updateChase();
            if (std::fabs(c.playheadX() - want) > 1.f) pinned = false;
        }
        check("stays centred within a pixel over 400 beats", pinned,
              c.playheadX(), want);
        check("the scroll followed", c.scroll > 380.0, c.scroll, 380.0);
    }

    printf("\nZoom keeps it centred\n");
    {
        Chase c; c.playhead = 100.0; c.updateChase();
        float want = c.boxW * 0.5f;
        bool ok = true;
        double zooms[6] = { 0.002, 0.01, 0.05, 0.5, 4.0, 64.0 };
        for (int i = 0; i < 6; i++)
        {
            c.tpp = zooms[i];
            c.updateChase();
            // Zoomed far out the playhead may sit before the centre; that is
            // phase 1 behaviour and correct.
            bool centred = std::fabs(c.playheadX() - want) < 1.f;
            bool phase1 = (c.scroll == 0.0 && c.playhead < c.viewSpan() * 0.5);
            if (!centred && !phase1) ok = false;
        }
        check("centred at every zoom level, or legitimately in phase 1", ok);
    }

    printf("\nLoop wrap and rewind\n");
    {
        Chase c; c.playhead = 200.0; c.updateChase();
        check("scrolled out to the playhead", c.scroll > 180.0, c.scroll, 180.0);
        c.playhead = 0.0;                       // the wrap: no serial bump
        c.updateChase();
        check("a wrap returns the view to the start", c.scroll == 0.0, c.scroll, 0.0);
        c.playhead = 200.0; c.updateChase();
        c.chaseBroken = true;                   // user had panned away
        c.playhead = 0.0; c.updateChase();      // now it wraps
        check("a wrap re-arms a broken chase", !c.chaseBroken);
    }

    printf("\nSuspension\n");
    {
        Chase c; c.playhead = 200.0; c.updateChase();
        double parked = c.scroll;
        c.breakChase();
        c.playhead = 210.0; c.updateChase();
        check("a broken chase does not move the view", c.scroll == parked,
              c.scroll, parked);

        // The regression this test caught: nudging the view while the
        // playhead is ON SCREEN must not re-arm on the next frame.
        Chase n; n.playhead = 200.0; n.updateChase();
        n.breakChase();
        n.scroll -= 2.0;                        // a small deliberate nudge
        for (int i = 0; i < 20; i++) { n.playhead += 0.05; n.updateChase(); }
        check("a nudge with the playhead visible stays broken", n.chaseBroken);

        // Self-healing: the music catches up to where the user is looking.
        Chase d; d.playhead = 100.0; d.updateChase();
        d.breakChase();
        d.scroll = 300.0;                       // user panned ahead
        d.playhead = 101.0; d.updateChase();    // playhead now off screen
        d.playhead = 305.0;                     // music arrives in view
        d.updateChase();
        check("chase re-arms when the playhead enters the view", !d.chaseBroken);
        check("and it centres again on the next frame",
              std::fabs(d.playheadX() - d.boxW * 0.5f) < 1.f,
              d.playheadX(), d.boxW * 0.5f);

        // A held chase (a drag in progress) must not move the view.
        Chase h; h.playhead = 200.0; h.updateChase();
        double held = h.scroll;
        h.chaseHeld = true;
        h.playhead = 260.0; h.updateChase();
        check("a held chase does not move the view during a drag",
              h.scroll == held, h.scroll, held);
        h.chaseHeld = false; h.updateChase();
        check("release resumes centring",
              std::fabs(h.playheadX() - h.boxW * 0.5f) < 1.f,
              h.playheadX(), h.boxW * 0.5f);
    }

    printf("\nA seek re-arms too\n");
    {
        Chase c; c.playhead = 200.0; c.updateChase();
        c.breakChase();
        c.playhead = 400.0; c.seekSerial++;     // a forward seek
        c.updateChase();
        check("a seek clears the break", !c.chaseBroken);
    }

    printf("\nChase never drives the scroll negative\n");
    {
        Chase c;
        bool ok = true;
        for (double ph = 0.0; ph < 40.0; ph += 0.01)
        {
            c.playhead = ph; c.updateChase();
            if (c.scroll < 0.0) ok = false;
        }
        check("scroll stays at or above zero", ok, c.scroll, 0.0);
    }

    printf("\nChase stays inside the elastic extent\n");
    {
        // extent = max(content, viewEnd) + view. Chase puts viewEnd at
        // playhead + view/2, so maxScroll always exceeds the chased scroll.
        Chase c;
        bool ok = true;
        for (double ph = 0.0; ph < 500.0; ph += 0.5)
        {
            c.playhead = ph; c.updateChase();
            double v = c.viewSpan();
            double reach = c.playhead;             // content = the playhead
            double viewEnd = c.scroll + v;
            if (viewEnd > reach) reach = viewEnd;
            double maxScroll = (reach + v) - v;
            if (c.scroll > maxScroll + 1e-9) ok = false;
        }
        check("chase never exceeds maxScroll", ok);
    }

    printf("\nDisabled chase does nothing\n");
    {
        Chase c; c.chaseEnabled = false;
        c.scroll = 12.0;
        c.playhead = 500.0; c.updateChase();
        check("the view stays where the user left it", c.scroll == 12.0,
              c.scroll, 12.0);
    }

    printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "ALL PASS", fails,
           fails == 1 ? "" : "s");
    return fails ? 1 : 0;
}
