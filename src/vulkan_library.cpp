/// Copyright (c) 2019, FilipeCN.
///
/// The MIT License (MIT)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
/// \file vulkan_library.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-08-03
///
/// The contets were based on the Vulkan Cookbook 2017.
///
/// \brief Implementation of circe's vulkan library interface.

#include "vulkan_library.h"
#include <iostream>

#if defined __APPLE__
#include <dlfcn.h>
#endif

namespace circe {

std::string vulkanResultString(VkResult err) {
  switch (err) {
  case VK_SUCCESS:
    return "VK_SUCCESS Command successfully completed";
  case VK_NOT_READY:
    return "VK_NOT_READY A fence or query has not yet completed";
  case VK_TIMEOUT:
    return "VK_TIMEOUT A wait operation has not completed in the specified "
           "time";
  case VK_EVENT_SET:
    return "VK_EVENT_SET An event is signaled";
  case VK_EVENT_RESET:
    return "VK_EVENT_RESET An event is unsignaled";
  case VK_INCOMPLETE:
    return "VK_INCOMPLETE A return array was too small for the result";
  case VK_SUBOPTIMAL_KHR:
    return "VK_SUBOPTIMAL_KHR A swapchain no longer matches the surface "
           "properties exactly, but can still be used to present to the "
           "surface successfully.";
  case VK_ERROR_OUT_OF_HOST_MEMORY:
    return "VK_ERROR_OUT_OF_HOST_MEMORY A host memory allocation has failed.";
  case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    return "VK_ERROR_OUT_OF_DEVICE_MEMORY A device memory allocation has "
           "failed.";
  case VK_ERROR_INITIALIZATION_FAILED:
    return "VK_ERROR_INITIALIZATION_FAILED Initialization of an object could "
           "not be completed for implementation-specific reasons.";
  case VK_ERROR_DEVICE_LOST:
    return "VK_ERROR_DEVICE_LOST The logical or physical device has been lost. "
           "See Lost Device";
  case VK_ERROR_MEMORY_MAP_FAILED:
    return "VK_ERROR_MEMORY_MAP_FAILED Mapping of a memory object has failed.";
  case VK_ERROR_LAYER_NOT_PRESENT:
    return "VK_ERROR_LAYER_NOT_PRESENT A requested layer is not present or "
           "could not be loaded.";
  case VK_ERROR_EXTENSION_NOT_PRESENT:
    return "VK_ERROR_EXTENSION_NOT_PRESENT A requested extension is not "
           "supported.";
  case VK_ERROR_FEATURE_NOT_PRESENT:
    return "VK_ERROR_FEATURE_NOT_PRESENT A requested feature is not supported.";
  case VK_ERROR_INCOMPATIBLE_DRIVER:
    return "VK_ERROR_INCOMPATIBLE_DRIVER The requested version of Vulkan is "
           "not supported by the driver or is otherwise incompatible for "
           "implementation-specific reasons.";
  case VK_ERROR_TOO_MANY_OBJECTS:
    return "VK_ERROR_TOO_MANY_OBJECTS Too many objects of the type have "
           "already been created.";
  case VK_ERROR_FORMAT_NOT_SUPPORTED:
    return "VK_ERROR_FORMAT_NOT_SUPPORTED A requested format is not supported "
           "on this device.";
  case VK_ERROR_FRAGMENTED_POOL:
    return "VK_ERROR_FRAGMENTED_POOL A pool allocation has failed due to "
           "fragmentation of the pool’s memory. This must only be returned if "
           "no attempt to allocate host or device memory was made to "
           "accommodate the new allocation. This should be returned in "
           "preference to VK_ERROR_OUT_OF_POOL_MEMORY, but only if the "
           "implementation is certain that the pool allocation failure was due "
           "to fragmentation.";
  case VK_ERROR_SURFACE_LOST_KHR:
    return "VK_ERROR_SURFACE_LOST_KHR A surface is no longer available.";
  case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
    return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR The requested window is already "
           "in use by Vulkan or another API in a manner which prevents it from "
           "being used again.";
  case VK_ERROR_OUT_OF_DATE_KHR:
    return "VK_ERROR_OUT_OF_DATE_KHR A surface has changed in such a way that "
           "it is no longer compatible with the swapchain, and further "
           "presentation requests using the swapchain will fail. Applications "
           "must query the new surface properties and recreate their swapchain "
           "if they wish to continue presenting to the surface.";
  case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
    return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR The display used by a swapchain "
           "does not use the same presentable image layout, or is incompatible "
           "in a way that prevents sharing an image.";
  case VK_ERROR_INVALID_SHADER_NV:
    return "VK_ERROR_INVALID_SHADER_NV One or more shaders failed to compile "
           "or link. More details are reported back to the application via "
           "https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/"
           "vkspec.html#VK_EXT_debug_report if enabled.";
  case VK_ERROR_OUT_OF_POOL_MEMORY:
    return "VK_ERROR_OUT_OF_POOL_MEMORY A pool memory allocation has failed. "
           "This must only be returned if no attempt to allocate host or "
           "device memory was made to accommodate the new allocation. If the "
           "failure was definitely due to fragmentation of the pool, "
           "VK_ERROR_FRAGMENTED_POOL should be returned instead.";
  case VK_ERROR_INVALID_EXTERNAL_HANDLE:
    return "VK_ERROR_INVALID_EXTERNAL_HANDLE An external handle is not a valid "
           "handle of the specified type.";
  case VK_ERROR_FRAGMENTATION_EXT:
    return "VK_ERROR_FRAGMENTATION_EXT A descriptor pool creation has failed "
           "due to fragmentation.";
  case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
    return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT A buffer creation failed "
           "because the requested address is not available.";
  case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
    return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT An operation on a "
           "swapchain created with "
           "VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT failed as it "
           "did not have exlusive full-screen access. This may occur due to "
           "implementation-dependent reasons, outside of the application’s "
           "control.";
  // case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
  //   return "VK_ERROR_OUT_OF_POOL_MEMORY_KHR";
  case VK_ERROR_VALIDATION_FAILED_EXT:
    return "VK_ERROR_VALIDATION_FAILED_EXT";
  case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
    return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
  case VK_ERROR_NOT_PERMITTED_EXT:
    return "VK_ERROR_NOT_PERMITTED_EXT";
  // case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR:
  //   return "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR";
  default:
    return "UNDEFINED";
  }
  return "UNDEFINED";
}

bool handleVulkanError(VkResult err, const char *file, int line) {
  if (err != VK_SUCCESS) {
    std::cerr << "[VULKAN_ERROR] in [" << file << "][" << line
              << "]: " << vulkanResultString(err) << std::endl;
    return false;
  }
  return true;
}

#define CHECK_VULKAN(err, M)                                                   \
  if (!handleVulkanError(err, __FILE__, __LINE__)) {                           \
    std::cerr << (M) << std::endl;                                             \
    return false;                                                              \
  }

#define CHECK(A, M)                                                            \
  if (!(A)) {                                                                  \
    std::cerr << "[ERROR] in [" << __FILE__ << "][" << __LINE__                \
              << "]: " << (M) << std::endl;                                    \
    return false;                                                              \
  }

// VULKAN API FUNCTION LOADING /////////////////////////////////////////////////

bool VulkanLibraryInterface::loadLoaderFunctionFromVulkan(
    VulkanLibraryType &vulkanLibrary) {
#if defined _WIN32
#define LoadFunction GetProcAddress
#elif defined __linux
#define LoadFunction dlsym
#elif defined __APPLE__
#define LoadFunction dlsym
#endif

#define EXPORTED_VULKAN_FUNCTION(name)                                         \
  name = (PFN_##name)LoadFunction(vulkanLibrary, #name);                       \
  CHECK(name != nullptr,                                                       \
        concat("Could not load exported Vulkan function named: ", #name))
#include "vulkan_api.inl"
  return true;
}

void VulkanLibraryInterface::releaseVulkanLoaderLibrary(
    VulkanLibraryType &vulkan_library) {
  if (nullptr != vulkan_library) {
#if defined _WIN32
    FreeLibrary(vulkan_library);
#elif defined __linux
    dlclose(vulkan_library);
#elfif defined __APPLE__
    dlclose(vulkan_library);
#endif
    vulkan_library = nullptr;
  }
}

bool VulkanLibraryInterface::loadGlobalLevelFunctions() {
#define GLOBAL_LEVEL_VULKAN_FUNCTION(name)                                     \
  name = (PFN_##name)vkGetInstanceProcAddr(nullptr, #name);                    \
  CHECK(name != nullptr,                                                       \
        concat("Could not load global-level function named: ", #name))
#include "vulkan_api.inl"
  return true;
}

bool VulkanLibraryInterface::loadInstanceLevelFunctions(
    VkInstance instance, const std::vector<const char *> &extensions) {
// Load core Vulkan API instance-level functions
#define INSTANCE_LEVEL_VULKAN_FUNCTION(name)                                   \
  name = (PFN_##name)vkGetInstanceProcAddr(instance, #name);                   \
  CHECK(                                                                       \
      name != nullptr,                                                         \
      concat("Could not load instance-level Vulkan function named: ", #name))

  // Load instance-level functions from enabled extensions
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)         \
  for (auto &ext : extensions) {                                               \
    if (std::string(ext) == std::string(extension)) {                          \
      name = (PFN_##name)vkGetInstanceProcAddr(instance, #name);               \
      CHECK(name != nullptr,                                                   \
            concat("Could not load instance-level Vulkan function named: ",    \
                   #name))                                                     \
    }                                                                          \
  }

#include "vulkan_api.inl"

  return true;
}

bool VulkanLibraryInterface::loadDeviceLevelFunctions(
    VkDevice logical_device,
    std::vector<char const *> const &enabled_extensions) {
  // Load core Vulkan API device-level functions
#define DEVICE_LEVEL_VULKAN_FUNCTION(name)                                     \
  name = (PFN_##name)vkGetDeviceProcAddr(logical_device, #name);               \
  if (name == nullptr) {                                                       \
    std::cout << "Could not load device-level Vulkan function named: " #name   \
              << std::endl;                                                    \
    return false;                                                              \
  }

  // Load device-level functions from enabled extensions
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)           \
  for (auto &enabled_extension : enabled_extensions) {                         \
    if (std::string(enabled_extension) == std::string(extension)) {            \
      name = (PFN_##name)vkGetDeviceProcAddr(logical_device, #name);           \
      if (name == nullptr) {                                                   \
        std::cout                                                              \
            << "Could not load device-level Vulkan function named: " #name     \
            << std::endl;                                                      \
        return false;                                                          \
      }                                                                        \
    }                                                                          \
  }

#include "vulkan_api.inl"

  return true;
}

// VULKAN API EXTENSIONS //////////////////////////////////////////////////////

bool VulkanLibraryInterface::checkAvaliableInstanceExtensions(
    std::vector<VkExtensionProperties> &extensions) {
  uint32_t extensionsCount = 0;
  VkResult result = VK_SUCCESS;
  result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount,
                                                  nullptr);
  CHECK(result == VK_SUCCESS && extensionsCount != 0,
        "Could not get the number of Instance extensions.");
  extensions.resize(extensionsCount);
  result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount,
                                                  &extensions[0]);
  CHECK(result == VK_SUCCESS && extensionsCount != 0,
        "Could enumerate Instance extensisons.");
  return true;
}

bool VulkanLibraryInterface::checkAvailableDeviceExtensions(
    VkPhysicalDevice physicalDevice,
    std::vector<VkExtensionProperties> &extensions) {
  uint32_t extensionsCount = 0;
  VkResult result = VK_SUCCESS;
  result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr,
                                                &extensionsCount, nullptr);
  CHECK(result == VK_SUCCESS && extensionsCount == 0,
        "Could not get the number of device extensions.");

  extensions.resize(extensionsCount);
  result = vkEnumerateDeviceExtensionProperties(
      physicalDevice, nullptr, &extensionsCount, extensions.data());
  CHECK(result == VK_SUCCESS && extensionsCount == 0,
        "Could not enumerate device extensions.");

  return true;
}

bool VulkanLibraryInterface::isExtensionSupported(
    const std::vector<VkExtensionProperties> &extensions,
    const char *extensionName) {
  for (auto &extension : extensions)
    if (extension.extensionName == extensionName)
      return true;
  return false;
}

// VULKAN INSTANCE ////////////////////////////////////////////////////////////

bool VulkanLibraryInterface::createInstance(
    const std::vector<const char *> &extensions, std::string applicationName,
    VkInstance &instance) {
  std::vector<VkExtensionProperties> instanceExtensions;
  if (!checkAvaliableInstanceExtensions(instanceExtensions))
    return false;
  for (auto &extension : extensions)
    CHECK(isExtensionSupported(instanceExtensions, extension),
          concat("Extension named '", extension, "' is not supported."));
  VkApplicationInfo info;
  info.pApplicationName = applicationName.c_str();
  info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  info.apiVersion = VK_MAKE_VERSION(1, 0, 0);
  info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  info.pNext = nullptr;
  info.pEngineName = "circe";
  VkInstanceCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.flags = 0;
  createInfo.pApplicationInfo = &info;
  createInfo.enabledLayerCount = 0;
  createInfo.ppEnabledLayerNames = nullptr;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames =
      (extensions.size()) ? extensions.data() : nullptr;
  CHECK_VULKAN(vkCreateInstance(&createInfo, nullptr, &instance),
               "Could not create Vulkan Instance.");
  CHECK(instance != VK_NULL_HANDLE, "Could not create Vulkan Instance.");
  return true;
}

void VulkanLibraryInterface::destroyVulkanInstance(VkInstance &instance) {
  if (instance) {
    vkDestroyInstance(instance, nullptr);
    instance = VK_NULL_HANDLE;
  }
}
// VULKAN PHYSICAL DEVICES ////////////////////////////////////////////////////

bool VulkanLibraryInterface::enumerateAvailablePhysicalDevices(
    VkInstance instance, std::vector<VkPhysicalDevice> &devices) {
  uint32_t devicesCount = 0;
  VkResult result = VK_SUCCESS;

  result = vkEnumeratePhysicalDevices(instance, &devicesCount, nullptr);
  CHECK(result == VK_SUCCESS && devicesCount != 0,
        "Could not get the number of available physical devices.");

  devices.resize(devicesCount);
  result = vkEnumeratePhysicalDevices(instance, &devicesCount, devices.data());
  CHECK(result == VK_SUCCESS && devicesCount != 0,
        "Could not enumerate physical devices.");

  return true;
}

void VulkanLibraryInterface::getFeaturesAndPropertiesOfPhysicalDevice(
    VkPhysicalDevice physical_device, VkPhysicalDeviceFeatures &device_features,
    VkPhysicalDeviceProperties &device_properties) {
  vkGetPhysicalDeviceFeatures(physical_device, &device_features);

  vkGetPhysicalDeviceProperties(physical_device, &device_properties);
}

// VULKAN QUEUE FAMILIES //////////////////////////////////////////////////////

bool VulkanLibraryInterface::checkAvailableQueueFamiliesAndTheirProperties(
    VkPhysicalDevice physical_device,
    std::vector<VkQueueFamilyProperties> &queue_families) {
  uint32_t queue_families_count = 0;

  vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                           &queue_families_count, nullptr);
  CHECK(queue_families_count != 0,
        "Could not get the number of queue families.\n");

  queue_families.resize(queue_families_count);
  vkGetPhysicalDeviceQueueFamilyProperties(
      physical_device, &queue_families_count, queue_families.data());
  CHECK(queue_families_count != 0,
        "Could not acquire properties of queue families.\n");

  return true;
}

bool VulkanLibraryInterface::selectIndexOfQueueFamilyWithDesiredCapabilities(
    VkPhysicalDevice physical_device, VkQueueFlags desired_capabilities,
    uint32_t &queue_family_index) {
  std::vector<VkQueueFamilyProperties> queue_families;
  CHECK(checkAvailableQueueFamiliesAndTheirProperties(physical_device,
                                                      queue_families),
        "");

  for (uint32_t index = 0; index < static_cast<uint32_t>(queue_families.size());
       ++index) {
    if ((queue_families[index].queueCount > 0) &&
        ((queue_families[index].queueFlags & desired_capabilities) ==
         desired_capabilities)) {
      queue_family_index = index;
      return true;
    }
  }
  return false;
}

// VULKAN LOGICAL DEVICE /////////////////////////////////////////////////////

bool VulkanLibraryInterface::createLogicalDevice(
    VkPhysicalDevice physical_device, std::vector<QueueInfo> queue_infos,
    std::vector<char const *> const &desired_extensions,
    VkPhysicalDeviceFeatures *desired_features, VkDevice &logical_device) {
  std::vector<VkExtensionProperties> available_extensions;
  CHECK(checkAvailableDeviceExtensions(physical_device, available_extensions),
        "");

  for (auto &extension : desired_extensions)
    CHECK(isExtensionSupported(available_extensions, extension),
          concat("Extension named '", extension,
                 "' is not supported by a physical device.\n"));

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

  for (auto &info : queue_infos) {
    queue_create_infos.push_back({
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // VkStructureType sType
        nullptr,           // const void                     * pNext
        0,                 // VkDeviceQueueCreateFlags         flags
        info.family_index, // uint32_t                         queueFamilyIndex
        static_cast<uint32_t>(info.priorities.size()), // uint32_t queueCount
        info.priorities
            .data() // const float                    * pQueuePriorities
    });
  };

  VkDeviceCreateInfo device_create_info = {
      VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, // VkStructureType sType
      nullptr, // const void                     * pNext
      0,       // VkDeviceCreateFlags              flags
      static_cast<uint32_t>(
          queue_create_infos.size()), // uint32_t queueCreateInfoCount
      queue_create_infos
          .data(), // const VkDeviceQueueCreateInfo  * pQueueCreateInfos
      0,           // uint32_t                         enabledLayerCount
      nullptr,     // const char * const             * ppEnabledLayerNames
      static_cast<uint32_t>(
          desired_extensions.size()), // uint32_t enabledExtensionCount
      desired_extensions
          .data(), // const char * const             * ppEnabledExtensionNames
      desired_features // const VkPhysicalDeviceFeatures * pEnabledFeatures
  };

  VkResult result = vkCreateDevice(physical_device, &device_create_info,
                                   nullptr, &logical_device);
  CHECK((result == VK_SUCCESS) && (logical_device != VK_NULL_HANDLE),
        "Could not create logical device.");
  return true;
}

void VulkanLibraryInterface::destroyLogicalDevice(VkDevice &logical_device) {
  if (logical_device) {
    vkDestroyDevice(logical_device, nullptr);
    logical_device = VK_NULL_HANDLE;
  }
}

void VulkanLibraryInterface::getDeviceQueue(VkDevice logical_device,
                                            uint32_t queue_family_index,
                                            uint32_t queue_index,
                                            VkQueue &queue) {
  vkGetDeviceQueue(logical_device, queue_family_index, queue_index, &queue);
}

bool VulkanLibraryInterface::
    createLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues(
        VkInstance instance, VkDevice &logical_device, VkQueue &graphics_queue,
        VkQueue &compute_queue) {
  std::vector<VkPhysicalDevice> physical_devices;
  enumerateAvailablePhysicalDevices(instance, physical_devices);
  for (auto &physical_device : physical_devices) {
    VkPhysicalDeviceFeatures device_features;
    VkPhysicalDeviceProperties device_properties;
    getFeaturesAndPropertiesOfPhysicalDevice(physical_device, device_features,
                                             device_properties);

    if (!device_features.geometryShader) {
      continue;
    } else {
      device_features = {};
      device_features.geometryShader = VK_TRUE;
    }

    uint32_t graphics_queue_family_index;
    if (!selectIndexOfQueueFamilyWithDesiredCapabilities(
            physical_device, VK_QUEUE_GRAPHICS_BIT,
            graphics_queue_family_index)) {
      continue;
    }

    uint32_t compute_queue_family_index;
    if (!selectIndexOfQueueFamilyWithDesiredCapabilities(
            physical_device, VK_QUEUE_COMPUTE_BIT,
            compute_queue_family_index)) {
      continue;
    }

    std::vector<QueueInfo> requested_queues = {
        {graphics_queue_family_index, {1.0f}}};
    if (graphics_queue_family_index != compute_queue_family_index) {
      requested_queues.push_back({compute_queue_family_index, {1.0f}});
    }

    if (!createLogicalDevice(physical_device, requested_queues, {},
                             &device_features, logical_device)) {
      continue;
    } else {
      if (!loadDeviceLevelFunctions(logical_device, {})) {
        return false;
      }
      getDeviceQueue(logical_device, graphics_queue_family_index, 0,
                     graphics_queue);
      getDeviceQueue(logical_device, compute_queue_family_index, 0,
                     compute_queue);
      return true;
    }
  }
  return false;
}

bool VulkanLibraryInterface::createVulkanInstanceWithWsiExtensionsEnabled(
    std::vector<char const *> &desired_extensions,
    char const *const application_name, VkInstance &instance) {
  desired_extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
  desired_extensions.emplace_back(
#ifdef VK_USE_PLATFORM_WIN32_KHR
      VK_KHR_WIN32_SURFACE_EXTENSION_NAME

#elif defined VK_USE_PLATFORM_XCB_KHR
      VK_KHR_XCB_SURFACE_EXTENSION_NAME

#elif defined VK_USE_PLATFORM_XLIB_KHR
      VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
  );

  return createInstance(desired_extensions, application_name, instance);
}

bool VulkanLibraryInterface::createPresentationSurface(
    VkInstance instance, WindowParameters window_parameters,
    VkSurfaceKHR &presentation_surface) {
  VkResult result;

#ifdef VK_USE_PLATFORM_WIN32_KHR

  VkWin32SurfaceCreateInfoKHR surface_create_info = {
      VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, // VkStructureType sType
      nullptr,                     // const void                    * pNext
      0,                           // VkWin32SurfaceCreateFlagsKHR    flags
      window_parameters.HInstance, // HINSTANCE                       hinstance
      window_parameters.HWnd       // HWND                            hwnd
  };

  result = vkCreateWin32SurfaceKHR(instance, &surface_create_info, nullptr,
                                   &presentation_surface);

#elif defined VK_USE_PLATFORM_XLIB_KHR

  VkXlibSurfaceCreateInfoKHR surface_create_info = {
      VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR, // VkStructureType sType
      nullptr,                 // const void                    * pNext
      0,                       // VkXlibSurfaceCreateFlagsKHR     flags
      window_parameters.Dpy,   // Display                       * dpy
      window_parameters.Window // Window                          window
  };

  result = vkCreateXlibSurfaceKHR(instance, &surface_create_info, nullptr,
                                  &presentation_surface);

#elif defined VK_USE_PLATFORM_XCB_KHR

  VkXcbSurfaceCreateInfoKHR surface_create_info = {
      VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR, // VkStructureType sType
      nullptr, // const void                    * pNext
      0,       // VkXcbSurfaceCreateFlagsKHR      flags
      window_parameters
          .Connection,         // xcb_connection_t              * connection
      window_parameters.Window // xcb_window_t                    window
  };

  result = vkCreateXcbSurfaceKHR(instance, &surface_create_info, nullptr,
                                 &presentation_surface);

#endif

  if ((VK_SUCCESS != result) || (VK_NULL_HANDLE == presentation_surface)) {
    std::cout << "Could not create presentation surface." << std::endl;
    return false;
  }
  return true;
}

bool VulkanLibraryInterface::
    selectQueueFamilyThatSupportsPresentationToGivenSurface(
        VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
        uint32_t &queue_family_index) {
  std::vector<VkQueueFamilyProperties> queue_families;
  if (!checkAvailableQueueFamiliesAndTheirProperties(physical_device,
                                                     queue_families)) {
    return false;
  }

  for (uint32_t index = 0; index < static_cast<uint32_t>(queue_families.size());
       ++index) {
    VkBool32 presentation_supported = VK_FALSE;
    VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
        physical_device, index, presentation_surface, &presentation_supported);
    if ((VK_SUCCESS == result) && (VK_TRUE == presentation_supported)) {
      queue_family_index = index;
      return true;
    }
  }
  return false;
}

bool VulkanLibraryInterface::createLogicalDeviceWithWsiExtensionsEnabled(
    VkPhysicalDevice physical_device, std::vector<QueueInfo> queue_infos,
    std::vector<char const *> &desired_extensions,
    VkPhysicalDeviceFeatures *desired_features, VkDevice &logical_device) {
  desired_extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  return createLogicalDevice(physical_device, queue_infos, desired_extensions,
                             desired_features, logical_device);
}

bool VulkanLibraryInterface::selectDesiredPresentationMode(
    VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
    VkPresentModeKHR desired_present_mode, VkPresentModeKHR &present_mode) {
  // Enumerate supported present modes
  uint32_t present_modes_count = 0;
  VkResult result = VK_SUCCESS;

  result = vkGetPhysicalDeviceSurfacePresentModesKHR(
      physical_device, presentation_surface, &present_modes_count, nullptr);
  if ((VK_SUCCESS != result) || (0 == present_modes_count)) {
    std::cout << "Could not get the number of supported present modes."
              << std::endl;
    return false;
  }

  std::vector<VkPresentModeKHR> present_modes(present_modes_count);
  result = vkGetPhysicalDeviceSurfacePresentModesKHR(
      physical_device, presentation_surface, &present_modes_count,
      present_modes.data());
  if ((VK_SUCCESS != result) || (0 == present_modes_count)) {
    std::cout << "Could not enumerate present modes." << std::endl;
    return false;
  }

  // Select present mode
  for (auto &current_present_mode : present_modes) {
    if (current_present_mode == desired_present_mode) {
      present_mode = desired_present_mode;
      return true;
    }
  }

  std::cout
      << "Desired present mode is not supported. Selecting default FIFO mode."
      << std::endl;
  for (auto &current_present_mode : present_modes) {
    if (current_present_mode == VK_PRESENT_MODE_FIFO_KHR) {
      present_mode = VK_PRESENT_MODE_FIFO_KHR;
      return true;
    }
  }

  std::cout << "VK_PRESENT_MODE_FIFO_KHR is not supported though it's "
               "mandatory for all drivers!"
            << std::endl;
  return false;
}

bool VulkanLibraryInterface::getCapabilitiesOfPresentationSurface(
    VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
    VkSurfaceCapabilitiesKHR &surface_capabilities) {
  VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physical_device, presentation_surface, &surface_capabilities);

  if (VK_SUCCESS != result) {
    std::cout << "Could not get the capabilities of a presentation surface."
              << std::endl;
    return false;
  }
  return true;
}

bool VulkanLibraryInterface::selectNumberOfSwapchainImages(
    VkSurfaceCapabilitiesKHR const &surface_capabilities,
    uint32_t &number_of_images) {
  number_of_images = surface_capabilities.minImageCount + 1;
  if ((surface_capabilities.maxImageCount > 0) &&
      (number_of_images > surface_capabilities.maxImageCount)) {
    number_of_images = surface_capabilities.maxImageCount;
  }
  return true;
}

bool VulkanLibraryInterface::chooseSizeOfSwapchainImages(
    VkSurfaceCapabilitiesKHR const &surface_capabilities,
    VkExtent2D &size_of_images) {
  if (0xFFFFFFFF == surface_capabilities.currentExtent.width) {
    size_of_images = {640, 480};

    if (size_of_images.width < surface_capabilities.minImageExtent.width) {
      size_of_images.width = surface_capabilities.minImageExtent.width;
    } else if (size_of_images.width >
               surface_capabilities.maxImageExtent.width) {
      size_of_images.width = surface_capabilities.maxImageExtent.width;
    }

    if (size_of_images.height < surface_capabilities.minImageExtent.height) {
      size_of_images.height = surface_capabilities.minImageExtent.height;
    } else if (size_of_images.height >
               surface_capabilities.maxImageExtent.height) {
      size_of_images.height = surface_capabilities.maxImageExtent.height;
    }
  } else {
    size_of_images = surface_capabilities.currentExtent;
  }
  return true;
}

bool VulkanLibraryInterface::selectDesiredUsageScenariosOfSwapchainImages(
    VkSurfaceCapabilitiesKHR const &surface_capabilities,
    VkImageUsageFlags desired_usages, VkImageUsageFlags &image_usage) {
  image_usage = desired_usages & surface_capabilities.supportedUsageFlags;

  return desired_usages == image_usage;
}

bool VulkanLibraryInterface::selectTransformationOfSwapchainImages(
    VkSurfaceCapabilitiesKHR const &surface_capabilities,
    VkSurfaceTransformFlagBitsKHR desired_transform,
    VkSurfaceTransformFlagBitsKHR &surface_transform) {
  if (surface_capabilities.supportedTransforms & desired_transform) {
    surface_transform = desired_transform;
  } else {
    surface_transform = surface_capabilities.currentTransform;
  }
  return true;
}

bool VulkanLibraryInterface::selectFormatOfSwapchainImages(
    VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
    VkSurfaceFormatKHR desired_surface_format, VkFormat &image_format,
    VkColorSpaceKHR &image_color_space) {
  // Enumerate supported formats
  uint32_t formats_count = 0;
  VkResult result = VK_SUCCESS;

  result = vkGetPhysicalDeviceSurfaceFormatsKHR(
      physical_device, presentation_surface, &formats_count, nullptr);
  if ((VK_SUCCESS != result) || (0 == formats_count)) {
    std::cout << "Could not get the number of supported surface formats."
              << std::endl;
    return false;
  }

  std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
  result = vkGetPhysicalDeviceSurfaceFormatsKHR(
      physical_device, presentation_surface, &formats_count,
      surface_formats.data());
  if ((VK_SUCCESS != result) || (0 == formats_count)) {
    std::cout << "Could not enumerate supported surface formats." << std::endl;
    return false;
  }

  // Select surface format
  if ((1 == surface_formats.size()) &&
      (VK_FORMAT_UNDEFINED == surface_formats[0].format)) {
    image_format = desired_surface_format.format;
    image_color_space = desired_surface_format.colorSpace;
    return true;
  }

  for (auto &surface_format : surface_formats) {
    if (desired_surface_format.format == surface_format.format &&
        desired_surface_format.colorSpace == surface_format.colorSpace) {
      image_format = desired_surface_format.format;
      image_color_space = desired_surface_format.colorSpace;
      return true;
    }
  }

  for (auto &surface_format : surface_formats) {
    if (desired_surface_format.format == surface_format.format) {
      image_format = desired_surface_format.format;
      image_color_space = surface_format.colorSpace;
      std::cout << "Desired combination of format and colorspace is not "
                   "supported. Selecting other colorspace."
                << std::endl;
      return true;
    }
  }

  image_format = surface_formats[0].format;
  image_color_space = surface_formats[0].colorSpace;
  std::cout << "Desired format is not supported. Selecting available format - "
               "colorspace combination."
            << std::endl;
  return true;
}

bool VulkanLibraryInterface::createSwapchain(
    VkDevice logical_device, VkSurfaceKHR presentation_surface,
    uint32_t image_count, VkSurfaceFormatKHR surface_format,
    VkExtent2D image_size, VkImageUsageFlags image_usage,
    VkSurfaceTransformFlagBitsKHR surface_transform,
    VkPresentModeKHR present_mode, VkSwapchainKHR &old_swapchain,
    VkSwapchainKHR &swapchain) {
  VkSwapchainCreateInfoKHR swapchain_create_info = {
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, // VkStructureType sType
      nullptr,               // const void                     * pNext
      0,                     // VkSwapchainCreateFlagsKHR        flags
      presentation_surface,  // VkSurfaceKHR                     surface
      image_count,           // uint32_t                         minImageCount
      surface_format.format, // VkFormat                         imageFormat
      surface_format.colorSpace, // VkColorSpaceKHR imageColorSpace
      image_size,                // VkExtent2D                       imageExtent
      1,           // uint32_t                         imageArrayLayers
      image_usage, // VkImageUsageFlags                imageUsage
      VK_SHARING_MODE_EXCLUSIVE, // VkSharingMode imageSharingMode
      0,       // uint32_t                         queueFamilyIndexCount
      nullptr, // const uint32_t                 * pQueueFamilyIndices
      surface_transform, // VkSurfaceTransformFlagBitsKHR    preTransform
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, // VkCompositeAlphaFlagBitsKHR
                                         // compositeAlpha
      present_mode, // VkPresentModeKHR                 presentMode
      VK_TRUE,      // VkBool32                         clipped
      old_swapchain // VkSwapchainKHR                   oldSwapchain
  };

  VkResult result = vkCreateSwapchainKHR(logical_device, &swapchain_create_info,
                                         nullptr, &swapchain);
  if ((VK_SUCCESS != result) || (VK_NULL_HANDLE == swapchain)) {
    std::cout << "Could not create a swapchain." << std::endl;
    return false;
  }

  if (VK_NULL_HANDLE != old_swapchain) {
    vkDestroySwapchainKHR(logical_device, old_swapchain, nullptr);
    old_swapchain = VK_NULL_HANDLE;
  }

  return true;
}

bool VulkanLibraryInterface::getHandlesOfSwapchainImages(
    VkDevice logical_device, VkSwapchainKHR swapchain,
    std::vector<VkImage> &swapchain_images) {
  uint32_t images_count = 0;
  VkResult result = VK_SUCCESS;

  result = vkGetSwapchainImagesKHR(logical_device, swapchain, &images_count,
                                   nullptr);
  if ((VK_SUCCESS != result) || (0 == images_count)) {
    std::cout << "Could not get the number of swapchain images." << std::endl;
    return false;
  }

  swapchain_images.resize(images_count);
  result = vkGetSwapchainImagesKHR(logical_device, swapchain, &images_count,
                                   swapchain_images.data());
  if ((VK_SUCCESS != result) || (0 == images_count)) {
    std::cout << "Could not enumerate swapchain images." << std::endl;
    return false;
  }

  return true;
}

bool VulkanLibraryInterface::
    createSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(
        VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
        VkDevice logical_device, VkImageUsageFlags swapchain_image_usage,
        VkExtent2D &image_size, VkFormat &image_format,
        VkSwapchainKHR &old_swapchain, VkSwapchainKHR &swapchain,
        std::vector<VkImage> &swapchain_images) {
  VkPresentModeKHR desired_present_mode;
  if (!selectDesiredPresentationMode(physical_device, presentation_surface,
                                     VK_PRESENT_MODE_MAILBOX_KHR,
                                     desired_present_mode)) {
    return false;
  }

  VkSurfaceCapabilitiesKHR surface_capabilities;
  if (!getCapabilitiesOfPresentationSurface(
          physical_device, presentation_surface, surface_capabilities)) {
    return false;
  }

  uint32_t number_of_images;
  if (!selectNumberOfSwapchainImages(surface_capabilities, number_of_images)) {
    return false;
  }

  if (!chooseSizeOfSwapchainImages(surface_capabilities, image_size)) {
    return false;
  }
  if ((0 == image_size.width) || (0 == image_size.height)) {
    return true;
  }

  VkImageUsageFlags image_usage;
  if (!selectDesiredUsageScenariosOfSwapchainImages(
          surface_capabilities, swapchain_image_usage, image_usage)) {
    return false;
  }

  VkSurfaceTransformFlagBitsKHR surface_transform;
  selectTransformationOfSwapchainImages(surface_capabilities,
                                        VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                                        surface_transform);

  VkColorSpaceKHR image_color_space;
  if (!selectFormatOfSwapchainImages(
          physical_device, presentation_surface,
          {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
          image_format, image_color_space)) {
    return false;
  }

  if (!createSwapchain(logical_device, presentation_surface, number_of_images,
                       {image_format, image_color_space}, image_size,
                       image_usage, surface_transform, desired_present_mode,
                       old_swapchain, swapchain)) {
    return false;
  }

  if (!getHandlesOfSwapchainImages(logical_device, swapchain,
                                   swapchain_images)) {
    return false;
  }
  return true;
}

bool VulkanLibraryInterface::acquireSwapchainImage(VkDevice logical_device,
                                                   VkSwapchainKHR swapchain,
                                                   VkSemaphore semaphore,
                                                   VkFence fence,
                                                   uint32_t &image_index) {
  VkResult result;

  result = vkAcquireNextImageKHR(logical_device, swapchain, 2000000000,
                                 semaphore, fence, &image_index);
  switch (result) {
  case VK_SUCCESS:
  case VK_SUBOPTIMAL_KHR:
    return true;
  default:
    return false;
  }
}

bool VulkanLibraryInterface::presentImage(
    VkQueue queue, std::vector<VkSemaphore> rendering_semaphores,
    std::vector<PresentInfo> images_to_present) {
  VkResult result;
  std::vector<VkSwapchainKHR> swapchains;
  std::vector<uint32_t> image_indices;

  for (auto &image_to_present : images_to_present) {
    swapchains.emplace_back(image_to_present.Swapchain);
    image_indices.emplace_back(image_to_present.ImageIndex);
  }

  VkPresentInfoKHR present_info = {
      VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, // VkStructureType          sType
      nullptr,                            // const void*              pNext
      static_cast<uint32_t>(
          rendering_semaphores.size()), // uint32_t waitSemaphoreCount
      rendering_semaphores.data(), // const VkSemaphore      * pWaitSemaphores
      static_cast<uint32_t>(swapchains.size()), // uint32_t swapchainCount
      swapchains.data(),    // const VkSwapchainKHR   * pSwapchains
      image_indices.data(), // const uint32_t         * pImageIndices
      nullptr               // VkResult*                pResults
  };

  result = vkQueuePresentKHR(queue, &present_info);
  switch (result) {
  case VK_SUCCESS:
    return true;
  default:
    return false;
  }
}

void VulkanLibraryInterface::destroySwapchain(VkDevice logical_device,
                                              VkSwapchainKHR &swapchain) {
  if (swapchain) {
    vkDestroySwapchainKHR(logical_device, swapchain, nullptr);
    swapchain = VK_NULL_HANDLE;
  }
}

void VulkanLibraryInterface::destroyPresentationSurface(
    VkInstance instance, VkSurfaceKHR &presentation_surface) {
  if (presentation_surface) {
    vkDestroySurfaceKHR(instance, presentation_surface, nullptr);
    presentation_surface = VK_NULL_HANDLE;
  }
}

bool VulkanLibraryInterface::createCommandPool(
    VkDevice logical_device, VkCommandPoolCreateFlags parameters,
    uint32_t queue_family, VkCommandPool &command_pool) {
  VkCommandPoolCreateInfo command_pool_create_info = {
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, // VkStructureType sType
      nullptr,     // const void                 * pNext
      parameters,  // VkCommandPoolCreateFlags     flags
      queue_family // uint32_t                     queueFamilyIndex
  };

  VkResult result = vkCreateCommandPool(
      logical_device, &command_pool_create_info, nullptr, &command_pool);
  if (VK_SUCCESS != result) {
    std::cout << "Could not create command pool." << std::endl;
    return false;
  }
  return true;
}

bool VulkanLibraryInterface::allocateCommandBuffers(
    VkDevice logical_device, VkCommandPool command_pool,
    VkCommandBufferLevel level, uint32_t count,
    std::vector<VkCommandBuffer> &command_buffers) {
  VkCommandBufferAllocateInfo command_buffer_allocate_info = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, // VkStructureType sType
      nullptr,      // const void             * pNext
      command_pool, // VkCommandPool            commandPool
      level,        // VkCommandBufferLevel     level
      count         // uint32_t                 commandBufferCount
  };

  command_buffers.resize(count);

  VkResult result = vkAllocateCommandBuffers(
      logical_device, &command_buffer_allocate_info, command_buffers.data());
  if (VK_SUCCESS != result) {
    std::cout << "Could not allocate command buffers." << std::endl;
    return false;
  }
  return true;
}

bool VulkanLibraryInterface::beginCommandBufferRecordingOperation(
    VkCommandBuffer command_buffer, VkCommandBufferUsageFlags usage,
    VkCommandBufferInheritanceInfo *secondary_command_buffer_info) {
  VkCommandBufferBeginInfo command_buffer_begin_info = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, // VkStructureType sType
      nullptr, // const void                           * pNext
      usage,   // VkCommandBufferUsageFlags              flags
      secondary_command_buffer_info // const VkCommandBufferInheritanceInfo *
                                    // pInheritanceInfo
  };

  VkResult result =
      vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
  if (VK_SUCCESS != result) {
    std::cout << "Could not begin command buffer recording operation."
              << std::endl;
    return false;
  }
  return true;
}

bool VulkanLibraryInterface::endCommandBufferRecordingOperation(
    VkCommandBuffer command_buffer) {
  VkResult result = vkEndCommandBuffer(command_buffer);
  if (VK_SUCCESS != result) {
    std::cout << "Error occurred during command buffer recording." << std::endl;
    return false;
  }
  return true;
}

bool VulkanLibraryInterface::resetCommandBuffer(VkCommandBuffer command_buffer,
                                                bool release_resources) {
  VkResult result = vkResetCommandBuffer(
      command_buffer,
      release_resources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0);
  if (VK_SUCCESS != result) {
    std::cout << "Error occurred during command buffer reset." << std::endl;
    return false;
  }
  return true;
}

bool VulkanLibraryInterface::resetCommandPool(VkDevice logical_device,
                                              VkCommandPool command_pool,
                                              bool release_resources) {
  VkResult result = vkResetCommandPool(
      logical_device, command_pool,
      release_resources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0);
  if (VK_SUCCESS != result) {
    std::cout << "Error occurred during command pool reset." << std::endl;
    return false;
  }
  return true;
}

bool VulkanLibraryInterface::createSemaphore(VkDevice logical_device,
                                             VkSemaphore &semaphore) {
  VkSemaphoreCreateInfo semaphore_create_info = {
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, // VkStructureType sType
      nullptr, // const void               * pNext
      0        // VkSemaphoreCreateFlags     flags
  };

  VkResult result = vkCreateSemaphore(logical_device, &semaphore_create_info,
                                      nullptr, &semaphore);
  if (VK_SUCCESS != result) {
    std::cout << "Could not create a semaphore." << std::endl;
    return false;
  }
  return true;
}

bool VulkanLibraryInterface::createFence(VkDevice logical_device, bool signaled,
                                         VkFence &fence) {
  VkFenceCreateInfo fence_create_info = {
      VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, // VkStructureType        sType
      nullptr,                             // const void           * pNext
      signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u, // VkFenceCreateFlags flags
  };

  VkResult result =
      vkCreateFence(logical_device, &fence_create_info, nullptr, &fence);
  if (VK_SUCCESS != result) {
    std::cout << "Could not create a fence." << std::endl;
    return false;
  }
  return true;
}

bool VulkanLibraryInterface::waitForFences(VkDevice logical_device,
                                           std::vector<VkFence> const &fences,
                                           VkBool32 wait_for_all,
                                           uint64_t timeout) {
  if (fences.size() > 0) {
    VkResult result =
        vkWaitForFences(logical_device, static_cast<uint32_t>(fences.size()),
                        fences.data(), wait_for_all, timeout);
    if (VK_SUCCESS != result) {
      std::cout << "Waiting on fence failed." << std::endl;
      return false;
    }
    return true;
  }
  return false;
}

bool VulkanLibraryInterface::resetFences(VkDevice logical_device,
                                         std::vector<VkFence> const &fences) {
  if (fences.size() > 0) {
    VkResult result = vkResetFences(
        logical_device, static_cast<uint32_t>(fences.size()), fences.data());
    if (VK_SUCCESS != result) {
      std::cout << "Error occurred when tried to reset fences." << std::endl;
      return false;
    }
    return VK_SUCCESS == result;
  }
  return false;
}

bool VulkanLibraryInterface::submitCommandBuffersToQueue(
    VkQueue queue, std::vector<WaitSemaphoreInfo> wait_semaphore_infos,
    std::vector<VkCommandBuffer> command_buffers,
    std::vector<VkSemaphore> signal_semaphores, VkFence fence) {
  std::vector<VkSemaphore> wait_semaphore_handles;
  std::vector<VkPipelineStageFlags> wait_semaphore_stages;

  for (auto &wait_semaphore_info : wait_semaphore_infos) {
    wait_semaphore_handles.emplace_back(wait_semaphore_info.Semaphore);
    wait_semaphore_stages.emplace_back(wait_semaphore_info.WaitingStage);
  }

  VkSubmitInfo submit_info = {
      VK_STRUCTURE_TYPE_SUBMIT_INFO, // VkStructureType                sType
      nullptr,                       // const void                   * pNext
      static_cast<uint32_t>(
          wait_semaphore_infos.size()), // uint32_t waitSemaphoreCount
      wait_semaphore_handles
          .data(), // const VkSemaphore            * pWaitSemaphores
      wait_semaphore_stages
          .data(), // const VkPipelineStageFlags   * pWaitDstStageMask
      static_cast<uint32_t>(
          command_buffers.size()), // uint32_t commandBufferCount
      command_buffers.data(), // const VkCommandBuffer        * pCommandBuffers
      static_cast<uint32_t>(
          signal_semaphores.size()), // uint32_t signalSemaphoreCount
      signal_semaphores
          .data() // const VkSemaphore            * pSignalSemaphores
  };

  VkResult result = vkQueueSubmit(queue, 1, &submit_info, fence);
  if (VK_SUCCESS != result) {
    std::cout << "Error occurred during command buffer submission."
              << std::endl;
    return false;
  }
  return true;
}

bool VulkanLibraryInterface::synchronizeTwoCommandBuffers(
    VkQueue first_queue,
    std::vector<WaitSemaphoreInfo> first_wait_semaphore_infos,
    std::vector<VkCommandBuffer> first_command_buffers,
    std::vector<WaitSemaphoreInfo> synchronizing_semaphores,
    VkQueue second_queue, std::vector<VkCommandBuffer> second_command_buffers,
    std::vector<VkSemaphore> second_signal_semaphores, VkFence second_fence) {
  std::vector<VkSemaphore> first_signal_semaphores;
  for (auto &semaphore_info : synchronizing_semaphores) {
    first_signal_semaphores.emplace_back(semaphore_info.Semaphore);
  }
  if (!submitCommandBuffersToQueue(first_queue, first_wait_semaphore_infos,
                                   first_command_buffers,
                                   first_signal_semaphores, VK_NULL_HANDLE)) {
    return false;
  }

  if (!submitCommandBuffersToQueue(second_queue, synchronizing_semaphores,
                                   second_command_buffers,
                                   second_signal_semaphores, second_fence)) {
    return false;
  }
  return true;
}

bool VulkanLibraryInterface::
    checkIfProcessingOfSubmittedCommandBufferHasFinished(
        VkDevice logical_device, VkQueue queue,
        std::vector<WaitSemaphoreInfo> wait_semaphore_infos,
        std::vector<VkCommandBuffer> command_buffers,
        std::vector<VkSemaphore> signal_semaphores, VkFence fence,
        uint64_t timeout, VkResult &wait_status) {
  if (!submitCommandBuffersToQueue(queue, wait_semaphore_infos, command_buffers,
                                   signal_semaphores, fence)) {
    return false;
  }

  return waitForFences(logical_device, {fence}, VK_FALSE, timeout);
}

bool VulkanLibraryInterface::waitUntilAllCommandsSubmittedToQueueAreFinished(
    VkQueue queue) {
  VkResult result = vkQueueWaitIdle(queue);
  if (VK_SUCCESS != result) {
    std::cout << "Waiting for all operations submitted to queue failed."
              << std::endl;
    return false;
  }
  return true;
}

bool VulkanLibraryInterface::waitForAllSubmittedCommandsToBeFinished(
    VkDevice logical_device) {
  VkResult result = vkDeviceWaitIdle(logical_device);
  if (VK_SUCCESS != result) {
    std::cout << "Waiting on a device failed." << std::endl;
    return false;
  }
  return true;
}

void VulkanLibraryInterface::destroyFence(VkDevice logical_device,
                                          VkFence &fence) {
  if (VK_NULL_HANDLE != fence) {
    vkDestroyFence(logical_device, fence, nullptr);
    fence = VK_NULL_HANDLE;
  }
}

void VulkanLibraryInterface::destroySemaphore(VkDevice logical_device,
                                              VkSemaphore &semaphore) {
  if (VK_NULL_HANDLE != semaphore) {
    vkDestroySemaphore(logical_device, semaphore, nullptr);
    semaphore = VK_NULL_HANDLE;
  }
}

void VulkanLibraryInterface::freeCommandBuffers(
    VkDevice logical_device, VkCommandPool command_pool,
    std::vector<VkCommandBuffer> &command_buffers) {
  if (command_buffers.size() > 0) {
    vkFreeCommandBuffers(logical_device, command_pool,
                         static_cast<uint32_t>(command_buffers.size()),
                         command_buffers.data());
    command_buffers.clear();
  }
}

void VulkanLibraryInterface::destroyCommandPool(VkDevice logical_device,
                                                VkCommandPool &command_pool) {
  if (VK_NULL_HANDLE != command_pool) {
    vkDestroyCommandPool(logical_device, command_pool, nullptr);
    command_pool = VK_NULL_HANDLE;
  }
}

} // namespace circe