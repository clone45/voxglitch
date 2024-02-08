#include "SequencerView.hpp"
#include "GateSequencer.hpp"

namespace vgLib_v2
{

    struct GateSequencerView : SequencerView
    {
        bool mouse_lock = false;
        int old_drag_bar_x = -1;
        bool trigger_edit_value = false;

        float bar_height = 0;
        double bar_width = 0;
        float bar_horizontal_padding = 0;
        unsigned int max_sequencer_steps = 0;

        GateSequencer **gate_sequencer = NULL;

        GateSequencerView(GateSequencer **pointer_to_active_gate_sequencer, float width, float height, float bar_height, float bar_horizontal_padding, unsigned int max_sequencer_steps)
            : SequencerView(width, height, (width / max_sequencer_steps) - bar_horizontal_padding, bar_horizontal_padding, max_sequencer_steps)
        {
            this->gate_sequencer = pointer_to_active_gate_sequencer;
            this->width = width;
            this->height = height;
            this->bar_width = (width / max_sequencer_steps) - bar_horizontal_padding;
            this->bar_height = bar_height;
            this->bar_horizontal_padding = bar_horizontal_padding;
            this->max_sequencer_steps = max_sequencer_steps;

            box.size = Vec(width, height);
        }

        void drawLayer(const DrawArgs &args, int layer) override
        {
            if (layer == 1)
            {
                const auto vg = args.vg;
                int value;
                double value_height;
                NVGcolor bar_color;

                nvgSave(vg);

                if (*gate_sequencer)
                {
                    for (unsigned int i = 0; i < max_sequencer_steps; i++)
                    {
                        value = (*gate_sequencer)->getValue(i);

                        // Draw grey background bar
                        if (i <= (*gate_sequencer)->getWindowEnd())
                        {
                            bar_color = brightness(nvgRGBA(60, 60, 64, 255), settings::rackBrightness);
                        }
                        else
                        {
                            bar_color = brightness(nvgRGBA(45, 45, 45, 255), settings::rackBrightness);
                        }
                        drawBar(vg, i, bar_height, height, bar_color);

                        unsigned int playback_position = (*gate_sequencer)->getPlaybackPosition();

                        if (i == playback_position)
                        {
                            bar_color = nvgRGBA(255, 255, 255, 250);
                        }

                        else if (i <= (*gate_sequencer)->getWindowEnd())
                        {
                            bar_color = nvgRGBA(255, 255, 255, 150);
                        }
                        else // dim bars past playback position
                        {
                            bar_color = nvgRGBA(255, 255, 255, 15);
                        }

                        value_height = (bar_height * value);
                        if (value_height > 0)
                            drawBar(vg, i, value_height, height, bar_color);

                        // highlight active column
                        if (i == playback_position)
                        {
                            drawBar(vg, i, bar_height, height, nvgRGBA(255, 255, 255, 20));
                        }
                    }
                }
                else // draw demo data for the module explorer
                {
                    int demo_sequence[32] = {1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0};

                    for (unsigned int i = 0; i < 32; i++)
                    {
                        value = demo_sequence[i];

                        // Draw background grey bar
                        drawBar(vg, i, bar_height, height, nvgRGBA(60, 60, 64, 255));

                        // Draw value bar
                        if (value > 0)
                            drawBar(vg, i, (bar_height * value), height, nvgRGBA(255, 255, 255, 150));

                        // highlight active column
                        if (i == 5)
                            drawBar(vg, i, bar_height, height, nvgRGBA(255, 255, 255, 20));
                    }
                }

                drawVerticalGuildes(vg, height);
                drawOverlay(vg, width, height);

                nvgRestore(vg);
            }
        }

        void onButton(const event::Button &e) override
        {
            e.consume(this);

            if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
            {
                if (this->mouse_lock == false)
                {
                    this->mouse_lock = true;

                    int index = getIndexFromX(e.pos.x);

                    // Store the value that's being set for later in case the user
                    // drags to set ("paints") additional triggers
                    this->trigger_edit_value = !(*gate_sequencer)->getValue(index);

                    // Set the trigger value in the sequencer
                    (*gate_sequencer)->setValue(index, this->trigger_edit_value);

                    // Store the initial drag position
                    drag_position = e.pos;
                }
            }
            else if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
            {
                this->mouse_lock = false;
            }
        }

        void onDragMove(const event::DragMove &e) override
        {
            TransparentWidget::onDragMove(e);

            double zoom = getAbsoluteZoom();
            drag_position = drag_position.plus(e.mouseDelta.div(zoom));

            // If the user drags past the edges, reset the drag action, otherwise it can get stuck.

            int drag_bar_x_index = getIndexFromX(drag_position.x);

            if (drag_bar_x_index < 0 || drag_bar_x_index > ((*gate_sequencer)->getMaxLength() - 1))
            {
                drag_bar_x_index = -1;
                this->mouse_lock = false;
            }
            else
            {
                if (drag_bar_x_index != old_drag_bar_x)
                {
                    (*gate_sequencer)->setValue(drag_bar_x_index, trigger_edit_value);
                    old_drag_bar_x = drag_bar_x_index;
                }
            }
        }

        void onHoverKey(const event::HoverKey &e) override
        {
            /*
            if (keypressRight(e))
            {
                gate_sequencer->shiftRight();
                if ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT)
                    module->selected_voltage_sequencer->shiftRight();
            }

            if (keypressLeft(e))
            {
                gate_sequencer->shiftLeft();
                if ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT)
                    module->selected_voltage_sequencer->shiftLeft();
            }
            */

            /*
            // Randomize sequence by hovering over and pressing 'r'
            if (e.key == GLFW_KEY_R && e.action == GLFW_PRESS)
            {
                if ((e.mods & RACK_MOD_MASK) != GLFW_MOD_CONTROL)
                {
                    gate_sequencer->randomize();
                    if ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT)
                        module->selected_voltage_sequencer->randomize();
                }
            }
            */
        }

        int getIndexFromX(double x)
        {
            return (x / (barWidth() + bar_horizontal_padding));
        }

        double barWidth()
        {
            return ((width / (*gate_sequencer)->getMaxLength()) - bar_horizontal_padding);
        }
    };

} // namespace vgLib_v2