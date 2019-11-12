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
///\file vulkan_physical_device.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-10-18
///
///\brief

#include "vulkan_physical_device.h"
#include "logging.h"
#include "vulkan_debug.h"

namespace circe {

namespace vk {

PhysicalDevice::PhysicalDevice(VkPhysicalDevice device_handle)
    : vk_device_(device_handle) {
  if (!checkAvailableExtensions(vk_extensions_))
    return;
  vkGetPhysicalDeviceFeatures(vk_device_, &vk_features_);
  vkGetPhysicalDeviceProperties(vk_device_, &vk_properties_);
  vkGetPhysicalDeviceMemoryProperties(vk_device_, &vk_memory_properties_);
  checkAvailableQueueFamilies();
}

VkPhysicalDevice PhysicalDevice::handle() const { return vk_device_; }

bool PhysicalDevice::good() const { return vk_device_ != VK_NULL_HANDLE; }

bool PhysicalDevice::isExtensionSupported(
    const char *desired_instance_extension) const {
  for (auto &extension : vk_extensions_)
    if (std::string(extension.extensionName) ==
        std::string(desired_instance_extension))
      return true;
  return false;
}

bool PhysicalDevice::checkAvailableQueueFamilies() {
  uint32_t queue_families_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(vk_device_, &queue_families_count,
                                           nullptr);
  D_RETURN_FALSE_IF_NOT(queue_families_count != 0,
                        "Could not get the number of queue families.\n");
  vk_queue_families_.resize(queue_families_count);
  vkGetPhysicalDeviceQueueFamilyProperties(vk_device_, &queue_families_count,
                                           vk_queue_families_.data());
  D_RETURN_FALSE_IF_NOT(queue_families_count != 0,
                        "Could not acquire properties of queue families.\n");
  return true;
}

bool PhysicalDevice::selectIndexOfQueueFamily(
    VkQueueFlags desired_capabilities, uint32_t &queue_family_index) const {
  for (uint32_t index = 0;
       index < static_cast<uint32_t>(vk_queue_families_.size()); ++index) {
    if ((vk_queue_families_[index].queueCount > 0) &&
        ((vk_queue_families_[index].queueFlags & desired_capabilities) ==
         desired_capabilities)) {
      queue_family_index = index;
      return true;
    }
  }
  return false;
}

bool PhysicalDevice::selectIndexOfQueueFamily(
    VkSurfaceKHR presentation_surface, uint32_t &queue_family_index) const {
  for (uint32_t index = 0;
       index < static_cast<uint32_t>(vk_queue_families_.size()); ++index) {
    VkBool32 presentation_supported = VK_FALSE;
    VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
        vk_device_, index, presentation_surface, &presentation_supported);
    if ((VK_SUCCESS == result) && (VK_TRUE == presentation_supported)) {
      queue_family_index = index;
      return true;
    }
  }
  return false;
}

bool PhysicalDevice::checkAvailableExtensions(
    std::vector<VkExtensionProperties> &extensions) const {
  uint32_t extensions_count = 0;
  R_CHECK_VULKAN(vkEnumerateDeviceExtensionProperties(
      vk_device_, nullptr, &extensions_count, nullptr));
  if (extensions_count == 0)
    return true;
  extensions.resize(extensions_count);
  R_CHECK_VULKAN(vkEnumerateDeviceExtensionProperties(
      vk_device_, nullptr, &extensions_count, extensions.data()));
  return true;
}

bool PhysicalDevice::formatProperties(VkFormat format,
                                      VkFormatProperties &properties) const {
  vkGetPhysicalDeviceFormatProperties(vk_device_, format, &properties);
  return true;
}

bool PhysicalDevice::imageFormatProperties(
    VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags,
    VkImageFormatProperties &properties) const {
  vkGetPhysicalDeviceImageFormatProperties(vk_device_, format, type, tiling,
                                           usage, flags, &properties);
  return true;
}

uint32_t
PhysicalDevice::chooseHeap(const VkMemoryRequirements &memory_requirements,
                           VkMemoryPropertyFlags required_flags,
                           VkMemoryPropertyFlags preferred_flags) const {
  uint32_t selected_type = ~0u;
  uint32_t memory_type;
  for (memory_type = 0; memory_type < 32; ++memory_type) {
    if (memory_requirements.memoryTypeBits & (1 << memory_type)) {
      const VkMemoryType &type = vk_memory_properties_.memoryTypes[memory_type];
      if ((type.propertyFlags & preferred_flags) == preferred_flags) {
        selected_type = memory_type;
        break;
      }
    }
  }
  if (selected_type != ~0u) {
    for (memory_type = 0; memory_type < 32; ++memory_type) {
      if (memory_requirements.memoryTypeBits & (1 << memory_type)) {
        const VkMemoryType &type =
            vk_memory_properties_.memoryTypes[memory_type];
        if ((type.propertyFlags & required_flags) == required_flags) {
          selected_type = memory_type;
          break;
        }
      }
    }
  }
  return selected_type;
}

bool PhysicalDevice::selectPresentationMode(
    VkSurfaceKHR presentation_surface, VkPresentModeKHR desired_present_mode,
    VkPresentModeKHR &present_mode) const {
  // Enumerate supported present modes
  uint32_t present_modes_count = 0;
  R_CHECK_VULKAN(vkGetPhysicalDeviceSurfacePresentModesKHR(
      vk_device_, presentation_surface, &present_modes_count, nullptr));
  D_RETURN_FALSE_IF_NOT(0 != present_modes_count,
                        "Could not get the number of supported present modes.");
  std::vector<VkPresentModeKHR> present_modes(present_modes_count);
  R_CHECK_VULKAN(vkGetPhysicalDeviceSurfacePresentModesKHR(
      vk_device_, presentation_surface, &present_modes_count,
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

bool PhysicalDevice::selectFormatOfSwapchainImages(
    VkSurfaceKHR presentation_surface,
    VkSurfaceFormatKHR desired_surface_format, VkFormat &image_format,
    VkColorSpaceKHR &image_color_space) const {
  // Enumerate supported formats
  uint32_t formats_count = 0;

  R_CHECK_VULKAN(vkGetPhysicalDeviceSurfaceFormatsKHR(
      vk_device_, presentation_surface, &formats_count, nullptr));
  D_RETURN_FALSE_IF_NOT(
      0 != formats_count,
      "Could not get the number of supported surface formats.");

  std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
  R_CHECK_VULKAN(vkGetPhysicalDeviceSurfaceFormatsKHR(
      vk_device_, presentation_surface, &formats_count,
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

bool PhysicalDevice::surfaceCapabilities(
    VkSurfaceKHR surface,
    VkSurfaceCapabilitiesKHR &surface_capabilities) const {
  R_CHECK_VULKAN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      vk_device_, surface, &surface_capabilities));
  return true;
}

const VkPhysicalDeviceProperties &PhysicalDevice::properties() const {
  return vk_properties_;
}

std::ostream &operator<<(std::ostream &os, VkPhysicalDeviceType e) {
#define PRINT_IF_ENUM(E)                                                       \
  if (e == E)                                                                  \
    os << #E;
  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_OTHER);
  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_CPU);
  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_BEGIN_RANGE);
  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_END_RANGE);
  PRINT_IF_ENUM(VK_PHYSICAL_DEVICE_TYPE_RANGE_SIZE);
#undef PRINT_IF_ENUM
  return os;
}

std::ostream &operator<<(std::ostream &os, const PhysicalDevice &d) {
  auto properties = d.properties();
  os << "PHYSICAL DEVICE INFO =====================" << std::endl;
#define PRINT_FIELD(F) std::cerr << #F << " = " << F << std::endl;
  PRINT_FIELD(properties.deviceName);
  PRINT_FIELD(properties.deviceType);
  PRINT_FIELD(properties.deviceID);
  PRINT_FIELD(properties.vendorID);
  PRINT_FIELD(properties.apiVersion);
  PRINT_FIELD(properties.driverVersion);
#undef PRINT_FIELD
  os << "==========================================" << std::endl;
  return os;
}

} // namespace vk

} // namespace circe