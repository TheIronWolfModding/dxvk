#include "dxvk_options.h"

namespace dxvk {

  DxvkOptions::DxvkOptions(const Config& config) {
    enableStateCache              = config.getOption<bool>    ("dxvk.enableStateCache",            true);
    enableOpenVR                  = config.getOption<bool>    ("dxvk.enableOpenVR",                true);
    enableOpenXR                  = config.getOption<bool>    ("dxvk.enableOpenXR",                false);
    numCompilerThreads            = config.getOption<int32_t> ("dxvk.numCompilerThreads",          0);
    useRawSsbo                    = config.getOption<Tristate>("dxvk.useRawSsbo",                  Tristate::Auto);
    shrinkNvidiaHvvHeap           = config.getOption<Tristate>("dxvk.shrinkNvidiaHvvHeap",         Tristate::Auto);
    hud                           = config.getOption<std::string>("dxvk.hud", "");
    forceSampleRateShading        = config.getOption<bool>("dxvk.forceSampleRateShading",          false);
    forcedSampleRateShadingFactor = config.getOption<float>("dxvk.forceSampleRateShadingFactor",   1.0f);
  }

}
