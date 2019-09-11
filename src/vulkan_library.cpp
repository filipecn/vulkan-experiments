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
/// The contets were heavely based on the Vulkan Cookbook 2017.
///
/// \brief Implementation of circe's vulkan library interface.

#include "vulkan_library.h"
#include "logging.h"
#include "vulkan_debug.h"
#include <iostream>

#if defined __APPLE__
#include <dlfcn.h>
#elif defined __linux
#include <dlfcn.h>
#endif

namespace circe {

namespace vk {

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
#ifndef GLFW_INCLUDE_VULKAN
#define EXPORTED_VULKAN_FUNCTION(name)                                         \
  name = (PFN_##name)LoadFunction(vulkanLibrary, #name);                       \
  CHECK(name != nullptr, concat("Could not load exported Vulkan function \
named:",                                                                       \
                                #name))
#include "vulkan_api.inl"
#endif
  return true;
}

void VulkanLibraryInterface::releaseVulkanLoaderLibrary(
    VulkanLibraryType &vulkan_library) {
  if (nullptr != vulkan_library) {
#if defined _WIN32
    FreeLibrary(vulkan_library);
#elif defined __linux
    dlclose(vulkan_library);
#elif defined __APPLE__
    dlclose(vulkan_library);
#endif
    vulkan_library = nullptr;
  }
}

bool VulkanLibraryInterface::loadGlobalLevelFunctions() {
#ifndef GLFW_INCLUDE_VULKAN
#define GLOBAL_LEVEL_VULKAN_FUNCTION(name)                                     \
  name = (PFN_##name)vkGetInstanceProcAddr(nullptr, #name);                    \
  CHECK(name != nullptr,                                                       \
        concat("Could not load global-level function named: ", #name))
#include "vulkan_api.inl"
#endif
  return true;
}

bool VulkanLibraryInterface::loadInstanceLevelFunctions(
    VkInstance instance, const std::vector<const char *> &extensions) {
#ifndef GLFW_INCLUDE_VULKAN
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
#endif
  return true;
}

bool VulkanLibraryInterface::loadDeviceLevelFunctions(
    VkDevice logical_device,
    std::vector<char const *> const &enabled_extensions) {
#ifndef GLFW_INCLUDE_VULKAN
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
#endif
  return true;
}

// VULKAN API EXTENSIONS //////////////////////////////////////////////////////

bool VulkanLibraryInterface::checkAvaliableInstanceExtensions(
    std::vector<VkExtensionProperties> &extensions) {
  uint32_t extensionsCount = 0;
  CHECK_VULKAN(vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount,
                                                      nullptr));
  ASSERT(extensionsCount != 0);
  extensions.resize(extensionsCount);
  CHECK_VULKAN(vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount,
                                                      &extensions[0]));
  ASSERT(extensionsCount != 0);
  return true;
}

bool VulkanLibraryInterface::checkAvailableDeviceExtensions(
    VkPhysicalDevice physicalDevice,
    std::vector<VkExtensionProperties> &extensions) {
  uint32_t extensionsCount = 0;
  CHECK_VULKAN(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr,
                                                    &extensionsCount, nullptr));
  if (extensionsCount == 0)
    return true;
  extensions.resize(extensionsCount);
  CHECK_VULKAN(vkEnumerateDeviceExtensionProperties(
      physicalDevice, nullptr, &extensionsCount, extensions.data()));
  return true;
}

bool VulkanLibraryInterface::isExtensionSupported(
    const std::vector<VkExtensionProperties> &extensions,
    const char *extensionName) {
  for (auto &extension : extensions)
    if (std::string(extension.extensionName) == std::string(extensionName))
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
    D_RETURN_FALSE_IF_NOT(
        isExtensionSupported(instanceExtensions, extension),
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
  CHECK_VULKAN(vkCreateInstance(&createInfo, nullptr, &instance));
  D_RETURN_FALSE_IF_NOT(instance != VK_NULL_HANDLE,
                        "Could not create Vulkan Instance.");
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
  CHECK_VULKAN(vkEnumeratePhysicalDevices(instance, &devicesCount, nullptr));
  D_RETURN_FALSE_IF_NOT(
      devicesCount != 0,
      "Could not get the number of available physical devices.");
  devices.resize(devicesCount);
  CHECK_VULKAN(
      vkEnumeratePhysicalDevices(instance, &devicesCount, devices.data()));
  D_RETURN_FALSE_IF_NOT(devicesCount != 0,
                        "Could not enumerate physical devices.");
  return true;
}

void VulkanLibraryInterface::getFeaturesAndPropertiesOfPhysicalDevice(
    VkPhysicalDevice physical_device, VkPhysicalDeviceFeatures &device_features,
    VkPhysicalDeviceProperties &device_properties) {
  vkGetPhysicalDeviceFeatures(physical_device, &device_features);
  vkGetPhysicalDeviceProperties(physical_device, &device_properties);
}

void VulkanLibraryInterface::getPhysicalDeviceMemoryProperties(
    VkPhysicalDevice physical_device,
    VkPhysicalDeviceMemoryProperties &properties) {
  vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);
}

// VULKAN QUEUE FAMILIES //////////////////////////////////////////////////////

bool VulkanLibraryInterface::checkAvailableQueueFamiliesAndTheirProperties(
    VkPhysicalDevice physical_device,
    std::vector<VkQueueFamilyProperties> &queue_families) {
  uint32_t queue_families_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                           &queue_families_count, nullptr);
  D_RETURN_FALSE_IF_NOT(queue_families_count != 0,
                        "Could not get the number of queue families.\n");
  queue_families.resize(queue_families_count);
  vkGetPhysicalDeviceQueueFamilyProperties(
      physical_device, &queue_families_count, queue_families.data());
  D_RETURN_FALSE_IF_NOT(queue_families_count != 0,
                        "Could not acquire properties of queue families.\n");
  return true;
}

bool VulkanLibraryInterface::selectIndexOfQueueFamilyWithDesiredCapabilities(
    VkPhysicalDevice physical_device, VkQueueFlags desired_capabilities,
    uint32_t &queue_family_index) {
  std::vector<VkQueueFamilyProperties> queue_families;
  D_RETURN_FALSE_IF_NOT(checkAvailableQueueFamiliesAndTheirProperties(
                            physical_device, queue_families),
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

// VULKAN LOGICAL DEVICE /////////////////////////////////////////////////////

bool VulkanLibraryInterface::createLogicalDevice(
    VkPhysicalDevice physical_device, std::vector<QueueFamilyInfo> queue_infos,
    std::vector<char const *> const &desired_extensions,
    VkPhysicalDeviceFeatures *desired_features, VkDevice &logical_device) {
  std::vector<VkExtensionProperties> available_extensions;
  D_RETURN_FALSE_IF_NOT(
      checkAvailableDeviceExtensions(physical_device, available_extensions),
      "");

  for (auto &extension : desired_extensions)
    D_RETURN_FALSE_IF_NOT(isExtensionSupported(available_extensions, extension),
                          concat("Extension named '", extension,
                                 "' is not supported by a physical device.\n"));

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

  for (auto &info : queue_infos) {
    queue_create_infos.push_back({
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // VkStructureType sType
        nullptr,                   // const void                     * pNext
        0,                         // VkDeviceQueueCreateFlags         flags
        info.family_index.value(), // uint32_t queueFamilyIndex
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
  CHECK_VULKAN(vkCreateDevice(physical_device, &device_create_info, nullptr,
                              &logical_device));
  D_RETURN_FALSE_IF_NOT(logical_device != VK_NULL_HANDLE,
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

// VULKAN RESOURCES AND MEMORY ///////////////////////////////////////////////

void freeMemoryObject(VkDevice logical_device, VkDeviceMemory &memory_object) {
  if (VK_NULL_HANDLE != memory_object) {
    vkFreeMemory(logical_device, memory_object, nullptr);
    memory_object = VK_NULL_HANDLE;
  }
}

// VULKAN BUFFER /////////////////////////////////////////////////////////////

void VulkanLibraryInterface::destroyBuffer(VkDevice logical_device,
                                           VkBuffer &buffer) {
  if (VK_NULL_HANDLE != buffer) {
    vkDestroyBuffer(logical_device, buffer, nullptr);
    buffer = VK_NULL_HANDLE;
  }
}

bool VulkanLibraryInterface::createBufferView(VkDevice logical_device,
                                              VkBuffer buffer, VkFormat format,
                                              VkDeviceSize memory_offset,
                                              VkDeviceSize memory_range,
                                              VkBufferView &buffer_view) {
  VkBufferViewCreateInfo buffer_view_create_info = {
      VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO, // VkStructureType sType
      nullptr,       // const void               * pNext
      0,             // VkBufferViewCreateFlags    flags
      buffer,        // VkBuffer                   buffer
      format,        // VkFormat                   format
      memory_offset, // VkDeviceSize               offset
      memory_range   // VkDeviceSize               range
  };

  VkResult result = vkCreateBufferView(logical_device, &buffer_view_create_info,
                                       nullptr, &buffer_view);
  if (VK_SUCCESS != result) {
    std::cout << "Could not creat buffer view." << std::endl;
    return false;
  }
  return true;
}

void VulkanLibraryInterface::destroyBufferView(VkDevice logical_device,
                                               VkBufferView &buffer_view) {
  if (VK_NULL_HANDLE != buffer_view) {
    vkDestroyBufferView(logical_device, buffer_view, nullptr);
    buffer_view = VK_NULL_HANDLE;
  }
}

bool VulkanLibraryInterface::allocateAndBindMemoryObjectToBuffer(
    VkPhysicalDevice physical_device, VkDevice logical_device, VkBuffer buffer,
    VkMemoryPropertyFlagBits memory_properties, VkDeviceMemory &memory_object) {
  // create a pointer to the physical device's memory properties: number of
  // heaps, sizes, etc
  VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device,
                                      &physical_device_memory_properties);
  // acquire parameters of a memory that needs to be used for the buffer
  // (buffer's memory may need to be bigger than the buffer's size)
  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(logical_device, buffer, &memory_requirements);
  memory_object = VK_NULL_HANDLE;
  // iterate over the available physical device's memory types
  for (uint32_t type = 0;
       type < physical_device_memory_properties.memoryTypeCount; ++type) {
    // check if type and properties match the desired ones
    if ((memory_requirements.memoryTypeBits & (1 << type)) &&
        ((physical_device_memory_properties.memoryTypes[type].propertyFlags &
          memory_properties) == memory_properties)) {
      // try to allocate memory
      VkMemoryAllocateInfo buffer_memory_allocate_info = {
          VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // VkStructureType    sType
          nullptr,                                // const void       * pNext
          memory_requirements.size, // VkDeviceSize       allocationSize
          type                      // uint32_t           memoryTypeIndex
      };
      VkResult result =
          vkAllocateMemory(logical_device, &buffer_memory_allocate_info,
                           nullptr, &memory_object);
      if (VK_SUCCESS == result) {
        break;
      }
    }
  }
  CHECK_INFO(VK_NULL_HANDLE == memory_object,
             "Could not allocate memory for a buffer.");
  CHECK_VULKAN(vkBindBufferMemory(logical_device, buffer, memory_object, 0));
  return true;
}

void VulkanLibraryInterface::setBufferMemoryBarrier(
    VkCommandBuffer command_buffer, VkPipelineStageFlags generating_stages,
    VkPipelineStageFlags consuming_stages,
    std::vector<BufferTransition> buffer_transitions) {

  std::vector<VkBufferMemoryBarrier> buffer_memory_barriers;

  for (auto &buffer_transition : buffer_transitions) {
    buffer_memory_barriers.push_back({
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, // VkStructureType    sType
        nullptr,                                 // const void       * pNext
        buffer_transition.CurrentAccess, // VkAccessFlags      srcAccessMask
        buffer_transition.NewAccess,     // VkAccessFlags      dstAccessMask
        buffer_transition.CurrentQueueFamily, // uint32_t srcQueueFamilyIndex
        buffer_transition.NewQueueFamily,     // uint32_t dstQueueFamilyIndex
        buffer_transition.Buffer,             // VkBuffer           buffer
        0,                                    // VkDeviceSize       offset
        VK_WHOLE_SIZE                         // VkDeviceSize       size
    });
  }

  if (buffer_memory_barriers.size() > 0) {
    vkCmdPipelineBarrier(command_buffer, generating_stages, consuming_stages, 0,
                         0, nullptr,
                         static_cast<uint32_t>(buffer_memory_barriers.size()),
                         buffer_memory_barriers.data(), 0, nullptr);
  }
}

// VULKAN IMAGE //////////////////////////////////////////////////////////////

void VulkanLibraryIntterface::destroyImage(VkDevice logical_device,
                                           VkImage &image) {
  if (VK_NULL_HANDLE != image) {
    vkDestroyImage(logical_device, image, nullptr);
    image = VK_NULL_HANDLE;
  }
}

bool VulkanLibraryInterface::createImageView(
    VkDevice logical_device, VkImage image, VkImageViewType view_type,
    VkFormat format, VkImageAspectFlags aspect, VkImageView &image_view) {
  VkImageViewCreateInfo image_view_create_info = {
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // VkStructureType sType
      nullptr,   // const void               * pNext
      0,         // VkImageViewCreateFlags     flags
      image,     // VkImage                    image
      view_type, // VkImageViewType            viewType
      format,    // VkFormat                   format
      {
          // VkComponentMapping         components
          VK_COMPONENT_SWIZZLE_IDENTITY, // VkComponentSwizzle         r
          VK_COMPONENT_SWIZZLE_IDENTITY, // VkComponentSwizzle         g
          VK_COMPONENT_SWIZZLE_IDENTITY, // VkComponentSwizzle         b
          VK_COMPONENT_SWIZZLE_IDENTITY  // VkComponentSwizzle         a
      },
      {
          // VkImageSubresourceRange    subresourceRange
          aspect,                   // VkImageAspectFlags         aspectMask
          0,                        // uint32_t                   baseMipLevel
          VK_REMAINING_MIP_LEVELS,  // uint32_t                   levelCount
          0,                        // uint32_t                   baseArrayLayer
          VK_REMAINING_ARRAY_LAYERS // uint32_t                   layerCount
      }};

  CHECK_VULKAN(vkCreateImageView(logical_device, &image_view_create_info,
                                 nullptr, &image_view));
  return true;
}

void VulkanLibraryInterface::destroyImageView(VkDevice logical_device,
                                              VkImageView &image_view) {
  if (VK_NULL_HANDLE != image_view) {
    vkDestroyImageView(logical_device, image_view, nullptr);
    image_view = VK_NULL_HANDLE;
  }
}

bool VulkanLibraryInterface::allocateAndBindMemoryObjectToImage(
    VkPhysicalDevice physical_device, VkDevice logical_device, VkImage image,
    VkMemoryPropertyFlagBits memory_properties, VkDeviceMemory &memory_object) {
  // create a pointer to the physical device's memory properties: number of
  // heaps, sizes, etc
  VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device,
                                      &physical_device_memory_properties);
  // acquire parameters of a memory that needs to be used for the image
  // (buffer's memory may need to be bigger than the buffer's size)
  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(logical_device, image, &memory_requirements);
  memory_object = VK_NULL_HANDLE;
  // iterate over the available physical device's memory types
  for (uint32_t type = 0;
       type < physical_device_memory_properties.memoryTypeCount; ++type) {
    // check if type and properties match the desired ones
    if ((memory_requirements.memoryTypeBits & (1 << type)) &&
        ((physical_device_memory_properties.memoryTypes[type].propertyFlags &
          memory_properties) == memory_properties)) {
      // try to allocate memory
      VkMemoryAllocateInfo image_memory_allocate_info = {
          VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // VkStructureType    sType
          nullptr,                                // const void       * pNext
          memory_requirements.size, // VkDeviceSize       allocationSize
          type                      // uint32_t           memoryTypeIndex
      };
      VkResult result = vkAllocateMemory(
          logical_device, &image_memory_allocate_info, nullptr, &memory_object);
      if (VK_SUCCESS == result) {
        break;
      }
    }
  }
  CHECK_INFO(VK_NULL_HANDLE == memory_object,
             "Could not allocate memory for an image.");
  CHECK_VULKAN(vkBindImageMemory(logical_device, image, memory_object, 0));
  return true;
}

void VulkanLibraryInterface::SetImageMemoryBarrier(
    VkCommandBuffer command_buffer, VkPipelineStageFlags generating_stages,
    VkPipelineStageFlags consuming_stages,
    std::vector<ImageTransition> image_transitions) {
  std::vector<VkImageMemoryBarrier> image_memory_barriers;

  for (auto &image_transition : image_transitions) {
    image_memory_barriers.push_back(
        {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, // VkStructureType sType
         nullptr,                        // const void               * pNext
         image_transition.CurrentAccess, // VkAccessFlags srcAccessMask
         image_transition.NewAccess, // VkAccessFlags              dstAccessMask
         image_transition.CurrentLayout, // VkImageLayout              oldLayout
         image_transition.NewLayout,     // VkImageLayout              newLayout
         image_transition.CurrentQueueFamily, // uint32_t srcQueueFamilyIndex
         image_transition.NewQueueFamily,     // uint32_t dstQueueFamilyIndex
         image_transition.Image, // VkImage                    image
         {
             // VkImageSubresourceRange    subresourceRange
             image_transition.Aspect, // VkImageAspectFlags         aspectMask
             0,                       // uint32_t                   baseMipLevel
             VK_REMAINING_MIP_LEVELS, // uint32_t                   levelCount
             0, // uint32_t                   baseArrayLayer
             VK_REMAINING_ARRAY_LAYERS // uint32_t                   layerCount
         }});
  }

  if (image_memory_barriers.size() > 0) {
    vkCmdPipelineBarrier(command_buffer, generating_stages, consuming_stages, 0,
                         0, nullptr, 0, nullptr,
                         static_cast<uint32_t>(image_memory_barriers.size()),
                         image_memory_barriers.data());
  }
}

// VULKAN SURFACE ////////////////////////////////////////////////////////////

bool VulkanLibraryInterface::createPresentationSurface(
    VkInstance instance, WindowParameters window_parameters,
    VkSurfaceKHR &presentation_surface) {
  VkResult result;

#ifdef VK_USE_PLATFORM_WIN32_KHR

  VkWin32SurfaceCreateInfoKHR surface_create_info = {
      VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, // VkStructureType
                                                       // sType
      nullptr,                     // const void                    * pNext
      0,                           // VkWin32SurfaceCreateFlagsKHR    flags
      window_parameters.HInstance, // HINSTANCE hinstance
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

void VulkanLibraryInterface::destroyPresentationSurface(
    VkInstance instance, VkSurfaceKHR &presentation_surface) {
  if (instance && presentation_surface) {
    vkDestroySurfaceKHR(instance, presentation_surface, nullptr);
    presentation_surface = VK_NULL_HANDLE;
  }
}

bool VulkanLibraryInterface::getCapabilitiesOfPresentationSurface(
    VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
    VkSurfaceCapabilitiesKHR &surface_capabilities) {
  CHECK_VULKAN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physical_device, presentation_surface, &surface_capabilities));
  return true;
}

// VULKAN SWAPCHAIN //////////////////////////////////////////////////////////

bool VulkanLibraryInterface::selectDesiredPresentationMode(
    VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
    VkPresentModeKHR desired_present_mode, VkPresentModeKHR &present_mode) {
  // Enumerate supported present modes
  uint32_t present_modes_count = 0;
  CHECK_VULKAN(vkGetPhysicalDeviceSurfacePresentModesKHR(
      physical_device, presentation_surface, &present_modes_count, nullptr));
  D_RETURN_FALSE_IF_NOT(0 != present_modes_count,
                        "Could not get the number of supported present modes.");
  std::vector<VkPresentModeKHR> present_modes(present_modes_count);
  CHECK_VULKAN(vkGetPhysicalDeviceSurfacePresentModesKHR(
      physical_device, presentation_surface, &present_modes_count,
      present_modes.data()));
  D_RETURN_FALSE_IF_NOT(0 != present_modes_count,
                        "Could not enumerate present modes.");
  // Select present mode
  for (auto &current_present_mode : present_modes) {
    if (current_present_mode == desired_present_mode) {
      present_mode = desired_present_mode;
      return true;
    }
  }
  INFO("Desired present mode is not supported. Selecting default FIFO mode.");
  for (auto &current_present_mode : present_modes) {
    if (current_present_mode == VK_PRESENT_MODE_FIFO_KHR) {
      present_mode = VK_PRESENT_MODE_FIFO_KHR;
      return true;
    }
  }
  INFO("VK_PRESENT_MODE_FIFO_KHR is not supported though it's mandatory "
       "for all drivers!");
  return false;
}

bool VulkanLibraryInterface::querySwapChainSupport(
    VkPhysicalDevice physical_device, VkSurfaceKHR surface,
    SwapChainSupportDetails &details) {
  CHECK_VULKAN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physical_device, surface, &details.capabilities));
  uint32_t format_count = 0;
  CHECK_VULKAN(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                                    &format_count, nullptr));
  if (format_count != 0) {
    details.formats.resize(format_count);
    CHECK_VULKAN(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device, surface, &format_count, details.formats.data()));
  }
  uint32_t present_mode_count = 0;
  CHECK_VULKAN(vkGetPhysicalDeviceSurfacePresentModesKHR(
      physical_device, surface, &present_mode_count, nullptr));
  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    CHECK_VULKAN(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device, surface, &present_mode_count,
        details.present_modes.data()));
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

  CHECK_VULKAN(vkGetPhysicalDeviceSurfaceFormatsKHR(
      physical_device, presentation_surface, &formats_count, nullptr));
  D_RETURN_FALSE_IF_NOT(
      0 != formats_count,
      "Could not get the number of supported surface formats.");

  std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
  CHECK_VULKAN(vkGetPhysicalDeviceSurfaceFormatsKHR(
      physical_device, presentation_surface, &formats_count,
      surface_formats.data()));
  D_RETURN_FALSE_IF_NOT(0 != formats_count,
                        "Could not enumerate supported surface formats.");

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
      INFO("Desired combination of format and colorspace is not "
           "supported. Selecting other colorspace.");
      return true;
    }
  }

