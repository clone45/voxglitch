//
// When adding a new LCD Page:
//
// 1. Add a new index for it in defines.h similar to LCD_PAGE_ENGINE
// 2. Add the page name to ENGINE_NAMES[3] in defines.h
// 3. Increase NUMBER_OF_LCD_PAGES in defines.h
// 4. Update LCDWidget.hpp by
//    a. include the new engine class
//    b. add the new engine to the lcd_pages array

struct LCDPage
{
  Scalar110 *module;
  virtual void draw(NVGcontext *vg) = 0;
  virtual void onButton(Vec position) = 0;
  virtual void onDragMove(Vec position) = 0;
};
