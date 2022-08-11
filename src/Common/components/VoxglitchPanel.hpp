struct VoxglitchPanel : app::SvgPanel {

  VoxglitchPanel()
  {
#if defined DEV_MODE
  panelBorder->visible = true;
#else
  panelBorder->visible = false;
#endif
  }
};