  image_format = surface_formats[0].format;
  image_color_space = surface_formats[0].colorSpace;
  INFO("Desired format is not supported. Selecting available format - "
       "colorspace combination.");
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

  CHECK_VULKAN(vkCreateSwapchainKHR(logical_device, &swapchain_create_info,
                                    nullptr, &swapchain));
  D_RETURN_FALSE_IF_NOT(VK_NULL_HANDLE != swapchain,
                        "Could not create a swapchain.");
  if (VK_NULL_HANDLE != old_swapchain) {
    vkDestroySwapchainKHR(logical_device, old_swapchain, nullptr);
    old_swapchain = VK_NULL_HANDLE;
  }
  return true;
}

void VulkanLibraryInterface::destroySwapchain(VkDevice logical_device,
                                              VkSwapchainKHR &swapchain) {
  if (swapchain) {
    vkDestroySwapchainKHR(logical_device, swapchain, nullptr);
    swapchain = VK_NULL_HANDLE;
  }
}

// SWAPCHAIN IMAGE OPERATIONS
// /////////////////////////////////////////////////

bool VulkanLibraryInterface::getHandlesOfSwapchainImages(
    VkDevice logical_device, VkSwapchainKHR swapchain,
    std::vector<VkImage> &swapchain_images) {
  uint32_t images_count = 0;
  CHECK_VULKAN(vkGetSwapchainImagesKHR(logical_device, swapchain, &images_count,
                                       nullptr));
  D_RETURN_FALSE_IF_NOT(0 != images_count,
                        "Could not get the number of swapchain images.");
  swapchain_images.resize(images_count);
  CHECK_VULKAN(vkGetSwapchainImagesKHR(logical_device, swapchain, &images_count,
                                       swapchain_images.data()));
  D_RETURN_FALSE_IF_NOT(0 != images_count,
                        "Could not enumerate swapchain images.");
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

// SYNCHRONIZATION ///////////////////////////////////////////////////////////

bool VulkanLibraryInterface::createSemaphore(VkDevice logical_device,
                                             VkSemaphore &semaphore) {
  VkSemaphoreCreateInfo semaphore_create_info = {
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, // VkStructureType sType
      nullptr, // const void               * pNext
      0        // VkSemaphoreCreateFlags     flags
  };
  CHECK_VULKAN(vkCreateSemaphore(logical_device, &semaphore_create_info,
                                 nullptr, &semaphore));
  return true;
}

void VulkanLibraryInterface::destroySemaphore(VkDevice logical_device,
                                              VkSemaphore &semaphore) {
  if (VK_NULL_HANDLE != semaphore) {
    vkDestroySemaphore(logical_device, semaphore, nullptr);
    semaphore = VK_NULL_HANDLE;
  }
}

bool VulkanLibraryInterface::createFence(VkDevice logical_device, bool signaled,
                                         VkFence &fence) {
  VkFenceCreateInfo fence_create_info = {
      VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, // VkStructureType        sType
      nullptr,                             // const void           * pNext
      signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u, // VkFenceCreateFlags flags
  };
  CHECK_VULKAN(
      vkCreateFence(logical_device, &fence_create_info, nullptr, &fence));
  return true;
}

void VulkanLibraryInterface::destroyFence(VkDevice logical_device,
                                          VkFence &fence) {
  if (VK_NULL_HANDLE != fence) {
    vkDestroyFence(logical_device, fence, nullptr);
    fence = VK_NULL_HANDLE;
  }
}

bool VulkanLibraryInterface::waitForFences(VkDevice logical_device,
                                           std::vector<VkFence> const &fences,
                                           VkBool32 wait_for_all,
                                           uint64_t timeout) {
  if (fences.size() > 0) {
    CHECK_VULKAN(vkWaitForFences(logical_device,
                                 static_cast<uint32_t>(fences.size()),
                                 fences.data(), wait_for_all, timeout));
    return true;
  }
  return false;
}

bool VulkanLibraryInterface::resetFences(VkDevice logical_device,
                                         std::vector<VkFence> const &fences) {
  if (fences.size() > 0) {
    CHECK_VULKAN(vkResetFences(
        logical_device, static_cast<uint32_t>(fences.size()), fences.data()));
    return true;
  }
  return false;
}

// VULKAN COMMAND BUFFERS ////////////////////////////////////////////////////

bool VulkanLibraryInterface::createCommandPool(
    VkDevice logical_device, VkCommandPoolCreateFlags parameters,
    uint32_t queue_family, VkCommandPool &command_pool) {
  VkCommandPoolCreateInfo command_pool_create_info = {
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, // VkStructureType sType
      nullptr,     // const void                 * pNext
      parameters,  // VkCommandPoolCreateFlags     flags
      queue_family // uint32_t                     queueFamilyIndex
  };
  CHECK_VULKAN(vkCreateCommandPool(logical_device, &command_pool_create_info,
                                   nullptr, &command_pool));
  return true;
}

void VulkanLibraryInterface::destroyCommandPool(VkDevice logical_device,
                                                VkCommandPool &command_pool) {
  if (VK_NULL_HANDLE != command_pool) {
    vkDestroyCommandPool(logical_device, command_pool, nullptr);
    command_pool = VK_NULL_HANDLE;
  }
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
  CHECK_VULKAN(vkAllocateCommandBuffers(
      logical_device, &command_buffer_allocate_info, command_buffers.data()));
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
  CHECK_VULKAN(
      vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));
  return true;
}

bool VulkanLibraryInterface::endCommandBufferRecordingOperation(
    VkCommandBuffer command_buffer) {
  CHECK_VULKAN(vkEndCommandBuffer(command_buffer));
  return true;
}

bool VulkanLibraryInterface::resetCommandBuffer(VkCommandBuffer command_buffer,
                                                bool release_resources) {
  CHECK_VULKAN(vkResetCommandBuffer(
      command_buffer,
      release_resources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0));
  return true;
}

bool VulkanLibraryInterface::resetCommandPool(VkDevice logical_device,
                                              VkCommandPool command_pool,
                                              bool release_resources) {
  CHECK_VULKAN(vkResetCommandPool(
      logical_device, command_pool,
      release_resources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0));
  return true;
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

// COMMAND BUFFER SUBMISSION ///////////////////////////////////////////////

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

  CHECK_VULKAN(vkQueueSubmit(queue, 1, &submit_info, fence));
  return true;
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

// COMMAND BUFFER SUBMISSION ///////////////////////////////////////////////

bool VulkanInterfaceLibrary::createShaderModule(
    VkDevice logical_device, std::vector<unsigned char> const &source_code,
    VkShaderModule &shader_module) {
  VkShaderModuleCreateInfo shader_module_create_info = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // VkStructureType sType
      nullptr,            // const void                 * pNext
      0,                  // VkShaderModuleCreateFlags    flags
      source_code.size(), // size_t                       codeSize
      reinterpret_cast<uint32_t const *>(
          source_code.data()) // const uint32_t             * pCode
  };
  CHECK_VULKAN(vkCreateShaderModule(logical_device, &shader_module_create_info,
                                    nullptr, &shader_module));
  return true;
}

void VulkanInterfaceLibrary::specifyPipelineShaderStages(
    std::vector<ShaderStageParameters> const &shader_stage_params,
    std::vector<VkPipelineShaderStageCreateInfo> &shader_stage_create_infos) {
  shader_stage_create_infos.clear();
  for (auto &shader_stage : shader_stage_params) {
    shader_stage_create_infos.push_back({
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // VkStructureType
                                                             // sType
        nullptr,                   // const void                       * pNext
        0,                         // VkPipelineShaderStageCreateFlags   flags
        shader_stage.ShaderStage,  // VkShaderStageFlagBits              stage
        shader_stage.ShaderModule, // VkShaderModule                     module
        shader_stage.EntryPointName, // const char                       * pName
        shader_stage.SpecializationInfo // const VkSpecializationInfo       *
                                        // pSpecializationInfo
    });
  }
}

void VulkanInterfaceLibrary::specifyPipelineVertexInputState(
    std::vector<VkVertexInputBindingDescription> const &binding_descriptions,
    std::vector<VkVertexInputAttributeDescription> const
        &attribute_descriptions,
    VkPipelineVertexInputStateCreateInfo &vertex_input_state_create_info) {
  vertex_input_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, // VkStructureType
                                                                 // sType
      nullptr, // const void                              * pNext
      0,       // VkPipelineVertexInputStateCreateFlags     flags
      static_cast<uint32_t>(
          binding_descriptions
              .size()),            // uint32_t vertexBindingDescriptionCount
      binding_descriptions.data(), // const VkVertexInputBindingDescription   *
                                   // pVertexBindingDescriptions
      static_cast<uint32_t>(
          attribute_descriptions
              .size()),             // uint32_t vertexAttributeDescriptionCount
      attribute_descriptions.data() // const VkVertexInputAttributeDescription *
                                    // pVertexAttributeDescriptions
  };
}

