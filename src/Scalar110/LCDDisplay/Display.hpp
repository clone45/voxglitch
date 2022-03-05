struct Display
{
  Scalar110 *module;
  virtual void draw(NVGcontext *vg) = 0;
};
