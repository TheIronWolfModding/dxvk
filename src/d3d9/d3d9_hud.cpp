#include "d3d9_hud.h"

namespace dxvk::hud {

  HudSamplerCount::HudSamplerCount(D3D9DeviceEx* device)
    : m_device       (device)
    , m_samplerCount ("0"){

  }


  void HudSamplerCount::update(dxvk::high_resolution_clock::time_point time) {
    m_samplerCount = str::format(m_device->GetSamplerCount());
  }


  HudPos HudSamplerCount::render(
          HudRenderer&      renderer,
          HudPos            position) {
    position.y += 16.0f;

    renderer.drawText(16.0f,
      { position.x, position.y },
      { 0.0f, 1.0f, 0.75f, 1.0f },
      "Samplers:");

    renderer.drawText(16.0f,
      { position.x + 120.0f, position.y },
      { 1.0f, 1.0f, 1.0f, 1.0f },
      m_samplerCount);

    position.y += 8.0f;
    return position;
  }

  HudTextureMemory::HudTextureMemory(D3D9DeviceEx* device)
          : m_device          (device)
          , m_texturesAllocatedString ("")
          , m_texturesMappedString    ("")
          , m_buffersAllocatedString ("")
          , m_buffersMappedString    ("") {}


  void HudTextureMemory::update(dxvk::high_resolution_clock::time_point time) {
    D3D9MemoryAllocator* textureAllocator = m_device->GetTextureAllocator();
    m_maxTextureAllocated = std::max(m_maxTextureAllocated, textureAllocator->AllocatedMemory());
    m_maxTextureUsed = std::max(m_maxTextureUsed, textureAllocator->UsedMemory());
    m_maxTextureMapped = std::max(m_maxTextureMapped, textureAllocator->MappedMemory());

    D3D9MemoryAllocator* bufferAllocator = m_device->GetBufferAllocator();
    m_maxBufferAllocated = std::max(m_maxBufferAllocated, bufferAllocator->AllocatedMemory());
    m_maxBufferUsed = std::max(m_maxBufferUsed, bufferAllocator->UsedMemory());
    m_maxBufferMapped = std::max(m_maxBufferMapped, bufferAllocator->MappedMemory());

    /*static uint32_t peakMapped = 0u;
    if (textureAllocator->MappedMemory() > peakMapped)
      peakMapped = textureAllocator->MappedMemory();

    if ((::GetAsyncKeyState(VK_NUMPAD0) & 1) != 0)
      peakMapped = 0u;*/

    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(time - m_lastUpdate);

    if (elapsed.count() < UpdateInterval)
      return;

    m_texturesAllocatedString = str::format(std::setfill(' '), std::setw(5), m_maxTextureAllocated >> 20, " MB     ", std::setw(5), m_maxTextureUsed >> 20, " MB used");
    m_texturesMappedString = str::format(std::setfill(' '), std::setw(5), m_maxTextureMapped >> 20, " MB"/* (Peak: ", peakMapped >> 20, " MB)"*/);
    m_maxTextureAllocated = 0;
    m_maxTextureUsed = 0;
    m_maxTextureMapped = 0;

    m_buffersAllocatedString = str::format(std::setfill(' '), std::setw(5), m_maxBufferAllocated >> 20, " MB     ", std::setw(5), m_maxBufferUsed >> 20, " MB used");
    m_buffersMappedString = str::format(std::setfill(' '), std::setw(5), m_maxBufferMapped >> 20, " MB"/* (Peak: ", peakMapped >> 20, " MB)"*/);
    m_maxBufferAllocated = 0;
    m_maxBufferUsed = 0;
    m_maxBufferMapped = 0;

    m_lastUpdate = time;
  }


  HudPos HudTextureMemory::render(
          HudRenderer&      renderer,
          HudPos            position) {
    position.y += 16.0f;

    renderer.drawText(16.0f,
                      { position.x, position.y },
                      { 0.0f, 1.0f, 0.75f, 1.0f },
                      "Mappable Textures:");

    renderer.drawText(16.0f,
                      { position.x + 220.0f, position.y },
                      { 1.0f, 1.0f, 1.0f, 1.0f },
                      m_texturesAllocatedString);

    position.y += 24.0f;

    renderer.drawText(16.0f,
                      { position.x, position.y },
                      { 0.0f, 1.0f, 0.75f, 1.0f },
                      "Mapped Textures:");

    renderer.drawText(16.0f,
                      { position.x + 220.0f, position.y },
                      { 1.0f, 1.0f, 1.0f, 1.0f },
                      m_texturesMappedString);

    position.y += 8.0f;
    position.y += 16.0f;

    renderer.drawText(16.0f,
                      { position.x, position.y },
                      { 0.0f, 1.0f, 0.75f, 1.0f },
                      "Mappable Buffers:");

    renderer.drawText(16.0f,
                      { position.x + 220.0f, position.y },
                      { 1.0f, 1.0f, 1.0f, 1.0f },
                      m_buffersAllocatedString);

    position.y += 24.0f;

    renderer.drawText(16.0f,
                      { position.x, position.y },
                      { 0.0f, 1.0f, 0.75f, 1.0f },
                      "Mapped Buffers:");

    renderer.drawText(16.0f,
                      { position.x + 220.0f, position.y },
                      { 1.0f, 1.0f, 1.0f, 1.0f },
                      m_buffersMappedString);

    position.y += 8.0f;

    return position;
  }

}