void VulkanInterfaceLibrary::specifyPipelineInputAssemblyState(
    VkPrimitiveTopology topology, bool primitive_restart_enable,
    VkPipelineInputAssemblyStateCreateInfo &input_assembly_state_create_info) {
  input_assembly_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, // VkStructureType
                                                                   // sType
      nullptr,  // const void                              * pNext
      0,        // VkPipelineInputAssemblyStateCreateFlags   flags
      topology, // VkPrimitiveTopology                       topology
      primitive_restart_enable // VkBool32 primitiveRestartEnable
  };
}

void VulkanInterfaceLibrary::specifyPipelineTessellationState(
    uint32_t patch_control_points_count,
    VkPipelineTessellationStateCreateInfo &tessellation_state_create_info) {
  tessellation_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, // VkStructureType
                                                                 // sType
      nullptr, // const void                               * pNext
      0,       // VkPipelineTessellationStateCreateFlags     flags
      patch_control_points_count // uint32_t patchControlPoints
  };
}

void VulkanInterfaceLibrary::specifyPipelineViewportAndScissorTestState(
    ViewportInfo const &viewport_infos,
    VkPipelineViewportStateCreateInfo &viewport_state_create_info) {
  uint32_t viewport_count =
      static_cast<uint32_t>(viewport_infos.Viewports.size());
  uint32_t scissor_count =
      static_cast<uint32_t>(viewport_infos.Scissors.size());
  viewport_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, // VkStructureType
                                                             // sType
      nullptr,        // const void                         * pNext
      0,              // VkPipelineViewportStateCreateFlags   flags
      viewport_count, // uint32_t                             viewportCount
      viewport_infos.Viewports.data(), // const VkViewport * pViewports
      scissor_count, // uint32_t                             scissorCount
      viewport_infos.Scissors
          .data() // const VkRect2D                     * pScissors
  };
}

