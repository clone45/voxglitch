struct Display
{
  Scalar110 *module;
  virtual void draw(NVGcontext *vg) = 0;
  virtual void onButton(Vec position) = 0;
  virtual void onDragMove(Vec position) = 0;

};
