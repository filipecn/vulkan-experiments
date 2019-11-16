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

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
  return VK_FALSE;
}

void populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &create_info) {
  create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = debugCallback;
}

bool SupportInfo::first_build_flag = true;
std::vector<VkExtensionProperties> SupportInfo::vk_extensions_;
std::vector<VkLayerProperties> SupportInfo::vk_validation_layers_;

SupportInfo::SupportInfo() {
  if (first_build_flag) {
    ASSERT(checkAvailableExtensions(vk_extensions_));
    ASSERT(checkAvailableValidationLayers(vk_validation_layers_));
  }
}

bool SupportInfo::checkAvailableExtensions(
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

bool SupportInfo::checkAvailableValidationLayers(
    std::vector<VkLayerProperties> &validation_layers) {
  uint32_t layer_count = 0;
  R_CHECK_VULKAN(vkEnumerateInstanceLayerProperties(&layer_count, nullptr));
  ASSERT(layer_count != 0);
  validation_layers.resize(layer_count);
  R_CHECK_VULKAN(vkEnumerateInstanceLayerProperties(&layer_count,
                                                    validation_layers.data()));
  ASSERT(layer_count != 0);
  return true;
}

bool SupportInfo::isInstanceExtensionSupported(
    const char *desired_instance_extension) const {
  for (const auto &extension : vk_extensions_)
    if (std::string(extension.extensionName) ==
        std::string(desired_instance_extension))
      return true;
  return false;
}

bool SupportInfo::isValidationLayerSupported(
    const char *validation_layer) const {
  for (const auto &layer : vk_validation_layers_)
    if (std::string(layer.layerName) == std::string(validation_layer))
      return true;
  return false;
}

Instance::Instance(std::string application_name,
                   const std::vector<const char *> &desired_instance_extensions,
                   const std::vector<const char *> &validation_layers) {
  SupportInfo support_info;
  std::vector<const char *> instance_extensions = desired_instance_extensions;
  instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  for (auto &extension : instance_extensions)
    if (!support_info.isInstanceExtensionSupported(extension)) {
      INFO(concat("Extension named '", extension, "' is not supported."));
      return;
    }
  for (auto &layer : validation_layers)
    if (!support_info.isValidationLayerSupported(layer)) {
      INFO(concat("Validation layer named '", layer, "' is not supported."));
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
  create_info.enabledLayerCount = validation_layers.size();
  create_info.ppEnabledLayerNames =
      (validation_layers.size()) ? validation_layers.data() : nullptr;
  create_info.enabledExtensionCount =
      static_cast<uint32_t>(instance_extensions.size());
  create_info.ppEnabledExtensionNames =
      (instance_extensions.size()) ? instance_extensions.data() : nullptr;

  VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
  populateDebugMessengerCreateInfo(debug_create_info);
  create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_create_info;

  CHECK_VULKAN(vkCreateInstance(&create_info, nullptr, &vk_instance_));

  VkDebugUtilsMessengerCreateInfoEXT debug_after_create_info;
  populateDebugMessengerCreateInfo(debug_after_create_info);
  if (CreateDebugUtilsMessengerEXT(vk_instance_, &debug_after_create_info,
                                   nullptr, &vk_debug_messenger_) != VK_SUCCESS)
    INFO("failed to set up debug messenger!");
}

Instance::~Instance() {
  if (vk_instance_) {
    DestroyDebugUtilsMessengerEXT(vk_instance_, vk_debug_messenger_, nullptr);
    vkDestroyInstance(vk_instance_, nullptr);
  }
}

VkInstance Instance::handle() const { return vk_instance_; }

bool Instance::good() const { return vk_instance_ != VK_NULL_HANDLE; }

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