void VulkanInterfaceLibrary::specifyPipelineRasterizationState(
    bool depth_clamp_enable, bool rasterizer_discard_enable,
    VkPolygonMode polygon_mode, VkCullModeFlags culling_mode,
    VkFrontFace front_face, bool depth_bias_enable,
    float depth_bias_constant_factor, float depth_bias_clamp,
    float depth_bias_slope_factor, float line_width,
    VkPipelineRasterizationStateCreateInfo &rasterization_state_create_info) {
  rasterization_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, // VkStructureType
                                                                  // sType
      nullptr,            // const void                               * pNext
      0,                  // VkPipelineRasterizationStateCreateFlags    flags
      depth_clamp_enable, // VkBool32 depthClampEnable
      rasterizer_discard_enable, // VkBool32 rasterizerDiscardEnable
      polygon_mode, // VkPolygonMode                              polygonMode
      culling_mode, // VkCullModeFlags                            cullMode
      front_face,   // VkFrontFace                                frontFace
      depth_bias_enable,          // VkBool32 depthBiasEnable
      depth_bias_constant_factor, // float depthBiasConstantFactor
      depth_bias_clamp,           // float depthBiasClamp
      depth_bias_slope_factor,    // float depthBiasSlopeFactor
      line_width // float                                      lineWidth
  };
}

