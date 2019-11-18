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
      graphics_display_(new GraphicsDisplay(w, h, name)) {
  graphics_display_->resize_callback = [&](int new_w, int new_h) {
    framebuffer_resized_ = true;
  };
}

App::~App() {
  destroySwapchain();
  if (vk_surface_ != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(instance_->handle(), vk_surface_, nullptr);
}

void App::run(const std::function<void()> &render_callback) {
  recreateSwapchain();
  images_in_flight_.resize(framebuffers_.size(), VK_NULL_HANDLE);
  auto draw_callback = [&]() { this->draw(); };
  // graphics_display_->open(render_callback);
  graphics_display_->open(draw_callback);
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
  graphics_display_->createWindowSurface(instance_.get(), vk_surface_);
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
    if (physical_devices[i].selectIndexOfQueueFamily(vk_surface_,
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

  if (!command_pool_) {
    command_pool_ = std::make_unique<CommandPool>(
        logicalDevice(), 0,
        queue_families_.family("graphics").family_index.value());
  }
  { // setup synchronization objects
    for (size_t i = 0; i < max_frames_in_flight; ++i) {
      in_flight_fences_.emplace_back(logicalDevice(),
                                     VK_FENCE_CREATE_SIGNALED_BIT);
      image_available_semaphores_.emplace_back(logicalDevice());
      render_finished_semaphores_.emplace_back(logicalDevice());
    }
  }

  return logical_device_->good();
}

bool App::setupSwapChain(VkFormat desired_format,
                         VkColorSpaceKHR desired_color_space) {
  if (!logical_device_)
    createLogicalDevice();
  // PRESENTATION MODE
  VkPresentModeKHR present_mode;
  if (!physical_device_->selectPresentationMode(
          vk_surface_, VK_PRESENT_MODE_MAILBOX_KHR, present_mode))
    return false;
  // CHECK SURFACE CAPABILITIES
  VkSurfaceCapabilitiesKHR surface_capabilities;
  if (!physical_device_->surfaceCapabilities(vk_surface_, surface_capabilities))
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
          vk_surface_, {desired_format, desired_color_space}, image_format,
          image_color_space))
    return false;
  // SWAP CHAIN
  VkSurfaceFormatKHR surface_format = {image_format, image_color_space};
  if (!swapchain_)
    swapchain_ = std::make_unique<Swapchain>(
        logical_device_.get(), vk_surface_, number_of_images, surface_format,
        swap_chain_image_size, image_usage, surface_transform, present_mode);
  else
    swapchain_->set(vk_surface_, number_of_images, surface_format,
                    swap_chain_image_size, image_usage, surface_transform,
                    present_mode);
  // CREATE IMAGE VIEWS
  const auto &swap_chain_images = swapchain_->images();
  for (uint32_t i = 0; i < swap_chain_images.size(); i++)
    swapchain_image_views_.emplace_back(&swap_chain_images[i],
                                        VK_IMAGE_VIEW_TYPE_2D, image_format,
                                        VK_IMAGE_ASPECT_COLOR_BIT);
  // setup framebuffers
  for (auto &image_view : swapchain_image_views_) {
    circe::vk::Framebuffer framebuffer(logical_device_.get(), renderpass(),
                                       swapchain_->imageSize().width,
                                       swapchain_->imageSize().height, 1);
    framebuffer.addAttachment(image_view);
    framebuffers_.emplace_back(framebuffer);
  }

  return true;
}

const LogicalDevice *App::logicalDevice() {
  if (!logical_device_)
    createLogicalDevice();
  return logical_device_.get();
}

Swapchain *App::swapchain() {
  if (!swapchain_)
    setupSwapChain();
  return swapchain_.get();
}

GraphicsPipeline *App::graphicsPipeline() {
  if (!pipeline_)
    pipeline_ = std::make_unique<GraphicsPipeline>(
        logicalDevice(), pipelineLayout(), renderpass(), 0);
  return pipeline_.get();
}

PipelineLayout *App::pipelineLayout() {
  if (!pipeline_layout_)
    pipeline_layout_ = std::make_unique<PipelineLayout>(logicalDevice());
  return pipeline_layout_.get();
}

RenderPass *App::renderpass() {
  if (!renderpass_)
    renderpass_ = std::make_unique<RenderPass>(logicalDevice());
  return renderpass_.get();
}

const std::vector<Image::View> &App::swapchainImageViews() {
  if (!swapchain_)
    setupSwapChain();
  return swapchain_image_views_;
}

std::vector<CommandBuffer> &App::commandBuffers() {
  if (!command_buffers_.size()) {
    swapchain();
    if (!framebuffers_.size())
      exit();
    command_pool_->allocateCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                          framebuffers_.size(),
                                          command_buffers_);
  }
  return command_buffers_;
}

