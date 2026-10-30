//
// BrowserSmoke — the module-browser smoke test. Test builds only: see
// BrowserSmoke/BrowserSmoke.hpp and scripts/browser_smoke.py.
//
#ifdef VOXGLITCH_BROWSER_SMOKE

#include "plugin.hpp"
#include "BrowserSmoke/BrowserSmoke.hpp"

Model* modelBrowserSmoke = createModel<BrowserSmoke, BrowserSmokeWidget>("BrowserSmoke");

#endif