void VulkanInterfaceLibrary::specifyPipelineMultisampleState(
    VkSampleCountFlagBits sample_count, bool per_sample_shading_enable,
    float min_sample_shading, VkSampleMask const *sample_masks,
    bool alpha_to_coverage_enable, bool alpha_to_one_enable,
    VkPipelineMultisampleStateCreateInfo &multisample_state_create_info) {
  multisample_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, // VkStructureType
                                                                // sType
      nullptr,      // const void                             * pNext
      0,            // VkPipelineMultisampleStateCreateFlags    flags
      sample_count, // VkSampleCountFlagBits rasterizationSamples
      per_sample_shading_enable, // VkBool32 sampleShadingEnable
      min_sample_shading,        // float minSampleShading
      sample_masks, // const VkSampleMask                     * pSampleMask
      alpha_to_coverage_enable, // VkBool32 alphaToCoverageEnable
      alpha_to_one_enable       // VkBool32 alphaToOneEnable
  };
}

void VulkanInterfaceLibrary::specifyPipelineDepthAndStencilState(
    bool depth_test_enable, bool depth_write_enable,
    VkCompareOp depth_compare_op, bool depth_bounds_test_enable,
    float min_depth_bounds, float max_depth_bounds, bool stencil_test_enable,
    VkStencilOpState front_stencil_test_parameters,
    VkStencilOpState back_stencil_test_parameters,
    VkPipelineDepthStencilStateCreateInfo
        &depth_and_stencil_state_create_info) {
  depth_and_stencil_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, // VkStructureType
                                                                  // sType
      nullptr,            // const void                               * pNext
      0,                  // VkPipelineDepthStencilStateCreateFlags     flags
      depth_test_enable,  // VkBool32 depthTestEnable
      depth_write_enable, // VkBool32 depthWriteEnable
      depth_compare_op,   // VkCompareOp depthCompareOp
      depth_bounds_test_enable,      // VkBool32 depthBoundsTestEnable
      stencil_test_enable,           // VkBool32 stencilTestEnable
      front_stencil_test_parameters, // VkStencilOpState front
      back_stencil_test_parameters,  // VkStencilOpState back
      min_depth_bounds,              // float minDepthBounds
      max_depth_bounds               // float maxDepthBounds
  };
}

