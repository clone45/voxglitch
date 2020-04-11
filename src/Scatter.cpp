//
// Voxglitch "Scatter" module for VCV Rack
//
// TODO: memory cleanup for samples?

#include "plugin.hpp"
#include "osdialog.h"
#include "Common/sample.hpp"

#include "Scatter/defines.h"
#include "Scatter/SamplePlayer.hpp"
#include "Scatter/Scatter.hpp"
#include "Scatter/ScatterLoadSample.hpp"
#include "Scatter/ScatterWidget.hpp"

Model* modelScatter = createModel<Scatter, ScatterWidget>("scatter");
