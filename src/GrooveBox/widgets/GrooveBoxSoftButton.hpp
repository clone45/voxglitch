struct GrooveboxSoftButton : SvgSwitch
{
    GrooveBox *module;
    bool is_moused_over = false;
    Vec drag_position;
    unsigned int index = 0;
  
    GrooveboxSoftButton()
    {
        momentary = false;
        shadow->opacity = 0;
                                                                     
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_soft_button.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_soft_button_lit.svg")));

        box.size = Vec(16.240, 16.240); // was 15.5   (19.28)
    }

    void draw(const DrawArgs &args) override
    {
        // Draw background shadow
        /*
        nvgBeginPath(args.vg);
        nvgRect(args.vg, -1, -1, box.size.x + 3.0, box.size.y + 3.0);
        nvgFillColor(args.vg, nvgRGBA(0, 0, 0, 60));
        nvgFill(args.vg);
        */

        SvgSwitch::draw(args);

        // For testing, draw rectangle around box size
        /*
        nvgBeginPath(args.vg);
        nvgStrokeColor(args.vg, nvgRGB(100, 120, 255));
        nvgStrokeWidth(args.vg, 0.2f);
        nvgFillColor(args.vg, nvgRGB(100, 120, 255));
        nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
        nvgStroke(args.vg);
        */

        //
        // Draw glow effect
        //
        /*
        if(module)
        {
          ParamQuantity *param_quantity = getParamQuantity();

          if(! (param_quantity->getValue() == param_quantity->getMinValue()))
          {
            math::Vec c = box.size.div(2);
            float radius = std::min(box.size.x, box.size.y) / 2.0;
            float oradius = radius + std::min(radius * 2.f, 8.f);

            nvgBeginPath(args.vg);
            nvgRect(args.vg, c.x - oradius, c.y - oradius, 2 * oradius, 2 * oradius);

            NVGcolor icol = nvgRGBA(255, 208, 183, 60);
            NVGcolor ocol = nvgRGBA(0, 0, 0, 0);
            NVGpaint paint = nvgRadialGradient(args.vg, c.x, c.y, radius, oradius, icol, ocol);
            nvgFillPaint(args.vg, paint);
            nvgFill(args.vg);
          }
        }
        */
    }

    /*

    //
    //
    // TODO: Restore these special events before the module goes live
    //
    //


    void onButton(const event::Button &e) override
    {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            e.consume(this);
            if (module->step_copy_paste_mode)
            {
                module->copyStep(this->index);
            }
            else
            {
                drag_position = this->box.pos + Vec(this->box.size[0] / 2, 0);
                // BlueLight::onButton(e);
            }
        }
    }

    void onEnter(const event::Enter &e) override
    {
        this->is_moused_over = true;
        // BlueLight::onEnter(e);
    }

    void onLeave(const event::Leave &e) override
    {
        this->is_moused_over = false;
        // BlueLight::onLeave(e);
    }

    void onHover(const event::Hover &e) override
    {
        e.consume(this);
        this->is_moused_over = true;
    }

    void step() override
    {
        // BlueLight::step();
    }

    void onDragMove(const event::DragMove &e) override
    {
        float space_between_buttons = (button_positions[1][0] - button_positions[0][0]);

        if (module && module->shift_key)
        {
            float zoom = getAbsoluteZoom();
            drag_position = drag_position.plus(e.mouseDelta.div(zoom));

            int amount = -1 * (drag_position.x / space_between_buttons);

            if (amount != 0)
            {
                if (module->control_key)
                {
                    module->shiftAllTracks(amount);
                }
                else
                {
                    module->shiftTrack(amount);
                }

                drag_position.x = e.mouseDelta.div(zoom).x;
            }
        }
    }

    void onDoubleClick(const event::DoubleClick &e) override
    {
        if (module->shift_key)
        {
            module->step_copy_paste_mode = true;
            module->copied_step_index = this->index;
        }
    }
    */
};
