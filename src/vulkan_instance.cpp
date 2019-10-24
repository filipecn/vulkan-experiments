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
///\file vulkan_instance.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-10-16
///
///\brief

#include "vulkan_instance.h"
#include "logging.h"
#include "vulkan_debug.h"

namespace circe {

namespace vk {

bool Instance::checkAvailableExtensions(
    std::vector<VkExtensionProperties> &extensions) {
  uint32_t extensions_count = 0;
  R_CHECK_VULKAN(vkEnumerateInstanceExtensionProperties(
      nullptr, &extensions_count, nullptr));
  ASSERT(extensions_count != 0);
  extensions.resize(extensions_count);
  R_CHECK_VULKAN(vkEnumerateInstanceExtensionProperties(
      nullptr, &extensions_count, &extensions[0]));
  ASSERT(extensions_count != 0);
  return true;
}

Instance::Instance(
    std::string application_name,
    const std::vector<const char *> &desired_instance_extensions) {
  if (!checkAvailableExtensions(vk_extensions_))
    return;
  for (auto &extension : desired_instance_extensions)
    if (!isExtensionSupported(extension)) {
      INFO(concat("Extension named '", extension, "' is not supported."));
      return;
    }
  VkApplicationInfo info;
  info.pApplicationName = application_name.c_str();
  info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  info.apiVersion = VK_MAKE_VERSION(1, 0, 0);
  info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  info.pNext = nullptr;
  info.pEngineName = "circe";
  VkInstanceCreateInfo create_info;
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.pApplicationInfo = &info;
  create_info.enabledLayerCount = 0;
  create_info.ppEnabledLayerNames = nullptr;
  create_info.enabledExtensionCount =
      static_cast<uint32_t>(desired_instance_extensions.size());
  create_info.ppEnabledExtensionNames = (desired_instance_extensions.size())
                                            ? desired_instance_extensions.data()
                                            : nullptr;
  CHECK_VULKAN(vkCreateInstance(&create_info, nullptr, &vk_instance_));
}

Instance::~Instance() {
  if (vk_instance_)
    vkDestroyInstance(vk_instance_, nullptr);
}

bool Instance::good() const { return vk_instance_ != VK_NULL_HANDLE; }

bool Instance::isExtensionSupported(
    const char *desired_instance_extension) const {
  for (auto &extension : vk_extensions_)
    if (std::string(extension.extensionName) ==
        std::string(desired_instance_extension))
      return true;
  return false;
}

bool Instance::enumerateAvailablePhysicalDevices(
    std::vector<PhysicalDevice> &physical_devices) const {
  uint32_t devices_count = 0;
  R_CHECK_VULKAN(
      vkEnumeratePhysicalDevices(vk_instance_, &devices_count, nullptr));
  D_RETURN_FALSE_IF_NOT(
      devices_count != 0,
      "Could not get the number of available physical devices.");
  std::vector<VkPhysicalDevice> devices(devices_count);
  R_CHECK_VULKAN(
      vkEnumeratePhysicalDevices(vk_instance_, &devices_count, devices.data()));
  D_RETURN_FALSE_IF_NOT(devices_count != 0,
                        "Could not enumerate physical devices.");
  for (auto &device : devices)
    physical_devices.emplace_back(device);

  return true;
}

} // namespace vk

} // namespace circe