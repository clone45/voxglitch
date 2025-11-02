# VS1 Knob System - Developer Documentation

This document describes the layered knob system used in the TempestVS1 module, which can be referenced for creating similar knobs in other modules.

## Overview

The VS1 knob system uses a **layer-based architecture** where each knob consists of:
1. A **static background layer** (SVG) - doesn't rotate
2. A **rotating overlay layer** (SVG) - rotates to indicate position
3. Optional **shadow layer** (PNG) - for visual depth

This approach creates professional-looking Moog-style knobs with clear position indicators.

## Architecture

### File Structure

For the VS1 module, the knob system is organized as follows:

```
voxglitch/
├── src/
│   ├── vgLib-2.0/
│   │   ├── components/
│   │   │   ├── VoxglitchKnobs.hpp          # Knob component definitions
│   │   │   ├── ImageWidget.hpp             # PNG image support for layers
│   │   │   └── VoxglitchComponents.hpp     # Main component includes
│   │   └── panelHelper.hpp                  # SVG position finder
│   └── modules/
│       └── TempestVS1/
│           ├── TempestVS1.hpp               # Module definition
│           └── TempestVS1Widget.hpp         # Widget/UI setup
└── res/
    └── components/
        ├── moog_style_di.svg                # Rotating overlay (Double Indicator)
        ├── moog_style_di_bg.svg             # Static background
        ├── skirted_ventura.svg              # Small knob overlay
        ├── skirted_ventura_bg.svg           # Small knob background
        ├── flying_saucer_skirt_knob.svg     # Epic knob overlay
        └── flying_saucer_skirt_knob_bg.svg  # Epic knob background
```

## Layer System Explained

### How Layers Work

Each knob is built using VCV Rack's widget hierarchy with **bottom-to-top layering**:

```
┌─────────────────────────┐
│  Rotating Overlay (top) │  ← setSvg() - rotates with parameter value
├─────────────────────────┤
│  Static Background      │  ← addChildBottom() - never rotates
├─────────────────────────┤
│  Shadow (bottom)        │  ← addChildBottom() - visual depth (optional)
└─────────────────────────┘
```

**Key Concept**: The `SvgKnob` base class automatically rotates the main SVG (set via `setSvg()`), while layers added via `addChildBottom()` remain static.

### Example: VS1KnobDI (Double Indicator Knob)

```cpp
// From VoxglitchKnobs.hpp:135-152
template <typename TBase = VoxglitchKnob>
struct TVS1KnobDI : TBase {
    Widget* bg;  // Pointer to background widget

    TVS1KnobDI() {
        // Step 1: Set the rotating overlay (the knob indicator/pointer)
        this->svgFile = "moog_style_di.svg";
        this->setSvg(APP->window->loadSvg(
            asset::plugin(pluginInstance, "res/components/" + this->svgFile)
        ));

        // Step 2: Add the static background SVG
        bg = new SvgWidget();
        std::shared_ptr<Svg> bgSvg = APP->window->loadSvg(
            asset::plugin(pluginInstance, "res/components/moog_style_di_bg.svg")
        );
        dynamic_cast<SvgWidget*>(bg)->setSvg(bgSvg);
        this->addChildBottom(bg);  // ← Adds to bottom, stays static
        bg->setPosition(Vec(0, 0));
    }
};
typedef TVS1KnobDI<> VS1KnobDI;
```

### Layer Order Details

- **setSvg()**: Sets the main SVG that rotates with the knob's value
- **addChildBottom()**: Adds a widget below the main SVG (rendered first, appears behind)
- **addChild()**: Adds a widget above the main SVG (rendered last, appears in front)

## Creating New Knob Types

### Template Pattern

All Voxglitch knobs follow this template-based inheritance pattern:

