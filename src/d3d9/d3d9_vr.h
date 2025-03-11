#pragma once

#include <d3d9.h>

#define VK_USE_PLATFORM_WIN32_KHR 1
#include <vulkan/vulkan.h>
#undef VK_USE_PLATFORM_WIN32_KHR

struct D3D9_TEXTURE_VR_DESC
{
  uint64_t Image;
  VkDevice Device;
  VkPhysicalDevice PhysicalDevice;
  VkInstance Instance;
  VkQueue Queue;
  uint32_t QueueFamilyIndex;

  uint32_t Width;
  uint32_t Height;
  VkFormat Format;
  uint32_t SampleCount;
};

struct OXR_VK_DEVICE_DESC
{
  VkDevice Device;
  VkPhysicalDevice PhysicalDevice;
  VkInstance Instance;
  uint32_t QueueIndex;
  uint32_t QueueFamilyIndex;
};

// Remember: this class is very similar to D3D9VkInteropDevice introduced later.
//           Keep an eye on that class for sync changes.
//
// Remember: add new functions at the end to improve back compat.
MIDL_INTERFACE("7e272b32-a49c-46c7-b1a4-ef52936bec87")
IDirect3DVR9 : public IUnknown
{
  virtual HRESULT STDMETHODCALLTYPE GetVRDesc(IDirect3DSurface9 * pSurface,
                                              D3D9_TEXTURE_VR_DESC * pDesc) = 0;
  virtual HRESULT STDMETHODCALLTYPE TransferSurfaceForVR(IDirect3DSurface9 *
                                                         pSurface) = 0;
  virtual HRESULT STDMETHODCALLTYPE BeginVRSubmit() = 0;
  virtual HRESULT STDMETHODCALLTYPE EndVRSubmit() = 0;
  virtual HRESULT STDMETHODCALLTYPE LockDevice() = 0;
  virtual HRESULT STDMETHODCALLTYPE UnlockDevice() = 0;

  // Wait* APIs are experimental.
  virtual HRESULT STDMETHODCALLTYPE WaitDeviceIdle(BOOL flush) = 0;
  virtual HRESULT STDMETHODCALLTYPE WaitGraphicsQueueIdle(BOOL flush) = 0;

  virtual HRESULT STDMETHODCALLTYPE GetOXRVkDeviceDesc(OXR_VK_DEVICE_DESC*
                                                       vkDeviceDescOut) = 0;

  // Deteger's work:
  //virtual HRESULT STDMETHODCALLTYPE CopySurfaceToVulkanImage(IDirect3DSurface9 * pSurface, VkImage dst, int64_t format, uint32_t dstWidth, uint32_t dstHeight) = 0;
  //virtual HRESULT STDMETHODCALLTYPE Flush() = 0;
  //virtual HRESULT STDMETHODCALLTYPE LockSubmissionQueue() = 0;
  //virtual HRESULT STDMETHODCALLTYPE UnlockSubmissionQueue() = 0;
  // Likely FOVeated work, not yet.
  //virtual HRESULT STDMETHODCALLTYPE ImportFence(HANDLE handle, uint64_t value) = 0;
  //virtual HRESULT STDMETHODCALLTYPE SignalFence(uint64_t value) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetShaderHash(IDirect3DVertexShader9 *d3dShader, char** out) = 0;
  virtual HRESULT STDMETHODCALLTYPE PatchSPIRVToVertexShader(IDirect3DVertexShader9 *d3dShader, const uint32_t* data, uint32_t size) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateMultiViewRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE*pSharedHandle, UINT Views) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateMultiViewDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE*pSharedHandle, UINT Views) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateMultiViewTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle, UINT Views) = 0;
  virtual HRESULT STDMETHODCALLTYPE CopySurfaceLayers(IDirect3DSurface9 *srcSurface, IDirect3DSurface9** dsts, UINT layerCount) = 0;
};
#ifdef _MSC_VER
struct __declspec(uuid("7e272b32-a49c-46c7-b1a4-ef52936bec87")) IDirect3DVR9;
#else
__CRT_UUID_DECL(IDirect3DVR9,
                0x7e272b32,
                0xa49c,
                0x46c7,
                0xb1,
                0xa4,
                0xef,
                0x52,
                0x93,
                0x6b,
                0xec,
                0x87);
#endif

HRESULT __stdcall Direct3DCreateVRImpl(IDirect3DDevice9* pDevice,
                                       IDirect3DVR9** pInterface);

HRESULT __stdcall CreateVertexShaderFromSPRIV(IDirect3DVertexShader9 *shader, const uint32_t* data, uint32_t size);

