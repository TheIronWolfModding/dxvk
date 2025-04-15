#pragma once

#include "d3d9_include.h"

namespace dxvk::caps {

  constexpr uint32_t MaxClipPlanes                = 6;
  constexpr uint32_t MaxSamplers                  = 16;
  constexpr uint32_t MaxStreams                   = 16;
  constexpr uint32_t MaxSimultaneousTextures      = 8;
  constexpr uint32_t MaxTextureBlendStages        = MaxSimultaneousTextures;
  constexpr uint32_t MaxSimultaneousRenderTargets = D3D_MAX_SIMULTANEOUS_RENDERTARGETS;

  // GTR2_SPECIFIC: Some shaders use indexing/relative access into c136 for calculating
  // lighting (I think).  The very worst possible case is c136+64.
  //
  // DxsoCompiler::emitRegisterPtr unfortunately maxes out the size in such case:
  //
  //   if (!relative) {
  //     ...
  //   else {
  //     m_meta.maxConstIndexF = m_layout->floatCount;
  //
  // and constant upload is one of the biggest DXVK costs on the main thread.
  //
  // c136 is the last register I ever seen and 12 is the highest index I know of.
  // See: IDirect3DDevice9Proxy::SetVertexShaderConstantI.
  constexpr uint32_t MaxFloatConstantsVS          = 136 + 12;
  //constexpr uint32_t MaxFloatConstantsVS          = 256;
  constexpr uint32_t MaxFloatConstantsPS          = 224;
  constexpr uint32_t MaxOtherConstants            = 16;
  constexpr uint32_t MaxFloatConstantsSoftware    = 8192;
  constexpr uint32_t MaxOtherConstantsSoftware    = 2048;

  constexpr uint32_t InputRegisterCount           = 16;

  constexpr uint32_t MaxTextureDimension          = 16384;
  constexpr uint32_t MaxMipLevels                 = 15;
  constexpr uint32_t MaxSubresources              = 15 * 6;

  constexpr uint32_t MaxTransforms                = 10 + 256;

  constexpr uint32_t TextureStageCount           = MaxSimultaneousTextures;

  constexpr uint32_t MaxEnabledLights             = 8;

  constexpr uint32_t MaxTexturesVS                = 4;
  constexpr uint32_t MaxTexturesPS                = 16;

}