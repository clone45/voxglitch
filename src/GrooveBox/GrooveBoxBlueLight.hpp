struct GrooveBoxBlueLight : BlueLight {

  float halo_counter = 1.0;
  GrooveBox *module;

  void pulse()
  {
    halo_counter = 2.0;
  }

  void drawHalo(const DrawArgs& args) override {
  	// Don't draw halo if rendering in a framebuffer, e.g. screenshots or Module Browser
  	if (args.fb)
  		return;

  	const float halo = settings::haloBrightness;
  	if (halo == 0.f)
  		return;

    float new_halo = (halo / 2.0) * halo_counter;

    if(this->module)
    {
      if(halo_counter > 1.0) halo_counter -= module->sample_rate;
    }

  	// If light is off, rendering the halo gives no effect.
  	if (color.r == 0.f && color.g == 0.f && color.b == 0.f)
  		return;

  	math::Vec c = box.size.div(2);
  	float radius = std::min(box.size.x, box.size.y) / 2.0;
  	float oradius = radius + std::min(radius * 4.f, 15.f);

  	nvgBeginPath(args.vg);
  	nvgRect(args.vg, c.x - oradius, c.y - oradius, 2 * oradius, 2 * oradius);

  	NVGcolor icol = color::mult(color, new_halo);
  	NVGcolor ocol = nvgRGBA(0, 0, 0, 0);
  	NVGpaint paint = nvgRadialGradient(args.vg, c.x, c.y, radius, oradius, icol, ocol);
  	nvgFillPaint(args.vg, paint);
  	nvgFill(args.vg);
  }
};