void VulkanInterfaceLibrary::specifyPipelineBlendState(
    bool logic_op_enable, VkLogicOp logic_op,
    std::vector<VkPipelineColorBlendAttachmentState> const
        &attachment_blend_states,
    std::array<float, 4> const &blend_constants,
    VkPipelineColorBlendStateCreateInfo &blend_state_create_info) {
  blend_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, // VkStructureType
                                                                // sType
      nullptr,         // const void                                 * pNext
      0,               // VkPipelineColorBlendStateCreateFlags         flags
      logic_op_enable, // VkBool32 logicOpEnable
      logic_op,        // VkLogicOp                                    logicOp
      static_cast<uint32_t>(
          attachment_blend_states.size()), // uint32_t attachmentCount
      attachment_blend_states
          .data(), // const VkPipelineColorBlendAttachmentState  * pAttachments
      {// float                                        blendConstants[4]
       blend_constants[0], blend_constants[1], blend_constants[2],
       blend_constants[3]}};
}

void VulkanInterfaceLibrary::specifyPipelineDynamicStates(
    std::vector<VkDynamicState> const &dynamic_states,
    VkPipelineDynamicStateCreateInfo &dynamic_state_creat_info) {
  dynamic_state_creat_info = {
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, // VkStructureType
                                                            // sType
      nullptr, // const void                         * pNext
      0,       // VkPipelineDynamicStateCreateFlags    flags
      static_cast<uint32_t>(
          dynamic_states.size()), // uint32_t dynamicStateCount
      dynamic_states
          .data() // const VkDynamicState               * pDynamicStates
  };
}

