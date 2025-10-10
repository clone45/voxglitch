# Context Menu Implementation in Voxglitch Modules

This document describes how context menus are implemented in newer Voxglitch modules like ArpSeq, CueResearch, WavBank, Ghosts, and Looper.

## Basic Structure

All module widgets implement context menus by overriding the `appendContextMenu` method in their widget class:

```cpp
void appendContextMenu(Menu *menu) override
{
    ModuleName *module = dynamic_cast<ModuleName *>(this->module);
    assert(module);

    // Menu items go here
}
```

## Common Menu Patterns

### 1. Simple Boolean Options

Use `createBoolPtrMenuItem` for toggle options that directly modify a boolean member variable:

```cpp
menu->addChild(createBoolPtrMenuItem("Option Name", "", &module->bool_variable));
```

Example from CueResearch (src/modules/CueResearch/CueResearchWidget.hpp:101-103):
- "Enable Vertical-Drag Zoom"
- "Clear markers on sample load"
- "Loop sample playback"

Example from ArpSeq (src/modules/ArpSeq/ArpSeqWidget.hpp:602):
- "Legacy Reset Mode"

### 2. Submenu with Multiple Choices

Use `createIndexSubmenuItem` for options with multiple predefined choices:

```cpp
menu->addChild(createIndexSubmenuItem("Menu Title",
    {"Option 1", "Option 2", "Option 3"},
    [=]() {
        return module->current_index;  // Get current selection
    },
    [=](int index) {
        module->setOption(index);      // Set new selection
    }
));
```

Examples from ArpSeq (src/modules/ArpSeq/ArpSeqWidget.hpp:618-644):
- "Rate Attenuverter Response" with options "-5V to +5V" and "-10V to +10V"
- "Shape Attenuverter Response" with similar voltage range options

### 3. Custom Submenus with MenuItem Classes

For more complex submenus, create custom MenuItem classes:

```cpp
struct CustomMenuItem : MenuItem
{
    ModuleName *module;

    Menu *createChildMenu() override
    {
        Menu *menu = new Menu;
        // Add child menu items here
        return menu;
    }
};
```

Example from ArpSeq (src/modules/ArpSeq/ArpSeqWidget.hpp:492-554):
- `ProbabilityOutputSettingsMenuItem` for probability output configuration
- `CycleOutputSettingsMenuItem` for cycle output configuration
- `QuantizeOutputSettingsMenuItem` for quantization settings

### 4. Action Menu Items

Use `createMenuItem` with a lambda for immediate actions:

```cpp
menu->addChild(createMenuItem("Action Name", "", [=]() {
    module->performAction();
}));
```

Examples from CueResearch (src/modules/CueResearch/CueResearchWidget.hpp:130-132):
- "Clear All Markers" - immediately clears all markers

From Ghosts (src/modules/Ghosts/GhostsWidget.hpp:87-91):
- "Remove Sample" - immediately unloads the sample

### 5. Custom Menu Items for Special Controls

For knobs and other controls that can have context menus, implement `appendContextMenu` directly on the control:

```cpp
struct NumberChooser : app::SvgKnob
{
    void appendContextMenu(Menu *menu) override
    {
        ArpSeq *module = dynamic_cast<ArpSeq *>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Reset Cycles", "", [=]() {
            module->resetPageCycleCounters();
        }));
    }
};
```

Example from ArpSeq (src/modules/ArpSeq/ArpSeqWidget.hpp:94-108):
- NumberChooser knob has its own context menu with "Reset Cycles" and "Smart Randomize"

## Menu Organization

### Separators and Labels

Use these for visual organization:

```cpp
menu->addChild(new MenuEntry);        // Blank spacing
menu->addChild(new MenuSeparator());  // Horizontal line
menu->addChild(createMenuLabel("Section Title"));
```

### Typical Menu Structure

1. **Separator** at top (new MenuEntry)
2. **Settings sections** with labels
3. **Boolean toggles** for simple options
4. **Submenus** for multi-choice options (with RIGHT_ARROW indicator)
5. **Action items** for immediate operations
6. **File operations** (load/save) typically at bottom

## Visual Indicators

- `CHECKMARK(condition)` - Shows checkmark when condition is true
- `RIGHT_ARROW` - Indicates a submenu will open

Example from WavBank (src/modules/WavBank/WavBankWidget.hpp:75-81):
```cpp
TriggerOption *trigger_option = createMenuItem<TriggerOption>(
    "Trigger",
    CHECKMARK(module->trig_input_response_mode == TRIGGER)
);
```

## File Operations in Menus

Many modules include sample/file loading in their context menus:

```cpp
struct LoadSampleMenuItem : MenuItem
{
    Module *module;

    void onAction(const event::Action &e) override
    {
        // File dialog and loading logic
    }
};
```

Examples:
- WavBank: "Select Directory Containing WAV Files"
- Ghosts: Sample loading with filename display
- CueResearch: Sample loading with current filename shown

## Best Practices

1. **Always cast and assert** the module pointer at the start
2. **Group related options** under labeled sections
3. **Use consistent naming** for similar options across modules
4. **Provide visual feedback** with checkmarks for current selections
5. **Use RIGHT_ARROW** to indicate submenus
6. **Place destructive actions** (like "Remove Sample") clearly separated
7. **Keep menu items concise** - detailed settings go in submenus