```cpp
template <typename TBase = VoxglitchKnob>
struct TYourKnobName : TBase {
    Widget* bg;  // Static background
    Widget* shadow;  // Optional shadow

    TYourKnobName() {
        // 1. Configure rotation angles (optional)
        this->minAngle = -0.83 * M_PI;
        this->maxAngle = 0.83 * M_PI;

        // 2. Set rotating overlay
        this->svgFile = "your_knob_overlay.svg";
        this->setSvg(APP->window->loadSvg(
            asset::plugin(pluginInstance, "res/components/" + this->svgFile)
        ));

        // 3. Add static background (optional)
        bg = new SvgWidget();
        std::shared_ptr<Svg> bgSvg = APP->window->loadSvg(
            asset::plugin(pluginInstance, "res/components/your_knob_bg.svg")
        );
        dynamic_cast<SvgWidget*>(bg)->setSvg(bgSvg);
        this->addChildBottom(bg);
        bg->setPosition(Vec(0, 0));

        // 4. Add shadow (optional)
        // For PNG shadows, use ImageWidget
        // For SVG shadows, use SvgWidget
    }
};
typedef TYourKnobName<> YourKnobName;
```

### Base Class: VoxglitchKnob

The base class (VoxglitchKnobs.hpp:1-14) provides common defaults:

```cpp
struct VoxglitchKnob : SvgKnob {
    std::string svgFile = "";
    float orientation = 0.0;

    VoxglitchKnob() {
        svgFile = "";
        orientation = 0.0;
        minAngle = -0.83*M_PI;  // Default rotation range
        maxAngle = 0.83*M_PI;
        this->shadow->opacity = 0;  // Disable default shadow
    }
};
```

### Using ImageWidget for PNG Layers

For knobs that use PNG images (like the legacy knobs), use `ImageWidget`:

```cpp
// From VoxglitchKnobs.hpp:22-37 (TLargeKnob example)
ImageWidget* bg;
ImageWidget* shadow;

TLargeKnob() {
    // Rotating overlay (SVG)
    this->svgFile = "large_knob_overlay.svg";
    this->setSvg(APP->window->loadSvg(
        asset::plugin(pluginInstance, "res/components/" + this->svgFile)
    ));

    // PNG background
    bg = new ImageWidget("res/components/png/Big-Knob.png", 22.2, 24.2);
    this->addChildBottom(bg);
    bg->setPosition(Vec(-2.8, -2.25));

    // PNG shadow
    shadow = new ImageWidget("res/themes/default/round_shadow.png", 32.0, 32.0);
    this->addChildBottom(shadow);
    shadow->setPosition(Vec(-16.0, 0.0));
}
```

## Adding Knobs to Panels

### Using PanelHelper

The `PanelHelper` class finds component positions from named shapes in your SVG panel:

```cpp
// From TempestVS1Widget.hpp:119-123
PanelHelper panelHelper(this);
panelHelper.loadPanel(asset::plugin(
    pluginInstance,
    "res/modules/tempest_vs1/tempest_vs1_panel.svg"
));
```

### Finding Positions in SVG

In your panel SVG, add named shapes (rectangles, circles, etc.) with IDs:

```xml
<!-- In your panel SVG file -->
<rect id="m1_knob" x="50" y="100" width="10" height="10" />
<rect id="m2_knob" x="50" y="150" width="10" height="10" />
```

Then use `findNamed()` to get positions:

```cpp
// From TempestVS1Widget.hpp:154-157
HoverIndicatorParam<VS1KnobDI>* m1Knob = createParamCentered<HoverIndicatorParam<VS1KnobDI>>(
    panelHelper.findNamed("m1_knob"),  // ← Gets Vec position from SVG
    module,
    TempestVS1::M1_KNOB_PARAM
);
```

### PanelHelper Methods

```cpp
// Find single named position
Vec findNamed(const std::string& name);

// Find all positions with prefix (e.g., "knob_" finds "knob_1", "knob_2", etc.)
std::vector<NamedPosition> findPrefixed(const std::string& prefix);

// Find positions matching regex pattern
std::vector<NamedPosition> findMatched(const std::string& pattern);

// Iterate over prefixed positions
void forEachPrefixed(const std::string& prefix,
    const std::function<void(unsigned int, const Vec&)>& callback);
```

## HoverIndicator System

The VS1 module includes a **hover indicator** system that shows which outputs are associated with each knob when you hover over it.

