#include "d3d9_common_buffer.h"

#include "d3d9_device.h"
#include "d3d9_util.h"

namespace dxvk {

  D3D9CommonBuffer::D3D9CommonBuffer(
          D3D9DeviceEx*      pDevice,
    const D3D9_BUFFER_DESC*  pDesc)
    : m_parent ( pDevice ), m_desc ( *pDesc ),
      m_mapMode(DetermineMapMode(pDevice->GetOptions())) {
    m_buffer = CreateBuffer();
    if (m_mapMode == D3D9_COMMON_BUFFER_MAP_MODE_BUFFER)
      m_stagingBuffer = CreateStagingBuffer();

    m_sliceHandle = GetMapBuffer()->getSliceHandle();

    if (m_desc.Pool != D3DPOOL_DEFAULT)
      m_dirtyRange = D3D9Range(0, m_desc.Size);
  }

  D3D9CommonBuffer::~D3D9CommonBuffer() {
    if (m_desc.Pool == D3DPOOL_DEFAULT)
      m_parent->DecrementLosableCounter();

    if (m_desc.Size != 0)
      m_parent->ChangeReportedMemory(m_desc.Size);

    m_parent->RemoveMappedBuffer(this);
  }


  HRESULT D3D9CommonBuffer::Lock(
          UINT   OffsetToLock,
          UINT   SizeToLock,
          void** ppbData,
          DWORD  Flags) {
    return m_parent->LockBuffer(
      this,
      OffsetToLock,
      SizeToLock,
      ppbData,
      Flags);
  }


  HRESULT D3D9CommonBuffer::Unlock() {
    return m_parent->UnlockBuffer(this);
  }

  // GTR2_SPECIFIC: This works reliably in GTR2, so keep it simple and ignore incoming changes.
  D3D9_COMMON_BUFFER_MAP_MODE D3D9CommonBuffer::DetermineMapMode(const D3D9Options* options) const {
    auto mm = (m_desc.Pool == D3DPOOL_DEFAULT && (m_desc.Usage & (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY)) && options->allowDirectBufferMapping)
      ? D3D9_COMMON_BUFFER_MAP_MODE_DIRECT
      : D3D9_COMMON_BUFFER_MAP_MODE_BUFFER;

    #ifdef D3D9_ALLOW_UNMAPPING
    if (likely(m_parent->GetOptions()->bufferMemory != 0) && mm == D3D9_COMMON_BUFFER_MAP_MODE_BUFFER)
      mm = D3D9_COMMON_BUFFER_MAP_MODE_UNMAPPABLE;
    #endif
    return mm;
  }


  HRESULT D3D9CommonBuffer::ValidateBufferProperties(const D3D9_BUFFER_DESC* pDesc) {
    if (pDesc->Size == 0)
      return D3DERR_INVALIDCALL;

    return D3D_OK;
  }


  void D3D9CommonBuffer::PreLoad() {
    if (IsPoolManaged(m_desc.Pool)) {
      auto lock = m_parent->LockDevice();

      if (NeedsUpload())
        m_parent->FlushBuffer(this);
    }
  }

  
  Rc<DxvkBuffer> D3D9CommonBuffer::CreateBuffer() const {
    DxvkBufferCreateInfo  info;
    info.size   = m_desc.Size;
    info.usage  = 0;
    info.stages = 0;
    info.access = 0;

    VkMemoryPropertyFlags memoryFlags = 0;

    if (m_desc.Type == D3DRTYPE_VERTEXBUFFER) {
      info.usage  |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
      info.stages |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
      info.access |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

      if (m_parent->SupportsSWVP()) {
        info.usage  |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        info.stages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
        info.access |= VK_ACCESS_SHADER_WRITE_BIT;
      }
    }
    else if (m_desc.Type == D3DRTYPE_INDEXBUFFER) {
      info.usage  |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
      info.stages |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
      info.access |= VK_ACCESS_INDEX_READ_BIT;
    }

    if (m_mapMode == D3D9_COMMON_BUFFER_MAP_MODE_DIRECT) {
      info.stages |= VK_PIPELINE_STAGE_HOST_BIT;
      info.access |= VK_ACCESS_HOST_WRITE_BIT;

      if (!(m_desc.Usage & D3DUSAGE_WRITEONLY))
        info.access |= VK_ACCESS_HOST_READ_BIT;

      memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                  |  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                  |  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    else {
      info.stages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
      info.usage  |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      info.access |= VK_ACCESS_TRANSFER_WRITE_BIT;

      memoryFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }

    if ((memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && m_parent->GetOptions()->cachedDynamicBuffers) {
      memoryFlags &= ~VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      memoryFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                  |  VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    }

    return m_parent->GetDXVKDevice()->createBuffer(info, memoryFlags);
  }


  Rc<DxvkBuffer> D3D9CommonBuffer::CreateStagingBuffer() const {
    DxvkBufferCreateInfo  info;
    info.size   = m_desc.Size;
    info.stages = VK_PIPELINE_STAGE_HOST_BIT
                | VK_PIPELINE_STAGE_TRANSFER_BIT;

    info.usage  = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    info.access = VK_ACCESS_HOST_WRITE_BIT
                | VK_ACCESS_TRANSFER_READ_BIT;

    if (!(m_desc.Usage & D3DUSAGE_WRITEONLY))
      info.access |= VK_ACCESS_HOST_READ_BIT;

    VkMemoryPropertyFlags memoryFlags = 
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

    return m_parent->GetDXVKDevice()->createBuffer(info, memoryFlags);
  }

  bool D3D9CommonBuffer::AllocData()
  {
      // TODO_MMF:
  /*if (m_mapMode != D3D9_COMMON_TEXTURE_MAP_MODE_UNMAPPABLE)
    return CreateBufferSubresource(Subresource);*/

    D3D9Memory& memory = m_data;
    if (likely(memory))
      return false;

    memory = m_parent->GetBufferAllocator()->Alloc(m_desc.Size);
    memory.Map();
    return true;
  }

  void* D3D9CommonBuffer::GetData()
  {
    // TODO_MMF:
    /*if (m_mapMode != D3D9_COMMON_TEXTURE_MAP_MODE_UNMAPPABLE)
      return m_mappedSlice.mapPtr;*/

    D3D9Memory& memory = m_data;
    memory.Map();
    return memory.Ptr();
  }

  }