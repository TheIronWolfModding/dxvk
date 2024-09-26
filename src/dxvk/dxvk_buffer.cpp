#include "dxvk_barrier.h"
#include "dxvk_buffer.h"
#include "dxvk_device.h"

#include <algorithm>

namespace dxvk {
  
  DxvkBuffer::DxvkBuffer(
          DxvkDevice*           device,
    const DxvkBufferCreateInfo& createInfo,
          DxvkMemoryAllocator&  allocator,
          VkMemoryPropertyFlags memFlags)
  : m_vkd           (device->vkd()),
    m_allocator     (&allocator),
    m_properties    (memFlags),
    m_shaderStages  (util::shaderStages(createInfo.stages)),
    m_info          (createInfo) {
    // Create and assign actual buffer resource
    assignSlice(allocateSlice());
  }


  DxvkBuffer::DxvkBuffer(
          DxvkDevice*           device,
    const DxvkBufferCreateInfo& createInfo,
    const DxvkBufferImportInfo& importInfo,
          DxvkMemoryAllocator&  allocator,
          VkMemoryPropertyFlags memFlags)
  : m_vkd           (device->vkd()),
    m_allocator     (&allocator),
    m_properties    (memFlags),
    m_shaderStages  (util::shaderStages(createInfo.stages)),
    m_info          (createInfo) {
    VkBufferCreateInfo info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    info.flags = m_info.flags;
    info.usage = m_info.usage;
    info.size = m_info.size;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    assignSlice(allocator.importBufferResource(info, importInfo));
  }


  DxvkBuffer::~DxvkBuffer() {

  }



  Rc<DxvkBufferView> DxvkBuffer::createView(
    const DxvkBufferViewCreateInfo& info) {
    DxvkBufferViewKey key = { };
    key.format = info.format;
    key.offset = info.rangeOffset;
    key.size = info.rangeLength;
    key.usage = info.usage;

    std::unique_lock lock(m_viewMutex);

    auto entry = m_views.emplace(std::piecewise_construct,
      std::make_tuple(key), std::make_tuple(this, key));

    return &entry.first->second;
  }


  DxvkSparsePageTable* DxvkBuffer::getSparsePageTable() {
    return m_storage->getSparsePageTable();
  }
  
}