### How It Works

1. **Template Wrapper**: `HoverIndicatorParam<T>` wraps any parameter type
2. **Event Handlers**: Captures `onEnter` and `onLeave` mouse events
3. **Ring Widgets**: Shows/hides blue rings around associated output jacks

### Implementation

#### 1. Create the Template Wrapper (TempestVS1Widget.hpp:26-47)

```cpp
template <typename TParam>
struct HoverIndicatorParam : TParam {
    TempestVS1Widget* parentWidget = nullptr;
    int paramId = -1;

    void onEnter(const widget::Widget::EnterEvent& e) override {
        TParam::onEnter(e);
        if (parentWidget && paramId >= 0) {
            showOutputRing();
        }
    }

    void onLeave(const widget::Widget::LeaveEvent& e) override {
        TParam::onLeave(e);
        if (parentWidget && paramId >= 0) {
            hideOutputRing();
        }
    }

    void showOutputRing();  // Implemented later
    void hideOutputRing();
};
```

#### 2. Create Ring Widget (TempestVS1Widget.hpp:11-23)

```cpp
struct OutputRingWidget : widget::TransparentWidget {
    NVGcolor ringColor = nvgRGB(0, 150, 255);
    float ringWidth = 2.0;

    void draw(const DrawArgs& args) override {
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, box.size.x / 2.0, box.size.y / 2.0,
                  box.size.x / 2.0 - ringWidth / 2.0);
        nvgStrokeColor(args.vg, ringColor);
        nvgStrokeWidth(args.vg, ringWidth);
        nvgStroke(args.vg);
        widget::TransparentWidget::draw(args);
    }
};
```

#### 3. Add Ring Management to Widget (TempestVS1Widget.hpp:101-116)

```cpp
struct TempestVS1Widget : ModuleWidget {
    std::map<int, std::vector<Widget*>> outputRings; // paramId -> ring widgets

    void showOutputRing(int paramId) {
        if (outputRings.count(paramId)) {
            for (Widget* ring : outputRings[paramId]) {
                ring->visible = true;
            }
        }
    }

    void hideOutputRing(int paramId) {
        if (outputRings.count(paramId)) {
            for (Widget* ring : outputRings[paramId]) {
                ring->visible = false;
            }
        }
    }
};
```

#### 4. Use Wrapped Knobs (TempestVS1Widget.hpp:154-157)

```cpp
HoverIndicatorParam<VS1KnobDI>* m1Knob =
    createParamCentered<HoverIndicatorParam<VS1KnobDI>>(
        panelHelper.findNamed("m1_knob"),
        module,
        TempestVS1::M1_KNOB_PARAM
    );
m1Knob->parentWidget = this;  // ← Set parent for callbacks
m1Knob->paramId = TempestVS1::M1_KNOB_PARAM;
addParam(m1Knob);
```

#### 5. Create Rings for Outputs (TempestVS1Widget.hpp:283-290)

```cpp
void addOutputRing(int paramId, Vec position) {
    OutputRingWidget* ring = new OutputRingWidget();
    ring->box.size = Vec(38.0, 38.0); // Slightly larger than port
    ring->box.pos = position.minus(Vec(19.0, 19.0)); // Center the ring
    ring->visible = false; // Initially hidden
    addChild(ring);
    outputRings[paramId].push_back(ring);
}

// In constructor:
addOutputRing(TempestVS1::M1_KNOB_PARAM, panelHelper.findNamed("m1_knob_output"));
```

## VS1 Knob Types Reference

### Main Knobs (VS1KnobDI)
- **Use**: Large knobs with double indicator marks
- **Files**: `moog_style_di.svg` + `moog_style_di_bg.svg`
- **Example**: M1, M2, M3, M4 knobs

### Small Knobs (SkirtedVenturaKnob)
- **Use**: Small skirted knobs with single indicator
- **Files**: `skirted_ventura.svg` + `skirted_ventura_bg.svg`
- **Example**: SM1-SM4 knobs

