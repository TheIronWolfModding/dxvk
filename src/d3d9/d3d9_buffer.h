#pragma once


#include "d3d9_resource.h"

#include "d3d9_common_buffer.h"
//#undef NDEBUG
#include <cassert>

extern IDirect3DVertexBuffer9* gVB;
namespace dxvk {

  template <typename... Type>
  class D3D9Buffer : public D3D9Resource<Type...> {

  public:

    D3D9Buffer(
            D3D9DeviceEx*      pDevice,
      const D3D9_BUFFER_DESC*  pDesc)
    : D3D9Resource<Type...> (pDevice),
      m_buffer              (pDevice, pDesc) {

    }

    HRESULT STDMETHODCALLTYPE Lock(
            UINT   OffsetToLock,
            UINT   SizeToLock,
            void** ppbData,
            DWORD  Flags) final {
      //if (Flags == 4096)  did not help.
        //Flags = 8192;
      if (this == (void*)gVB) {
        char buff[4096];
        ::sprintf(buff,
                  Flags == 8192
                    ? "DISCARD LOCK Offset: %d    Size: %d    Flags: %d\n"
                    : "LOCK Offset: %d    Size: %d    Flags: %d\n",
                  OffsetToLock,
                  SizeToLock,
                  Flags);
        // SOME COMPILER SETTING PREVENTS THIS FROM WORKING.
        ::OutputDebugStringA(buff);
        //if (Flags == 8192)
          //assert(false);
          //__debugbreak();
        //::DebugBreak();
        
      }

      return m_buffer.Lock(
        OffsetToLock,
        SizeToLock,
        ppbData,
        Flags);
    }

    HRESULT STDMETHODCALLTYPE Unlock() final {
      return m_buffer.Unlock();
    }

    void STDMETHODCALLTYPE PreLoad() final {
      m_buffer.PreLoad();
    }

    D3D9CommonBuffer* GetCommonBuffer() {
      return &m_buffer;
    }

  protected:

    D3D9CommonBuffer m_buffer;

  };


  using D3D9VertexBufferBase = D3D9Buffer<IDirect3DVertexBuffer9>;
  class D3D9VertexBuffer final : public D3D9VertexBufferBase {

  public:

    D3D9VertexBuffer(
            D3D9DeviceEx*      pDevice,
      const D3D9_BUFFER_DESC*  pDesc);

    HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID  riid,
            void** ppvObject);

    D3DRESOURCETYPE STDMETHODCALLTYPE GetType();

    HRESULT STDMETHODCALLTYPE GetDesc(D3DVERTEXBUFFER_DESC* pDesc);

  };

  using D3D9IndexBufferBase = D3D9Buffer<IDirect3DIndexBuffer9>;
  class D3D9IndexBuffer final : public D3D9IndexBufferBase {

  public:

    D3D9IndexBuffer(
            D3D9DeviceEx*      pDevice,
      const D3D9_BUFFER_DESC*  pDesc);

    HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID  riid,
            void** ppvObject);

    D3DRESOURCETYPE STDMETHODCALLTYPE GetType();

    HRESULT STDMETHODCALLTYPE GetDesc(D3DINDEXBUFFER_DESC* pDesc);

  };

  template <typename T>
  inline D3D9CommonBuffer* GetCommonBuffer(const T& pResource) {
    return pResource != nullptr ? pResource->GetCommonBuffer() : nullptr;
  }

}

//#define NDEBUG