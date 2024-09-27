#include "dxvk_instance.h"
#include "dxvk_openxr.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif

using PFN___wineopenxr_GetVulkanInstanceExtensions = int (WINAPI *)(uint32_t, uint32_t *, char *);
using PFN___wineopenxr_GetVulkanDeviceExtensions = int (WINAPI *)(uint32_t, uint32_t *, char *);

namespace dxvk {
  
  struct WineXrFunctions {
    PFN___wineopenxr_GetVulkanInstanceExtensions __wineopenxr_GetVulkanInstanceExtensions = nullptr;
    PFN___wineopenxr_GetVulkanDeviceExtensions __wineopenxr_GetVulkanDeviceExtensions = nullptr;
  };
  
  WineXrFunctions g_winexrFunctions;
  DxvkXrProvider DxvkXrProvider::s_instance;

  DxvkXrProvider:: DxvkXrProvider() { }

  DxvkXrProvider::~DxvkXrProvider() { }


  std::string_view DxvkXrProvider::getName() {
    return "OpenXR";
  }
  
  
  DxvkNameSet DxvkXrProvider::getInstanceExtensions() {
    std::lock_guard<dxvk::mutex> lock(m_mutex);
    return m_insExtensions;
  }


  DxvkNameSet DxvkXrProvider::getDeviceExtensions(uint32_t adapterId) {
    std::lock_guard<dxvk::mutex> lock(m_mutex);
    return m_devExtensions;
  }


  void DxvkXrProvider::initInstanceExtensions() {
    std::lock_guard<dxvk::mutex> lock(m_mutex);
    if (m_initializedInsExt)
      return;

    m_insExtensions = this->queryInstanceExtensions();
    m_initializedInsExt = true;
  }


  bool DxvkXrProvider::loadFunctions() {
    g_winexrFunctions.__wineopenxr_GetVulkanInstanceExtensions =
        reinterpret_cast<PFN___wineopenxr_GetVulkanInstanceExtensions>(this->getSym("__wineopenxr_GetVulkanInstanceExtensions"));
    g_winexrFunctions.__wineopenxr_GetVulkanDeviceExtensions =
        reinterpret_cast<PFN___wineopenxr_GetVulkanDeviceExtensions>(this->getSym("__wineopenxr_GetVulkanDeviceExtensions"));
    return g_winexrFunctions.__wineopenxr_GetVulkanInstanceExtensions != nullptr
      && g_winexrFunctions.__wineopenxr_GetVulkanDeviceExtensions != nullptr;
  }


  void DxvkXrProvider::initDeviceExtensions(const DxvkInstance* instance) {
    std::lock_guard<dxvk::mutex> lock(m_mutex);
    if (m_initializedDevExt)
      return;
    
    m_devExtensions = this->queryDeviceExtensions();
    m_initializedDevExt = true;
  }


  DxvkNameSet DxvkXrProvider::queryInstanceExtensions() const {
    auto extensionList = env::getEnvVar("GTR2_XR_VK_INSTANCE_EXT_REQUIREMENTS");
    return parseExtensionList(extensionList, true /*instance*/);
  }
  
  
  DxvkNameSet DxvkXrProvider::queryDeviceExtensions() const {
    auto extensionList = env::getEnvVar("GTR2_XR_VK_DEVICE_EXT_REQUIREMENTS");
    return parseExtensionList(extensionList, false /*instance*/);
  }
  
  
  DxvkNameSet
  DxvkXrProvider::parseExtensionList(const std::string& str,
                                     bool instance) const
  {
    DxvkNameSet result;
    
    std::stringstream strstream(str);
    std::string       section;
    
    if (instance)
      Logger::info("OpenXR: Instance Extensions requested:");
    else
      Logger::info("OpenXR: Device Extensions requested:");

    while (std::getline(strstream, section, ' ')) {
      result.add(section.c_str());
      Logger::info(section);
    }
    
    return result;
  }
  
  
  void DxvkXrProvider::shutdown() {
    if (m_loadedOxrApi)
      this->freeLibrary();
    
    m_loadedOxrApi      = false;
    m_wineOxr = nullptr;
  }


  HMODULE DxvkXrProvider::loadLibrary() {
    HMODULE handle = ::LoadLibrary("wineopenxr.dll");

    m_loadedOxrApi = handle != nullptr;
    return handle;
  }


  void DxvkXrProvider::freeLibrary() {
    ::FreeLibrary(m_wineOxr);
  }

  
  void* DxvkXrProvider::getSym(const char* sym) {
    return reinterpret_cast<void*>(
      ::GetProcAddress(m_wineOxr, sym));
  }
}
