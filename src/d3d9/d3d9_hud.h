#pragma once

#include "d3d9_device.h"
#include "../dxvk/hud/dxvk_hud_item.h"

namespace dxvk::hud {

  /**
   * \brief HUD item to display sampler count
   */
  class HudSamplerCount : public HudItem {

  public:

    HudSamplerCount(D3D9DeviceEx* device);

    void update(dxvk::high_resolution_clock::time_point time);

    HudPos render(
            HudRenderer&      renderer,
            HudPos            position);

  private:

    D3D9DeviceEx* m_device;

    std::string m_samplerCount;

  };

    /**
     * \brief HUD item to display unmappable memory
     */
    class HudTextureMemory : public HudItem {
      constexpr static int64_t UpdateInterval = 500'000;

    public:

        HudTextureMemory(D3D9DeviceEx* device);

        void update(dxvk::high_resolution_clock::time_point time);

        HudPos render(
                HudRenderer&      renderer,
                HudPos            position);

    private:

        D3D9DeviceEx* m_device;

        uint32_t m_maxTextureAllocated = 0;
        uint32_t m_maxTextureUsed      = 0;
        uint32_t m_maxTextureMapped    = 0;
        uint32_t m_maxBufferAllocated = 0;
        uint32_t m_maxBufferUsed = 0;
        uint32_t m_maxBufferMapped = 0;

        dxvk::high_resolution_clock::time_point m_lastUpdate
          = dxvk::high_resolution_clock::now();

        std::string m_texturesAllocatedString;
        std::string m_texturesMappedString;
        std::string m_buffersAllocatedString;
        std::string m_buffersMappedString;
    };

}