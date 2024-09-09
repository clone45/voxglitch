namespace vgLib_v1
{

    struct VoxglitchSamplerModuleWidget : ModuleWidget
    {
        struct InterpolationOffOption : MenuItem
        {
            VoxglitchSamplerModule *module;

            void onAction(const event::Action &e) override
            {
                module->interpolation = 0;
            }
        };

        struct InterpolationLinearOption : MenuItem
        {
            VoxglitchSamplerModule *module;

            void onAction(const event::Action &e) override
            {
                module->interpolation = 1;
            }
        };

        struct SampleInterpolationMenuItem : MenuItem
        {
            VoxglitchSamplerModule *module;

            Menu *createChildMenu() override
            {
                Menu *menu = new Menu;

                InterpolationOffOption *interpolation_off_option = createMenuItem<InterpolationOffOption>("Off", CHECKMARK(module->interpolation == 0));
                interpolation_off_option->module = module;
                menu->addChild(interpolation_off_option);

                InterpolationLinearOption *interpolation_linear_option = createMenuItem<InterpolationLinearOption>("Linear (Better Quality)", CHECKMARK(module->interpolation == 1));
                interpolation_linear_option->module = module;
                menu->addChild(interpolation_linear_option);

                return menu;
            }
        };
    };

} // namespace vgLib_v1