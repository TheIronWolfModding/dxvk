#include "d3d9_hud.h"

namespace dxvk::hud {

  HudSamplerCount::HudSamplerCount(D3D9DeviceEx* device)
    : m_device       (device)
    , m_samplerCount ("0"){

  }


  void HudSamplerCount::update(dxvk::high_resolution_clock::time_point time) {
    DxvkSamplerStats stats = m_device->GetDXVKDevice()->getSamplerStats();
    m_samplerCount = str::format(stats.totalCount);
  }


  HudPos HudSamplerCount::render(
    const DxvkContextObjects& ctx,
    const HudPipelineKey&     key,
    const HudOptions&         options,
          HudRenderer&        renderer,
          HudPos              position) {
    position.y += 16;
    renderer.drawText(16, position, 0xffc0ff00u, "Samplers:");
    renderer.drawText(16, { position.x + 120, position.y }, 0xffffffffu, m_samplerCount);

    position.y += 8;
    return position;
  }

  HudTextureMemory::HudTextureMemory(D3D9DeviceEx* device)
  : m_device          (device)
  , m_texturesAllocatedString ("")
  , m_texturesMappedString    ("") { }


  void HudTextureMemory::update(dxvk::high_resolution_clock::time_point time) {
    D3D9MemoryAllocator* allocator = m_device->GetTextureAllocator();

    m_maxTexturesAllocated = std::max(m_maxTexturesAllocated, allocator->AllocatedMemory());
    m_maxTexturesUsed = std::max(m_maxTexturesUsed, allocator->UsedMemory());
    m_maxTexturesMapped = std::max(m_maxTexturesMapped, allocator->MappedMemory());

    D3D9MemoryAllocator* bufferAllocator = m_device->GetBufferAllocator();
    m_maxBuffersAllocated =
      std::max(m_maxBuffersAllocated, bufferAllocator->AllocatedMemory());
    m_maxBuffersUsed = std::max(m_maxBuffersUsed, bufferAllocator->UsedMemory());
    m_maxBuffersMapped =
      std::max(m_maxBuffersMapped, bufferAllocator->MappedMemory());

    /*static uint32_t peakMapped = 0u;
    if (textureAllocator->MappedMemory() > peakMapped)
      peakMapped = textureAllocator->MappedMemory();

    if ((::GetAsyncKeyState(VK_NUMPAD0) & 1) != 0)
      peakMapped = 0u;*/

    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(time - m_lastUpdate);

    if (elapsed.count() < UpdateInterval)
      return;

    m_texturesAllocatedString = str::format(std::setfill(' '), std::setw(5), m_maxTexturesAllocated >> 20, " MB   ", std::setw(5), m_maxTexturesUsed >> 20, " MB used");
    m_texturesMappedString = str::format(std::setfill(' '), std::setw(5), m_maxTexturesMapped >> 20, " MB"/* (Peak: ", peakMapped >> 20, " MB)"*/);
    //m_texturesAllocatedString = str::format(m_maxTexturesAllocated >> 20, " MB       Used: ", m_maxTexturesUsed >> 20, " MB");
    //m_texturesMappedString = str::format(m_maxTexturesMapped >> 20, " MB"/* (Peak: ", peakMapped >> 20, " MB)"*/);
    m_maxTexturesAllocated = 0;
    m_maxTexturesUsed = 0;
    m_maxTexturesMapped = 0;

    m_buffersAllocatedString = str::format(std::setfill(' '), std::setw(5), m_maxBuffersAllocated >> 20, " MB   ", std::setw(5), m_maxBuffersUsed >> 20, " MB used");
    m_buffersMappedString = str::format(std::setfill(' '), std::setw(5), m_maxBuffersMapped >> 20, " MB"/* (Peak: ", peakMapped >> 20, " MB)"*/);
    m_maxBuffersAllocated = 0;
    m_maxBuffersUsed = 0;
    m_maxBuffersMapped = 0;

    m_lastUpdate = time;
  }


  HudPos HudTextureMemory::render(
    const DxvkContextObjects& ctx,
    const HudPipelineKey&     key,
    const HudOptions&         options,
          HudRenderer&        renderer,
          HudPos              position) {
    position.y += 16;
    renderer.drawText(16, position, 0xffc0ff00u, "Mappable Textures:");
    renderer.drawText(16, { position.x + 220, position.y }, 0xffffffffu, m_texturesAllocatedString);

    position.y += 20;
    renderer.drawText(16, position, 0xffc0ff00u, "Mapped Textures:");
    renderer.drawText(16, { position.x + 220, position.y }, 0xffffffffu, m_texturesMappedString);
    
    position.y += 20;
    renderer.drawText(16, position, 0xffc0ff00u, "Mappable Buffers:");
    renderer.drawText(16, { position.x + 220, position.y }, 0xffffffffu, m_buffersAllocatedString);

    position.y += 20;
    renderer.drawText(16, position, 0xffc0ff00u, "Mapped Buffers:");
    renderer.drawText(16, { position.x + 220, position.y }, 0xffffffffu, m_buffersMappedString);

    position.y += 8;
    return position;
  }

  HudFixedFunctionShaders::HudFixedFunctionShaders(D3D9DeviceEx* device)
  : m_device        (device)
  , m_ffShaderCount ("") {}


  void HudFixedFunctionShaders::update(dxvk::high_resolution_clock::time_point time) {
    m_ffShaderCount = str::format(
      "VS: ", m_device->GetFixedFunctionVSCount(),
      " FS: ", m_device->GetFixedFunctionFSCount(),
      " SWVP: ", m_device->GetSWVPShaderCount()
    );
  }


  HudPos HudFixedFunctionShaders::render(
    const DxvkContextObjects& ctx,
    const HudPipelineKey&     key,
    const HudOptions&         options,
          HudRenderer&        renderer,
          HudPos              position) {
    position.y += 16;
    renderer.drawText(16, position, 0xffc0ff00u, "FF Shaders:");
    renderer.drawText(16, { position.x + 155, position.y }, 0xffffffffu, m_ffShaderCount);

    position.y += 8;
    return position;
  }


  HudSWVPState::HudSWVPState(D3D9DeviceEx* device)
          : m_device          (device)
          , m_isSWVPText ("") {}



  void HudSWVPState::update(dxvk::high_resolution_clock::time_point time) {
    if (m_device->IsSWVP()) {
      if (m_device->CanOnlySWVP()) {
        m_isSWVPText = "SWVP";
      } else {
        m_isSWVPText = "SWVP (Mixed)";
      }
    } else {
      if (m_device->CanSWVP()) {
        m_isSWVPText = "HWVP (Mixed)";
      } else {
        m_isSWVPText = "HWVP";
      }
    }
  }


  HudPos HudSWVPState::render(
    const DxvkContextObjects& ctx,
    const HudPipelineKey&     key,
    const HudOptions&         options,
          HudRenderer&        renderer,
          HudPos              position) {
    position.y += 16;
    renderer.drawText(16, position, 0xffc0ff00u, "Vertex Processing:");
    renderer.drawText(16, { position.x + 240, position.y }, 0xffffffffu, m_isSWVPText);

    position.y += 8;
    return position;
  }

}