### Epic Knob (FlyingSaucerSkirtKnob)
- **Use**: Large center knob with wide skirt
- **Files**: `flying_saucer_skirt_knob.svg` + `flying_saucer_skirt_knob_bg.svg`
- **Example**: Epic center knob

### Center Knob (VS1CenterKnob)
- **Use**: Standard Moog-style center knob
- **Files**: `moog_style_center.svg` + `moog_style_center_bg.svg`
- **Example**: Center knob

### Rotary Knob (VS1RotaryKnob)
- **Use**: Encoder knob without indicator dot
- **Files**: `moog_style_rotary.svg` + `moog_style_rotary_bg.svg`
- **Example**: Rotary encoder

## Complete Example: Adding a New Knob

### 1. Create SVG Assets

Create two SVG files:
- `my_knob.svg` - Rotating overlay with indicator
- `my_knob_bg.svg` - Static background

### 2. Define Knob Component

Add to `VoxglitchKnobs.hpp`:

```cpp
template <typename TBase = VoxglitchKnob>
struct TMyKnob : TBase {
    Widget* bg;

    TMyKnob() {
        this->svgFile = "my_knob.svg";
        this->setSvg(APP->window->loadSvg(
            asset::plugin(pluginInstance, "res/components/" + this->svgFile)
        ));

        bg = new SvgWidget();
        std::shared_ptr<Svg> bgSvg = APP->window->loadSvg(
            asset::plugin(pluginInstance, "res/components/my_knob_bg.svg")
        );
        dynamic_cast<SvgWidget*>(bg)->setSvg(bgSvg);
        this->addChildBottom(bg);
        bg->setPosition(Vec(0, 0));
    }
};
typedef TMyKnob<> MyKnob;
```

### 3. Add to Module Definition

In your module's `.hpp` file:

```cpp
struct MyModule : Module {
    enum ParamIds {
        MY_KNOB_PARAM,
        NUM_PARAMS
    };

    MyModule() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(MY_KNOB_PARAM, 0.0f, 10.0f, 5.0f, "My Knob", "V");
    }
};
```

### 4. Add to Widget

In your widget's constructor:

```cpp
PanelHelper panelHelper(this);
panelHelper.loadPanel(asset::plugin(pluginInstance, "res/modules/my_module/panel.svg"));

// Basic usage
MyKnob* knob = createParamCentered<MyKnob>(
    panelHelper.findNamed("my_knob"),
    module,
    MyModule::MY_KNOB_PARAM
);
addParam(knob);

// With hover indicators (optional)
HoverIndicatorParam<MyKnob>* knob = createParamCentered<HoverIndicatorParam<MyKnob>>(
    panelHelper.findNamed("my_knob"),
    module,
    MyModule::MY_KNOB_PARAM
);
knob->parentWidget = this;
knob->paramId = MyModule::MY_KNOB_PARAM;
addParam(knob);
```

### 5. Add Named Position to Panel SVG

In your `panel.svg` file:

```xml
<rect id="my_knob" x="50" y="100" width="20" height="20" opacity="0" />
```

## Key Takeaways

1. **Layers are key**: Use `setSvg()` for rotating parts, `addChildBottom()` for static parts
2. **Template pattern**: All knobs inherit from `VoxglitchKnob` via templates
3. **PanelHelper**: Positions come from named SVG shapes, not hardcoded coordinates
4. **HoverIndicator**: Optional but enhances UX by showing parameter-to-output relationships
5. **Typedef pattern**: Always create a typedef for easy usage: `typedef TMyKnob<> MyKnob;`

## Files to Study

- **VoxglitchKnobs.hpp**: All knob definitions and examples
- **TempestVS1Widget.hpp**: Complete example of knob usage with hover indicators
- **panelHelper.hpp**: SVG position finding utilities
- **ImageWidget.hpp**: PNG layer support

## See Also

- VCV Rack Component Library: https://vcvrack.com/manual/PluginDevelopmentTutorial
- SVG Knob Tutorial: Understanding `SvgKnob` base class behavior
- NanoVG Drawing: For custom widget rendering (like `OutputRingWidget`)
