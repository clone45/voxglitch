#include "plugin.hpp"
#include "osdialog.h"

#include "ByteBender/defines.h"
#include "ByteBender/ByteBender.hpp"
#include "ByteBender/ByteBenderWidget.hpp"

Model* modelByteBender = createModel<ByteBender, ByteBenderWidget>("bytebender");
