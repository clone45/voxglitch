#pragma once
//
// Spectral Granulator — the display.
//
// What the analysis hears, so FOCUS / WIDTH / SENS can be set by eye:
//
//   * the input's spectrum, 64 log-spaced columns (30 Hz .. 0.45 x the sample
//     rate), -72..0 dB. Columns inside the FOCUS / WIDTH band are bright, the
//     rest dim: the bright part is what ONSET, LEVEL, BRIGHT and TONAL hear.
//   * under it, the onset bar: the detector's value against its threshold.
//     The tick in the middle IS the threshold; the bar crossing it fires an
//     onset and the bar flashes. SENS moves the threshold, so it changes how
//     far the same material reaches.
//   * right, three meters: L (level), B (bright), T (tonal), the smoothed
//     0..1 signals that leave the output jacks.
//
// The plate is dark in both themes (the `_display` rule in the panel
// generator), so the colours are fixed. Sized from the panel's
// `spectrum_display` rect. Without a module (the browser preview) it draws a
// fixed, plausible spectrum.
//
// Reads spectral_granulator::Display and the module's meter floats while the audio
// thread writes them: a torn read is one stale frame.
//

struct SpectralGranulatorDisplay : TransparentWidget
{
    SpectralGranulator* module = NULL;
    uint32_t last_serial = 0;
    float flash = 0.f;

    static constexpr unsigned int COL_DIM    = 0x3a5a60;
    static constexpr unsigned int COL_BRIGHT = 0x7fd6c9;
    static constexpr unsigned int COL_HIT    = 0xffb347;
    static constexpr unsigned int COL_TRACK  = 0x2a2a2e;
    static constexpr unsigned int COL_TEXT   = 0x9a9aa0;

    SpectralGranulatorDisplay(SpectralGranulator* m, rack::Rect r)
    {
        module = m;
        box = r;
    }

    static NVGcolor colour(unsigned int rgb, float alpha = 1.f)
    {
        return nvgRGBA((unsigned char)((rgb >> 16) & 0xffu),
                       (unsigned char)((rgb >> 8) & 0xffu),
                       (unsigned char)(rgb & 0xffu),
                       (unsigned char)(rack::math::clamp(alpha, 0.f, 1.f) * 255.f + 0.5f));
    }

    static NVGcolor mix(unsigned int a, unsigned int b, float t)
    {
        NVGcolor ca = colour(a), cb = colour(b);
        return nvgLerpRGBA(ca, cb, rack::math::clamp(t, 0.f, 1.f));
    }

    void drawLayer(const DrawArgs& args, int layer) override
    {
        if (layer == 1) drawContents(args);
        Widget::drawLayer(args, layer);
    }

    void drawContents(const DrawArgs& args)
    {
        NVGcontext* vg = args.vg;
        const float pad = 5.f;
        const float meters_w = 34.f;                       // three 6 px bars, 5 px apart, and their letters
        const float bar_h = 4.f;
        const float sx = pad, sy = pad;
        const float sw = box.size.x - 2.f * pad - meters_w;
        const float sh = box.size.y - 2.f * pad - bar_h - 4.f;

        // What to draw: the engine's display, or a fixed curve for the browser.
        float col[spectral_granulator::DISPLAY_COLS];
        float weight[spectral_granulator::DISPLAY_COLS];
        float flux_frac = 0.3f;
        float meters[3] = { 0.6f, 0.45f, 0.7f };
        if (module)
        {
            const spectral_granulator::Display& d = module->engine.display;
            for (int i = 0; i < spectral_granulator::DISPLAY_COLS; i++) { col[i] = d.col[i]; weight[i] = d.weight[i]; }
            flux_frac = d.threshold > 0.f ? d.flux / (2.f * d.threshold) : 0.f;
            meters[0] = module->meter_level;
            meters[1] = module->meter_bright;
            meters[2] = module->meter_tonal;
            const uint32_t serial = d.onsetSerial;
            if (serial != last_serial) { last_serial = serial; flash = 1.f; }
        }
        else
        {
            for (int i = 0; i < spectral_granulator::DISPLAY_COLS; i++)
            {
                const float x = (float)i / (float)(spectral_granulator::DISPLAY_COLS - 1);
                col[i] = rack::math::clamp(0.75f - 0.5f * x + 0.18f * std::sin(x * 19.f) * (1.f - x), 0.05f, 1.f);
                weight[i] = (x > 0.2f && x < 0.7f) ? 1.f : 0.f;
            }
        }
        flash = std::max(0.f, flash - (float)APP->window->getLastFrameDuration() * 6.f);

        // Spectrum.
        const float cw = sw / (float)spectral_granulator::DISPLAY_COLS;
        for (int i = 0; i < spectral_granulator::DISPLAY_COLS; i++)
        {
            const float h = std::max(1.f, rack::math::clamp(col[i], 0.f, 1.f) * sh);
            nvgBeginPath(vg);
            nvgRect(vg, sx + (float)i * cw, sy + sh - h, cw - 0.6f, h);
            nvgFillColor(vg, mix(COL_DIM, COL_BRIGHT, weight[i]));
            nvgFill(vg);
        }

        // Onset bar: the tick at the middle is the threshold.
        const float by = sy + sh + 4.f;
        nvgBeginPath(vg);
        nvgRect(vg, sx, by, sw, bar_h);
        nvgFillColor(vg, colour(COL_TRACK));
        nvgFill(vg);
        nvgBeginPath(vg);
        nvgRect(vg, sx, by, sw * rack::math::clamp(flux_frac, 0.f, 1.f), bar_h);
        nvgFillColor(vg, mix(COL_BRIGHT, COL_HIT, flash));
        nvgFill(vg);
        nvgBeginPath(vg);
        nvgRect(vg, sx + 0.5f * sw - 0.5f, by - 1.5f, 1.f, bar_h + 3.f);
        nvgFillColor(vg, colour(0xffffff, 0.8f));
        nvgFill(vg);

        // Meters.
        std::shared_ptr<Font> font = APP->window->loadFont(
            asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
        static const char* const LETTERS[3] = { "L", "B", "T" };
        const float mx = sx + sw + 7.f;
        const float mh = box.size.y - 2.f * pad - 9.f;
        for (int i = 0; i < 3; i++)
        {
            const float x = mx + (float)i * 10.f;
            nvgBeginPath(vg);
            nvgRect(vg, x, sy, 6.f, mh);
            nvgFillColor(vg, colour(COL_TRACK));
            nvgFill(vg);
            const float h = rack::math::clamp(meters[i], 0.f, 1.f) * mh;
            nvgBeginPath(vg);
            nvgRect(vg, x, sy + mh - h, 6.f, h);
            nvgFillColor(vg, colour(COL_BRIGHT));
            nvgFill(vg);
            if (font)
            {
                nvgFontFaceId(vg, font->handle);
                nvgFontSize(vg, 8.f);
                nvgTextLetterSpacing(vg, 0.f);
                nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
                nvgFillColor(vg, colour(COL_TEXT));
                nvgText(vg, x + 3.f, box.size.y - pad + 2.f, LETTERS[i], NULL);
            }
        }
    }
};