std::vector<Framebuffer> &App::framebuffers() {
  swapchain();
  return framebuffers_;
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
    graphics_display_->framebufferSize();
  } else {
    size_of_images = surface_capabilities.currentExtent;
  }
  return true;
}

void App::destroySwapchain() {
  // when a swapchain is not valid adequate anymore we need to recreate the
  // swapchain with new parameters. For that, we need to destroy the old
  // swapchain and the objects related to the swapchain, to recreate them later
  // We must destroy the objects in the following order:
  // 1. framebuffers
  // 2. command buffers
  // 3. graphics pipeline
  // 4. pipeline layout
  // 5. renderpass
  // 6. swapchain image views
  // 7. swapchain
  framebuffers_.clear();
  command_pool_->freeCommandBuffers(command_buffers_);
  pipeline_->destroy();
  pipeline_layout_->destroy();
  renderpass_->destroy();
  swapchain_image_views_.clear();
  swapchain_->destroy();
}

void App::recreateSwapchain() {
  graphics_display_->waitForValidWindowSize();
  vkDeviceWaitIdle(logical_device_->handle());
  destroySwapchain();
  if (framebuffer_resized_ && resize_callback)
    resize_callback(graphics_display_->framebufferSize().width,
                    graphics_display_->framebufferSize().height);
  setupSwapChain();
  commandBuffers();
  if (record_command_buffer_callback)
    for (size_t i = 0; i < framebuffers_.size(); ++i)
      record_command_buffer_callback(command_buffers_[i], framebuffers_[i]);
}

void App::draw() {
  static size_t current_frame = 0;
  in_flight_fences_[current_frame].wait();
  uint32_t image_index = 0;
  auto next_image_result =
      swapchain_->nextImage(image_available_semaphores_[current_frame].handle(),
                            VK_NULL_HANDLE, image_index);
  if (next_image_result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapchain();
    return;
  } else if (next_image_result != VK_SUCCESS &&
             next_image_result != VK_SUBOPTIMAL_KHR) {
    INFO("error on getting next swapchain image!");
    return;
  }
  if (images_in_flight_[image_index] != VK_NULL_HANDLE)
    vkWaitForFences(logical_device_->handle(), 1,
                    &images_in_flight_[image_index], VK_TRUE, UINT64_MAX);
  images_in_flight_[image_index] = in_flight_fences_[current_frame].handle();

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkCommandBuffer command_buffer = command_buffers_[image_index].handle();
  VkSemaphore waitSemaphores[] = {
      image_available_semaphores_[current_frame].handle()};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &command_buffer;

  VkSemaphore signalSemaphores[] = {
      render_finished_semaphores_[current_frame].handle()};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  in_flight_fences_[current_frame].reset();

  VkResult result =
      vkQueueSubmit(queue_families_.family("graphics").vk_queues[0], 1,
                    &submitInfo, in_flight_fences_[current_frame].handle());
  if (VK_SUCCESS != result)
    std::cerr << "osp\n";

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapchain_->handle()};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = &image_index;

  result = vkQueuePresentKHR(
      queue_families_.family("presentation").vk_queues[0], &presentInfo);

  if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR ||
      framebuffer_resized_) {
    recreateSwapchain();
    framebuffer_resized_ = false;
  } else if (next_image_result != VK_SUCCESS) {
    INFO("error on presenting swapchain image!");
    return;
  }

  vkQueueWaitIdle(queue_families_.family("presentation").vk_queues[0]);

  current_frame = (current_frame + 1) % max_frames_in_flight;
} // namespace vk

} // namespace vk

} // namespace circe