bool VulkanInterfaceLibrary::createPipelineLayout(
    VkDevice logical_device,
    std::vector<VkDescriptorSetLayout> const &descriptor_set_layouts,
    std::vector<VkPushConstantRange> const &push_constant_ranges,
    VkPipelineLayout &pipeline_layout) {
  VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, // VkStructureType sType
      nullptr, // const void                     * pNext
      0,       // VkPipelineLayoutCreateFlags      flags
      static_cast<uint32_t>(
          descriptor_set_layouts.size()), // uint32_t setLayoutCount
      descriptor_set_layouts
          .data(), // const VkDescriptorSetLayout    * pSetLayouts
      static_cast<uint32_t>(
          push_constant_ranges.size()), // uint32_t pushConstantRangeCount
      push_constant_ranges
          .data() // const VkPushConstantRange      * pPushConstantRanges
  };
  CHECK_VULKAN(vkCreatePipelineLayout(
      logical_device, &pipeline_layout_create_info, nullptr, &pipeline_layout));
  return true;
}

void VulkanInterfaceLibrary::specifyGraphicsPipelineCreationParameters(
    VkPipelineCreateFlags additional_options,
    std::vector<VkPipelineShaderStageCreateInfo> const
        &shader_stage_create_infos,
    VkPipelineVertexInputStateCreateInfo const &vertex_input_state_create_info,
    VkPipelineInputAssemblyStateCreateInfo const
        &input_assembly_state_create_info,
    VkPipelineTessellationStateCreateInfo const *tessellation_state_create_info,
    VkPipelineViewportStateCreateInfo const *viewport_state_create_info,
    VkPipelineRasterizationStateCreateInfo const
        &rasterization_state_create_info,
    VkPipelineMultisampleStateCreateInfo const *multisample_state_create_info,
    VkPipelineDepthStencilStateCreateInfo const
        *depth_and_stencil_state_create_info,
    VkPipelineColorBlendStateCreateInfo const *blend_state_create_info,
    VkPipelineDynamicStateCreateInfo const *dynamic_state_creat_info,
    VkPipelineLayout pipeline_layout, VkRenderPass render_pass,
    uint32_t subpass, VkPipeline base_pipeline_handle,
    int32_t base_pipeline_index,
    VkGraphicsPipelineCreateInfo &graphics_pipeline_create_info) {
  graphics_pipeline_create_info = {
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, // VkStructureType sType
      nullptr, // const void                                   * pNext
      additional_options, // VkPipelineCreateFlags flags
      static_cast<uint32_t>(
          shader_stage_create_infos.size()), // uint32_t stageCount
      shader_stage_create_infos
          .data(), // const VkPipelineShaderStageCreateInfo        * pStages
      &vertex_input_state_create_info,   // const
                                         // VkPipelineVertexInputStateCreateInfo
                                         // * pVertexInputState
      &input_assembly_state_create_info, // const
                                         // VkPipelineInputAssemblyStateCreateInfo
                                         // * pInputAssemblyState
      tessellation_state_create_info, // const
                                      // VkPipelineTessellationStateCreateInfo
                                      // * pTessellationState
      viewport_state_create_info, // const VkPipelineViewportStateCreateInfo *
                                  // pViewportState
      &rasterization_state_create_info, // const
                                        // VkPipelineRasterizationStateCreateInfo
                                        // * pRasterizationState
      multisample_state_create_info, // const
                                     // VkPipelineMultisampleStateCreateInfo   *
                                     // pMultisampleState
      depth_and_stencil_state_create_info, // const
                                           // VkPipelineDepthStencilStateCreateInfo
                                           // * pDepthStencilState
      blend_state_create_info, // const VkPipelineColorBlendStateCreateInfo    *
                               // pColorBlendState
      dynamic_state_creat_info, // const VkPipelineDynamicStateCreateInfo *
                                // pDynamicState
      pipeline_layout, // VkPipelineLayout                               layout
      render_pass, // VkRenderPass                                   renderPass
      subpass,     // uint32_t                                       subpass
      base_pipeline_handle, // VkPipeline basePipelineHandle
      base_pipeline_index   // int32_t basePipelineIndex
  };
}

