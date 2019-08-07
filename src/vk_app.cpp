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
/// \file vk_app.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-08-04
///
/// \brief

#include "vk_app.h"
#include "vulkan_library.h"

namespace circe {

namespace vk {

App::App(size_t w, size_t h, const std::string &name)
    : application_name_(name),
      graphics_display_(new GraphicsDisplay(w, h, name)) {}

App::~App() {
  VulkanLibraryInterface::destroyLogicalDevice(device_);
  //   if (enableValidationLayers) {
  //     DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
  //   }
  if (instance_ && surface_)
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
  VulkanLibraryInterface::destroyVulkanInstance(instance_);
}

void App::run() { graphics_display_->open(); }

void App::exit() { graphics_display_->close(); }

bool App::createInstance(const std::vector<const char *> &extensions) {
  auto es = extensions;
  auto window_extensions = graphics_display_->requiredVkExtensions();
  for (auto e : window_extensions)
    es.emplace_back(e);
  if (!VulkanLibraryInterface::createInstance(es, application_name_, instance_))
    return false;
  if (!VulkanLibraryInterface::loadInstanceLevelFunctions(instance_, es))
    return false;
  return graphics_display_->createWindowSurface(instance_, surface_);
}

void App::pickPhysicalDevice(
    const std::function<bool(const VulkanLibraryInterface::PhysicalDevice &)>
        &f) {
  std::vector<VkPhysicalDevice> physical_devices;
  VulkanLibraryInterface::enumerateAvailablePhysicalDevices(instance_,
                                                            physical_devices);
  for (auto &physical_device : physical_devices) {
    VkPhysicalDeviceFeatures device_features;
    VkPhysicalDeviceProperties device_properties;
    VulkanLibraryInterface::getFeaturesAndPropertiesOfPhysicalDevice(
        physical_device, device_features, device_properties);
    std::vector<VkQueueFamilyProperties> queue_families;
    VulkanLibraryInterface::checkAvailableQueueFamiliesAndTheirProperties(
        physical_device, queue_families);
    VulkanLibraryInterface::PhysicalDevice p_device = {
        physical_device, device_features, device_properties, queue_families};
    // find a present family
    uint32_t family_index = 0;
    if (VulkanLibraryInterface::
            selectQueueFamilyThatSupportsPresentationToGivenSurface(
                physical_device, surface_, family_index))
      if (f(p_device)) {
        present_family_.family_index = family_index;
        present_family_.priorities = {1.f};
        physical_device_ = physical_device;
        return;
      }
  }
}

bool App::createLogicalDevice(
    const std::vector<VulkanLibraryInterface::QueueFamilyInfo> &queue_infos,
    const std::vector<const char *> &desired_extensions,
    VkPhysicalDeviceFeatures *desired_features) {
  return VulkanLibraryInterface::createLogicalDevice(
      physical_device_, queue_infos, desired_extensions, desired_features,
      device_);
}

bool App::setupSwapChain(VkFormat desired_format,
                         VkColorSpaceKHR desired_color_space) {
  // PRESENTATION MODE
  VkPresentModeKHR desired_present_mode;
  if (!VulkanLibraryInterface::selectDesiredPresentationMode(
          physical_device_, surface_, VK_PRESENT_MODE_MAILBOX_KHR,
          desired_present_mode))
    return false;
  // CHECK SURFACE CAPABILITIES
  VkSurfaceCapabilitiesKHR surface_capabilities;
  if (!VulkanLibraryInterface::getCapabilitiesOfPresentationSurface(
          physical_device_, surface_, surface_capabilities))
    return false;
  // GET NUMBER OF IMAGES
  uint32_t number_of_images;
  if (!VulkanLibraryInterface::selectNumberOfSwapchainImages(
          surface_capabilities, number_of_images))
    return false;
  // QUERY IMAGE SIZE
  VkExtent2D image_size = {graphics_display_->width(),
                           graphics_display_->height()};
  if (!VulkanLibraryInterface::chooseSizeOfSwapchainImages(surface_capabilities,
                                                           image_size))
    return false;
  if ((0 == image_size.width) || (0 == image_size.height))
    return false;
  // USAGE
  VkImageUsageFlags image_usage;
  if (!VulkanLibraryInterface::selectDesiredUsageScenariosOfSwapchainImages(
          surface_capabilities, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
          image_usage))
    return false;
  // IMAGE TRANSFORM
  VkSurfaceTransformFlagBitsKHR surface_transform;
  if (!VulkanLibraryInterface::selectTransformationOfSwapchainImages(
          surface_capabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
          surface_transform))
    return false;
  // COLOR SPACE
  VkFormat image_format;
  VkColorSpaceKHR image_color_space;
  if (!VulkanLibraryInterface::selectFormatOfSwapchainImages(
          physical_device_, surface_, {desired_format, desired_color_space},
          image_format, image_color_space))
    return false;
  // SWAP CHAIN
  if (!VulkanLibraryInterface::createSwapchain(
          device_, surface_, number_of_images,
          {image_format, image_color_space}, image_size, image_usage,
          surface_transform, desired_present_mode, old_swap_chain_,
          swap_chain_))
    return false;
  if (!VulkanLibraryInterface::getHandlesOfSwapchainImages(device_, swap_chain_,
                                                           swap_chain_images_))
    return false;
  return true;
}

} // namespace vk

} // namespace circe