#include "dxvk_options.h"

namespace dxvk {

  DxvkOptions::DxvkOptions(const Config& config) {
    enableDebugUtils              = config.getOption<bool>    ("dxvk.enableDebugUtils",       false);
    enableStateCache              = config.getOption<bool>    ("dxvk.enableStateCache",       true);
    numCompilerThreads            = config.getOption<int32_t> ("dxvk.numCompilerThreads",     0);
    useRawSsbo                    = config.getOption<Tristate>("dxvk.useRawSsbo",             Tristate::Auto);
    enableGraphicsPipelineLibrary = config.getOption<Tristate>("dxvk.enableGraphicsPipelineLibrary", Tristate::Auto);
    trackPipelineLifetime = config.getOption<Tristate>("dxvk.trackPipelineLifetime",  Tristate::Auto);
    hud                           = config.getOption<std::string>("dxvk.hud", "");
    forceSampleRateShading        = config.getOption<bool>("dxvk.forceSampleRateShading",          false);
    forcedSampleRateShadingFactor = config.getOption<float>("dxvk.forceSampleRateShadingFactor",   1.0f);
  }
}