bool VulkanInterfaceLibrary::createPipelineCacheObject(
    VkDevice logical_device, std::vector<unsigned char> const &cache_data,
    VkPipelineCache &pipeline_cache) {
  VkPipelineCacheCreateInfo pipeline_cache_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO, // VkStructureType sType
      nullptr, // const void                   * pNext
      0,       // VkPipelineCacheCreateFlags     flags
      static_cast<uint32_t>(cache_data.size()), // size_t initialDataSize
      cache_data.data() // const void                   * pInitialData
  };
  CHECK_VULKAN(vkCreatePipelineCache(
      logical_device, &pipeline_cache_create_info, nullptr, &pipeline_cache));
  return true;
}

bool VulkanInterfaceLibrary::retrieveDataFromPipelineCache(
    VkDevice logical_device, VkPipelineCache pipeline_cache,
    std::vector<unsigned char> &pipeline_cache_data) {
  size_t data_size = 0;
  CHECK_VULKAN(vkGetPipelineCacheData(logical_device, pipeline_cache,
                                      &data_size, nullptr));
  if (0 == data_size) {
    std::cout << "Could not get the size of the pipeline cache." << std::endl;
    return false;
  }
  pipeline_cache_data.resize(data_size);

  CHECK_VULKAN(vkGetPipelineCacheData(logical_device, pipeline_cache,
                                      &data_size, pipeline_cache_data.data()));
  if (0 == data_size) {
    std::cout << "Could not acquire pipeline cache data." << std::endl;
    return false;
  }
  return true;
}

bool VulkanInterfaceLibrary::mergeMultiplePipelineCacheObjects(
    VkDevice logical_device, VkPipelineCache target_pipeline_cache,
    std::vector<VkPipelineCache> const &source_pipeline_caches) {
  if (source_pipeline_caches.size() > 0) {
    CHECK_VULKAN(vkMergePipelineCaches(
        logical_device, target_pipeline_cache,
        static_cast<uint32_t>(source_pipeline_caches.size()),
        source_pipeline_caches.data()));
    return true;
  }
  return false;
}

bool VulkanInterfaceLibrary::createGraphicsPipelines(
    VkDevice logical_device,
    std::vector<VkGraphicsPipelineCreateInfo> const
        &graphics_pipeline_create_infos,
    VkPipelineCache pipeline_cache,
    std::vector<VkPipeline> &graphics_pipelines) {
  if (graphics_pipeline_create_infos.size() > 0) {
    graphics_pipelines.resize(graphics_pipeline_create_infos.size());
    CHECK_VULKAN(vkCreateGraphicsPipelines(
        logical_device, pipeline_cache,
        static_cast<uint32_t>(graphics_pipeline_create_infos.size()),
        graphics_pipeline_create_infos.data(), nullptr,
        graphics_pipelines.data()));
    return true;
  }
  return false;
}

bool VulkanLibraryInterface::createComputePipeline(
    VkDevice logical_device, VkPipelineCreateFlags additional_options,
    VkPipelineShaderStageCreateInfo const &compute_shader_stage,
    VkPipelineLayout pipeline_layout, VkPipeline base_pipeline_handle,
    VkPipelineCache pipeline_cache, VkPipeline &compute_pipeline) {
  VkComputePipelineCreateInfo compute_pipeline_create_info = {
      VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, // VkStructureType sType
      nullptr,              // const void                       * pNext
      additional_options,   // VkPipelineCreateFlags              flags
      compute_shader_stage, // VkPipelineShaderStageCreateInfo    stage
      pipeline_layout,      // VkPipelineLayout                   layout
      base_pipeline_handle, // VkPipeline basePipelineHandle
      -1 // int32_t                            basePipelineIndex
  };
  CHECK_VULKAN(vkCreateComputePipelines(logical_device, pipeline_cache, 1,
                                        &compute_pipeline_create_info, nullptr,
                                        &compute_pipeline));
  return true;
}

void VulkanLibraryInterface::bindPipelineObject(
    VkCommandBuffer command_buffer, VkPipelineBindPoint pipeline_type,
    VkPipeline pipeline) {
  vkCmdBindPipeline(command_buffer, pipeline_type, pipeline);
}

void VulkanLibraryInterface::destroyPipeline(VkDevice logical_device,
                                             VkPipeline &pipeline) {
  if (VK_NULL_HANDLE != pipeline) {
    vkDestroyPipeline(logical_device, pipeline, nullptr);
    pipeline = VK_NULL_HANDLE;
  }
}

void VulkanLibraryInterface::destroyPipelineCache(
    VkDevice logical_device, VkPipelineCache &pipeline_cache) {
  if (VK_NULL_HANDLE != pipeline_cache) {
    vkDestroyPipelineCache(logical_device, pipeline_cache, nullptr);
    pipeline_cache = VK_NULL_HANDLE;
  }
}

void VulkanLibraryInterface::destroyPipelineLayout(
    VkDevice logical_device, VkPipelineLayout &pipeline_layout) {
  if (VK_NULL_HANDLE != pipeline_layout) {
    vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);
    pipeline_layout = VK_NULL_HANDLE;
  }
}

void VulkanLibraryInterface::destroyShaderModule(
    VkDevice logical_device, VkShaderModule &shader_module) {
  if (VK_NULL_HANDLE != shader_module) {
    vkDestroyShaderModule(logical_device, shader_module, nullptr);
    shader_module = VK_NULL_HANDLE;
  }
}
// UTILS
// /////////////////////////////////////////////////////////////////////

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

bool VulkanLibraryInterface::createLogicalDeviceWithWsiExtensionsEnabled(
    VkPhysicalDevice physical_device, std::vector<QueueFamilyInfo> queue_infos,
    std::vector<char const *> &desired_extensions,
    VkPhysicalDeviceFeatures *desired_features, VkDevice &logical_device) {
  desired_extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  return createLogicalDevice(physical_device, queue_infos, desired_extensions,
                             desired_features, logical_device);
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

    std::vector<QueueFamilyInfo> requested_queues = {
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

} // namespace vk

} // namespace circe