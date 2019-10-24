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
///\file vulkan_physical_device.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-10-18
///
///\brief

#include "vulkan_library.h"

namespace circe {

namespace vk {

/// Physical devices are the hardware we intend to use with Vulkan. Thus we
/// need to look for the devices that supports the features we need. We can
/// select any number of graphics cards and use them simultaneously.
/// The Vulkan Library allows us to get devices capabilities and properties so
/// we can select the one that best suits for our application.
///
/// Every Vulkan operation requires commands that are submitted to a queue.
/// Different queues can be processed independently and may support different
/// types of operations.
/// Queues with the same capabilities are grouped into families. A device may
/// expose any number of queue families. For example, there could be a queue
/// family that only allows processing of compute commands or one that only
/// allows memory transfer related commands.
class PhysicalDevice {
public:
  ///\brief Construct a new Physical Device object
  ///
  ///\param device_handle **[in]**
  PhysicalDevice(VkPhysicalDevice device_handle);
  ///\brief Default destructor
  ~PhysicalDevice() = default;
  ///\brief
  ///\return VkPhysicalDevice vulkan handle
  VkPhysicalDevice handle() const;
  /// Checks with physical device object construction succeded
  ///\return bool true if this can be used
  bool good() const;
  /// Checks if extension is supported by the device
  ///\param desired_device_extension **[in]** extension name (ex: )
  ///\return bool true if extension is supported
  bool isExtensionSupported(const char *desired_device_extension) const;
  /// Finds the queue family index that supports the desired set of
  /// capabilities.
  /// \param desired_capabilities **[in]** desired set of capabalities
  /// \param queue_family_index **[out]** capable queue family index
  /// \return bool true if success
  bool selectIndexOfQueueFamily(VkQueueFlags desired_capabilities,
                                uint32_t &queue_family_index) const;
  /// Finds a queue family of a physical device that can accept commands for a
  /// given surface
  /// \param presentation_surface **[in]** surface handle
  /// \param queue_family_index **[out]** queue family index
  /// \return bool true if success
  bool selectIndexOfQueueFamily(VkSurfaceKHR presentation_surface,
                                uint32_t &queue_family_index) const;
  /// Gets the properties and level of support for a given format.
  ///\param format **[in]**
  ///\param properties **[out]**
  ///\return bool true if success
  bool formatProperties(VkFormat format, VkFormatProperties &properties) const;
  ///\brief Reports support for the format
  ///
  ///\param format **[in]**
  ///\param type **[in]** ex: VK_IMAGE_TYPE_[1D, 2D, 3D]
  ///\param tiling **[in]** ex: VK_IMAGE_TILING_[LINEAR, OPTIMAL]
  ///\param usage **[in]**
  ///\param flags **[in]**
  ///\param properties **[in]**
  ///\return bool
  bool imageFormatProperties(VkFormat format, VkImageType type,
                             VkImageTiling tiling, VkImageUsageFlags usage,
                             VkImageCreateFlags flags,
                             VkImageFormatProperties &properties) const;

private:
  /// Retrieve available queue families exposed by a physical device
  /// \return bool true if success
  bool checkAvailableQueueFamilies();
  /// Gets the list of the properties of supported extensions for the device.
  /// \param extensions **[out]** list of extensions
  /// \return bool true if success
  bool checkAvailableExtensions(
      std::vector<VkExtensionProperties> &extensions) const;

  VkPhysicalDevice vk_device_ = VK_NULL_HANDLE;
  std::vector<VkExtensionProperties>
      vk_extensions_; //!< Available device extensions
  VkPhysicalDeviceFeatures
      vk_features_; //< Device features include items such as geometry and
                    // tesselation shaders, depth clamp, etc.
  VkPhysicalDeviceProperties
      vk_properties_; //!< Device properties describe general information surch
                      //!< as name, version of a driver, type of the device
                      //!< (integrated or discrete), memory, etc.
  VkPhysicalDeviceMemoryProperties
      vk_memory_properties_; //!< Physical device memory properties such as
                             //!< number of heaps, sizes, types and etc.
  std::vector<VkQueueFamilyProperties> vk_queue_families_;
};

} // namespace vk

} // namespace circe