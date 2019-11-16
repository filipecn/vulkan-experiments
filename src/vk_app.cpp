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
#include "logging.h"
#include "vulkan_library.h"
#include <map>

namespace circe {

namespace vk {

App::App(uint32_t w, uint32_t h, const std::string &name)
    : application_name_(name),
      graphics_display_(new GraphicsDisplay(w, h, name)) {}

App::~App() {
  /*for (auto image_view : swap_chain_image_views_) {
    vkDestroyImageView(device_, image_view, nullptr);
  }
  VulkanLibraryInterface::destroySwapchain(device_, swap_chain_);
  VulkanLibraryInterface::destroyLogicalDevice(device_);
  VulkanLibraryInterface::destroyPresentationSurface(instance_, surface_);
  VulkanLibraryInterface::destroyVulkanInstance(instance_);*/
}

void App::run(const std::function<void()> &render_callback) {
  if (!swapchain_)
    setupSwapChain(VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
  graphics_display_->open(render_callback);
}

void App::exit() { graphics_display_->close(); }

void App::setValidationLayers(
    const std::vector<const char *> &validation_layer_names,
    bool instance_level, bool device_level) {
  validation_layer_names_ = validation_layer_names;
}

bool App::setInstance(const std::vector<const char *> &extensions) {
  auto es = extensions;
  auto window_extensions = graphics_display_->requiredVkExtensions();
  for (auto e : window_extensions)
    es.emplace_back(e);
  instance_ = std::make_unique<Instance>(application_name_, es,
                                         validation_layer_names_);
  graphics_display_->createWindowSurface(*instance_.get(), surface_);
  return instance_->good();
}

bool App::pickPhysicalDevice(
    const std::function<uint32_t(const PhysicalDevice &, QueueFamilies &)> &f) {
  if (!instance_)
    RETURN_FALSE_IF_NOT(setInstance());
  std::vector<PhysicalDevice> physical_devices;
  instance_->enumerateAvailablePhysicalDevices(physical_devices);
  // ordered map of <score, device index>
  std::multimap<uint32_t, uint32_t> candidates;
  std::vector<QueueFamilies> queue_families(physical_devices.size());
  for (uint32_t i = 0; i < physical_devices.size(); ++i) {
    uint32_t presentation_family = 0;
    uint32_t graphics_family = 0;
    // find a family that supports presentation and graphics
    if (physical_devices[i].selectIndexOfQueueFamily(surface_,
                                                     presentation_family) &&
        physical_devices[i].selectIndexOfQueueFamily(VK_QUEUE_GRAPHICS_BIT,
                                                     graphics_family)) {
      queue_families[i].add(graphics_family, "graphics");
      queue_families[i].add(presentation_family, "presentation");
      candidates.insert(
          std::make_pair(f(physical_devices[i], queue_families[i]), i));
    }
  }
  CHECK_INFO(candidates.size() && candidates.rbegin()->first > 0,
             "failed to find a suitable GPU!");
  uint32_t selected_index = candidates.rbegin()->second;
  physical_device_ =
      std::make_unique<PhysicalDevice>(physical_devices[selected_index]);
  queue_families_ = queue_families[selected_index];
  return true;
}

bool App::createLogicalDevice(
    const std::vector<const char *> &desired_extensions,
    VkPhysicalDeviceFeatures *desired_features) {
  if (!physical_device_)
    pickPhysicalDevice([&](const circe::vk::PhysicalDevice &d,
                           circe::vk::QueueFamilies &q) -> uint32_t {
      if (d.properties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        return 1000;
      return 1;
    });
  auto extensions = desired_extensions;
  extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  logical_device_ = std::make_unique<LogicalDevice>(
      *physical_device_.get(), extensions, desired_features, queue_families_,
      validation_layer_names_);
  return logical_device_->good();
}

bool App::setupSwapChain(VkFormat desired_format,
                         VkColorSpaceKHR desired_color_space) {
  if (!logical_device_)
    createLogicalDevice();
  // PRESENTATION MODE
  VkPresentModeKHR present_mode;
  if (!physical_device_->selectPresentationMode(
          surface_, VK_PRESENT_MODE_MAILBOX_KHR, present_mode))
    return false;
  // CHECK SURFACE CAPABILITIES
  VkSurfaceCapabilitiesKHR surface_capabilities;
  if (!physical_device_->surfaceCapabilities(surface_, surface_capabilities))
    return false;
  // GET NUMBER OF SWAPCHAIN IMAGES
  uint32_t number_of_images = 0;
  selectNumberOfSwapchainImages(surface_capabilities, number_of_images);
  // QUERY IMAGE SIZE
  VkExtent2D swap_chain_image_size;
  if (!chooseSizeOfSwapchainImages(surface_capabilities, swap_chain_image_size))
    return false;
  if ((0 == swap_chain_image_size.width) || (0 == swap_chain_image_size.height))
    return false;
  // USAGE
  VkImageUsageFlags image_usage = surface_capabilities.supportedUsageFlags &
                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  if (image_usage != VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    return false;
  // IMAGE TRANSFORM
  VkSurfaceTransformFlagBitsKHR surface_transform =
      surface_capabilities.currentTransform;
  if (surface_capabilities.supportedTransforms &
      VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    surface_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  // COLOR SPACE
  VkFormat image_format;
  VkColorSpaceKHR image_color_space;
  if (!physical_device_->selectFormatOfSwapchainImages(
          surface_, {desired_format, desired_color_space}, image_format,
          image_color_space))
    return false;
  // SWAP CHAIN
  VkSurfaceFormatKHR surface_format = {image_format, image_color_space};
  swapchain_ = std::make_unique<Swapchain>(
      logical_device_.get(), surface_, number_of_images, surface_format,
      swap_chain_image_size, image_usage, surface_transform, present_mode);
  // CREATE IMAGE VIEWS
  const auto &swap_chain_images = swapchain_->images();
  for (uint32_t i = 0; i < swap_chain_images.size(); i++)
    swap_chain_image_views_.emplace_back(&swap_chain_images[i],
                                         VK_IMAGE_VIEW_TYPE_2D, image_format,
                                         VK_IMAGE_ASPECT_COLOR_BIT);
  return true;
}

const LogicalDevice *App::logicalDevice() {
  if (!logical_device_)
    createLogicalDevice();
  return logical_device_.get();
}

const Swapchain *App::swapchain() {
  if (!swapchain_)
    setupSwapChain(VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
  return swapchain_.get();
}

const std::vector<Image::View> &App::swapchainImageViews() {
  if (!swapchain_)
    setupSwapChain(VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
  return swap_chain_image_views_;
}

QueueFamilies &App::queueFamilies() { return queue_families_; }

bool App::selectNumberOfSwapchainImages(
    VkSurfaceCapabilitiesKHR const &surface_capabilities,
    uint32_t &number_of_images) const {
  number_of_images = surface_capabilities.minImageCount + 1;
  if ((surface_capabilities.maxImageCount > 0) &&
      (number_of_images > surface_capabilities.maxImageCount)) {
    number_of_images = surface_capabilities.maxImageCount;
  }
  return true;
}

bool App::chooseSizeOfSwapchainImages(
    VkSurfaceCapabilitiesKHR const &surface_capabilities,
    VkExtent2D &size_of_images) const {
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

} // namespace vk

} // namespace circe