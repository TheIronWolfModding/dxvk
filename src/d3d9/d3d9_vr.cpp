#include "../dxvk/dxvk_include.h"

#include "d3d9_vr.h"

#include "d3d9_include.h"
#include "d3d9_surface.h"

#include "d3d9_device.h"

namespace dxvk {

class D3D9VR final : public ComObjectClamp<IDirect3DVR9>
{
public:
  D3D9VR(IDirect3DDevice9* pDevice)
    : m_device(static_cast<D3D9DeviceEx*>(pDevice))
  {}

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
  {
    if (ppvObject == nullptr)
      return E_POINTER;

    *ppvObject = nullptr;

    if (riid == __uuidof(IUnknown) || riid == __uuidof(IDirect3DVR9)) {
      *ppvObject = ref(this);
      return S_OK;
    }

    Logger::warn("D3D9VR::QueryInterface: Unknown interface query");
    Logger::warn(str::format(riid));
    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE GetVRDesc(IDirect3DSurface9* pSurface,
                                      D3D9_TEXTURE_VR_DESC* pDesc)
  {
    if (unlikely(pSurface == nullptr || pDesc == nullptr))
      return D3DERR_INVALIDCALL;

    D3D9Surface* surface = static_cast<D3D9Surface*>(pSurface);

    const auto* tex = surface->GetCommonTexture();

    const auto& desc = tex->Desc();
    const auto& image = tex->GetImage();
    const auto& device = tex->Device()->GetDXVKDevice();

    // I don't know why the image randomly is a uint64_t in OpenVR.
    pDesc->Image = uint64_t(image->handle());
    pDesc->Device = device->handle();
    pDesc->PhysicalDevice = device->adapter()->handle();
    pDesc->Instance = device->instance()->handle();
    pDesc->Queue = device->queues().graphics.queueHandle;
    pDesc->QueueFamilyIndex = device->queues().graphics.queueFamily;

    pDesc->Width = desc->Width;
    pDesc->Height = desc->Height;
    pDesc->Format = tex->GetFormatMapping().FormatColor;
    pDesc->SampleCount = uint32_t(image->info().sampleCount);

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE TransferSurfaceForVR(IDirect3DSurface9* pSurface)
  {
    if (unlikely(pSurface == nullptr))
      return D3DERR_INVALIDCALL;

    auto* tex = static_cast<D3D9Surface*>(pSurface)->GetCommonTexture();
    const auto& image = tex->GetImage();

    VkImageSubresourceRange subresources = { VK_IMAGE_ASPECT_COLOR_BIT,
                                             0,
                                             image->info().mipLevels,
                                             0,
                                             image->info().numLayers };

    m_device->TransformImage(tex,
                             &subresources,
                             image->info().layout,
                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE LockDevice()
  {
    m_lock = m_device->LockDevice();
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE UnlockDevice()
  {
    m_lock = D3D9DeviceLock();
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE WaitDeviceIdle(BOOL flush)
  {
    if (flush) {
      m_device->Flush();
      m_device->SynchronizeCsThread(DxvkCsThread::SynchronizeAll);
    }
    m_device->GetDXVKDevice()->waitForIdle();
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE WaitGraphicsQueueIdle(BOOL flush)
  {
    if (flush) {
      m_device->Flush();
      m_device->SynchronizeCsThread(DxvkCsThread::SynchronizeAll);
    }
    auto device = m_device->GetDXVKDevice();
    device->vkd()->vkQueueWaitIdle(device->queues().graphics.queueHandle);
    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE BeginVRSubmit()
  {
    m_device->Flush();
    m_device->SynchronizeCsThread(DxvkCsThread::SynchronizeAll);
    m_device->GetDXVKDevice()->lockSubmission();

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE EndVRSubmit()
  {
    m_device->GetDXVKDevice()->unlockSubmission();

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE
  GetOXRVkDeviceDesc(OXR_VK_DEVICE_DESC* vkDeviceDescOut)
  {
    if (unlikely(vkDeviceDescOut == nullptr))
      return D3DERR_INVALIDCALL;

    auto device = m_device->GetDXVKDevice();

    vkDeviceDescOut->Device = device->handle();
    vkDeviceDescOut->PhysicalDevice = device->adapter()->handle();
    vkDeviceDescOut->Instance = device->instance()->handle();
    vkDeviceDescOut->QueueIndex = device->queues().graphics.queueIndex;
    vkDeviceDescOut->QueueFamilyIndex = device->queues().graphics.queueFamily;

    return D3D_OK;
  }
  // FOVEATED?
  /*HRESULT STDMETHODCALLTYPE ImportFence(HANDLE handle, uint64_t value)
  {
    const DxvkFenceCreateInfo fenceInfo = { value, VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D11_FENCE_BIT, handle };
    m_fence = m_device->GetDXVKDevice()->createFence(fenceInfo);

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE SignalFence(uint64_t value)
  {
      m_fence->signal(value);
      return D3D_OK;
  }*/

  HRESULT STDMETHODCALLTYPE GetShaderHash(IDirect3DVertexShader9 *d3dShader, char** out)
  {
    D3D9Shader<IDirect3DVertexShader9>* shader = reinterpret_cast<D3D9Shader<IDirect3DVertexShader9>*>(d3dShader);
    D3D9CommonShader const* common = shader->GetCommonShader();
    Rc<DxvkShader> dxvkShader = common->GetShader();

    const auto shaderKey = dxvkShader->getShaderKey().toString();
    memcpy(out, shaderKey.c_str(), shaderKey.size());

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE GetSPIRVShaderCode(IDirect3DVertexShader9 *d3dShader, uint32_t* out, uint32_t* size)
  {
      D3D9Shader<IDirect3DVertexShader9>* shader = reinterpret_cast<D3D9Shader<IDirect3DVertexShader9>*>(d3dShader);
      D3D9CommonShader const* common = shader->GetCommonShader();
      Rc<DxvkShader> dxvkShader = common->GetShader();
      SpirvCodeBuffer code = dxvkShader->getRawCode();

      auto c = code.data();
      if (out != nullptr) {
          for(int i=0; i < code.dwords(); ++i) {
              out[i] = c[i];
          }
      }
      *size = code.dwords();

      return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE PatchSPIRVToVertexShader(IDirect3DVertexShader9 *d3dShader, const uint32_t* data, uint32_t size)
  {
      D3D9Shader<IDirect3DVertexShader9>* shader = reinterpret_cast<D3D9Shader<IDirect3DVertexShader9>*>(d3dShader);
      D3D9CommonShader const* common = shader->GetCommonShader();
      Rc<DxvkShader> dxvkShader = common->GetShader();
  
      SpirvCodeBuffer codeBuffer(size, data);
      DxvkShaderCreateInfo info = dxvkShader->info();

      // DXVK shaders copy the binding info into a more clever container
      // We need to dig it out here and pass it into the constructor again in a DxvkBindingInfo array
      auto bindings = dxvkShader->getBindings();
      DxvkBindingInfo* bindingsCopy;

	  if (info.bindingCount > 0) {
        bindingsCopy = reinterpret_cast<DxvkBindingInfo*>(malloc(sizeof(DxvkBindingInfo) * info.bindingCount));

        for (int i = 0; i < info.bindingCount; ++i)
        {
            bindingsCopy[i] = bindings.getBinding(DxvkDescriptorSets::VsAll, i);
        }

        info.bindings = bindingsCopy;
	  }

      *dxvkShader.ptr_mut() = DxvkShader(info, std::move(codeBuffer));

      return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE CreateMultiViewRenderTarget(
          UINT                Width,
          UINT                Height,
          D3DFORMAT           Format,
          D3DMULTISAMPLE_TYPE MultiSample,
          DWORD               MultisampleQuality,
          BOOL                Lockable,
          IDirect3DSurface9** ppSurface,
          HANDLE*             pSharedHandle,
          UINT                Views) {
    InitReturnPtr(ppSurface);

    if (unlikely(ppSurface == nullptr))
      return D3DERR_INVALIDCALL;

    D3D9_COMMON_TEXTURE_DESC desc;
    desc.Width              = Width;
    desc.Height             = Height;
    desc.Depth              = 1;
    desc.ArraySize          = Views;
    desc.MipLevels          = 1;
    desc.Usage              = D3DUSAGE_RENDERTARGET;
    desc.Format             = EnumerateFormat(Format);
    desc.Pool               = D3DPOOL_DEFAULT;
    desc.Discard            = FALSE;
    desc.MultiSample        = MultiSample;
    desc.MultisampleQuality = MultisampleQuality;
    desc.IsBackBuffer       = FALSE;
    desc.IsAttachmentOnly   = TRUE;
    desc.IsLockable         = Lockable;

    return m_device->CreateRenderTargetFromDesc(&desc, ppSurface, pSharedHandle);
  }

  HRESULT STDMETHODCALLTYPE CreateMultiViewDepthStencilSurface(
          UINT                Width,
          UINT                Height,
          D3DFORMAT           Format,
          D3DMULTISAMPLE_TYPE MultiSample,
          DWORD               MultisampleQuality,
          BOOL                Discard,
          IDirect3DSurface9** ppSurface,
          HANDLE*             pSharedHandle,
          UINT                Views) {
    InitReturnPtr(ppSurface);

    if (unlikely(ppSurface == nullptr))
      return D3DERR_INVALIDCALL;

    D3D9_COMMON_TEXTURE_DESC desc;
    desc.Width              = Width;
    desc.Height             = Height;
    desc.Depth              = 1;
    desc.ArraySize          = Views;
    desc.MipLevels          = 1;
    desc.Usage              = D3DUSAGE_DEPTHSTENCIL;
    desc.Format             = EnumerateFormat(Format);
    desc.Pool               = D3DPOOL_DEFAULT;
    desc.Discard            = Discard;
    desc.MultiSample        = MultiSample;
    desc.MultisampleQuality = MultisampleQuality;
    desc.IsBackBuffer       = FALSE;
    desc.IsAttachmentOnly   = TRUE;
    desc.IsLockable         = IsLockableDepthStencilFormat(desc.Format);

    return m_device->CreateRenderTargetFromDesc(&desc, ppSurface, pSharedHandle);
  }

  HRESULT STDMETHODCALLTYPE CreateMultiViewTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle, UINT Views)
  {
    D3D9_COMMON_TEXTURE_DESC desc;
    desc.Width              = Width;
    desc.Height             = Height;
    desc.Depth              = 1;
    desc.ArraySize          = Views;
    desc.MipLevels          = Levels;
    desc.Usage              = Usage;
    desc.Format             = EnumerateFormat(Format);
    desc.Pool               = Pool;
    desc.Discard            = FALSE;
    desc.MultiSample        = D3DMULTISAMPLE_NONE;
    desc.MultisampleQuality = 0;
    desc.IsBackBuffer       = FALSE;
    desc.IsAttachmentOnly   = FALSE;

    return m_device->CreateTextureFromDesc(&desc, ppTexture, pSharedHandle);
  }

  HRESULT STDMETHODCALLTYPE CopySurfaceLayers(IDirect3DSurface9 *srcSurface, IDirect3DSurface9** dsts, UINT layerCount)
  {
    // Assumes that `srcSurface` has `layerCount` layers and `dsts` contains `layerCount` of destination surfaces
    D3D9DeviceLock lock = m_device->LockDevice();
    D3D9Surface* src = static_cast<D3D9Surface*>(srcSurface);

    for(int i=0; i<layerCount; ++i) {
      D3D9Surface* dst = static_cast<D3D9Surface*>(dsts[i]);
      if (unlikely(src == nullptr || dst == nullptr))
        return D3DERR_INVALIDCALL;
      if (unlikely(src == dst))
        return D3DERR_INVALIDCALL;

      if (auto ret = m_device->StretchRectInternal(src, nullptr, dst, nullptr, D3DTEXF_NONE, i); ret != D3D_OK) {
          return ret;
      }
    }

    return D3D_OK;
  }

private:
  D3D9DeviceEx* m_device;
  D3D9DeviceLock m_lock;
};

}

HRESULT __stdcall Direct3DCreateVRImpl(IDirect3DDevice9* pDevice,
                                       IDirect3DVR9** pInterface)
{
  if (pInterface == nullptr)
    return D3DERR_INVALIDCALL;

  *pInterface = new dxvk::D3D9VR(pDevice);

  return D3D_OK;
}