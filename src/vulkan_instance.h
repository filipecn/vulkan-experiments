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
///\file vulkan_instance.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-10-16
///
///\brief

#ifndef CIRCE_VULKAN_INSTANCE_H
#define CIRCE_VULKAN_INSTANCE_H

#include "vulkan_physical_device.h"
#include <string>
#include <vector>

namespace circe {

namespace vk {

class SupportInfo {
public:
  SupportInfo();
  bool isValidationLayerSupported(const char *validation_layer) const;
  /// Checks if extension is supported by the instance
  ///\param desired_instance_extension **[in]** extension name (ex: )
  ///\return bool true if extension is supported
  bool
  isInstanceExtensionSupported(const char *desired_instance_extension) const;

private:
  /// Gets the list of the properties of supported instance extensions on the
  /// current hardware platform.
  /// \param extensions **[out]** list of extensions
  /// \return bool true if success
  bool checkAvailableExtensions(std::vector<VkExtensionProperties> &extensions);
  bool checkAvailableValidationLayers(
      std::vector<VkLayerProperties> &validation_layers);

  static bool first_build_flag;
  static std::vector<VkExtensionProperties> vk_extensions_;
  static std::vector<VkLayerProperties> vk_validation_layers_;
};

///\brief Vulkan Instance object handle
/// The Vulkan Instance holds all kinds of information about the application,
/// such as application name, version, etc. The instance is the interface
/// between the application and the Vulkan Library.
class Instance final {
public:
  ///\brief Construct a new Instance object
  ///\param application_name **[in]**
  ///\param desired_instance_extensions **[in | default = {}]**
  Instance(std::string application_name,
           const std::vector<const char *> &desired_instance_extensions =
               std::vector<const char *>(),
           const std::vector<const char *> &validation_layers =
               std::vector<const char *>());
  ///\brief Default destructor
  ~Instance();
  VkInstance handle() const;
  /// Checks with instance object construction succeded
  ///\return bool true if this can be used
  bool good() const;
  ///\brief
  ///
  ///\param physical_devices **[in]**
  ///\return bool
  bool enumerateAvailablePhysicalDevices(
      std::vector<PhysicalDevice> &physical_devices) const;

private:
  VkInstance vk_instance_ = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT vk_debug_messenger_;
};

} // namespace vk

} // namespace circe

#endif
