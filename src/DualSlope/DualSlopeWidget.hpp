struct DualSlopeWidget : ModuleWidget
{
  DualSlopeWidget(DualSlope* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/dual_slope_front_panel.svg")));

    DualSlopeCanvas *dual_slope_canvas = new DualSlopeCanvas();
    dual_slope_canvas->box.pos = mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y));
    dual_slope_canvas->module = module;
    addChild(dual_slope_canvas);
  }

};
