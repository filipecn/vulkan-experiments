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
/// \file vk_render_engine.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-12-06
///
/// \brief

#include "vk_render_engine.h"
#include "logging.h"

namespace circe::vk {

bool RenderEngine::selectNumberOfSwapchainImages(
    VkSurfaceCapabilitiesKHR const &surface_capabilities,
    uint32_t &number_of_images) {
  number_of_images = surface_capabilities.minImageCount + 1;
  if ((surface_capabilities.maxImageCount > 0) &&
      (number_of_images > surface_capabilities.maxImageCount)) {
    number_of_images = surface_capabilities.maxImageCount;
  }
  return true;
}

bool RenderEngine::chooseSizeOfSwapchainImages(
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

void RenderEngine::destroySwapchain() {
  // when a swapchain is not valid/adequate anymore we need to recreate the
  // swapchain with new parameters. For that, we need to destroy the old
  // swapchain and the objects related to the swapchain, to recreate them later
  // We must destroy the objects in the following order:
  // 1. color image (anti-aliasing resources)
  // 2. depth buffer
  // 3. framebuffers
  // 4. command buffers
  // 5. graphics pipeline
  // 6. pipeline layout
  // 7. renderpass
  // 8. swapchain image views
  // 9. swapchain
  if (destroy_swapchain_callback)
    destroy_swapchain_callback();
  draw_command_pool_->freeCommandBuffers(draw_command_buffers_);
  swapchain_image_views_.clear();
  swapchain_->destroy();
}

void RenderEngine::recreateSwapchain() {
  // graphics_display_->waitForValidWindowSize();
  vkDeviceWaitIdle(logical_device_->handle());
  destroySwapchain();
  setupSwapChain();
  if (framebuffer_resized_ && resize_callback)
    resize_callback(width_, height_);
  if (create_swapchain_callback)
    create_swapchain_callback();
  commandBuffers();
  if (record_command_buffer_callback)
    for (size_t i = 0; i < swapchain_image_views_.size(); ++i)
      record_command_buffer_callback(draw_command_buffers_[i], i);
}

RenderEngine::RenderEngine() = default;

RenderEngine::RenderEngine(const LogicalDevice *logical_device,
                           uint32_t queue_family_index) {
  setDeviceInfo(logical_device, queue_family_index);
}

RenderEngine::~RenderEngine() = default;

void RenderEngine::setDeviceInfo(const LogicalDevice *logical_device,
                                 uint32_t queue_family_index) {
  physical_device_ = logical_device->physicalDevice();
  logical_device_ = logical_device;
  draw_command_pool_ =
      std::make_unique<CommandPool>(logical_device_,
                                    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                    queue_family_index);
  // setup synchronization objects
  for (size_t i = 0; i < max_frames_in_flight; ++i) {
    in_flight_fences_.emplace_back(logical_device_,
                                   VK_FENCE_CREATE_SIGNALED_BIT);
    image_available_semaphores_.emplace_back(logical_device_);
    render_finished_semaphores_.emplace_back(logical_device_);
  }
}

bool RenderEngine::setupSwapChain(VkFormat desired_format,
                                  VkColorSpaceKHR desired_color_space) {
  // TODO: save parameters to private fields? Maybe I don't need to do all that again to recreate the swapchain...
  // PRESENTATION MODE
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
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
  VkSurfaceTransformFlagBitsKHR surface_transform = surface_capabilities.currentTransform;
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
  surface_format_ = {image_format, image_color_space};
  if (!swapchain_)
    swapchain_ = std::make_unique<Swapchain>(
        logical_device_, vk_surface_, number_of_images, surface_format_,
        swap_chain_image_size, image_usage, surface_transform, present_mode);
  else
    swapchain_->set(vk_surface_, number_of_images, surface_format_,
                    swap_chain_image_size, image_usage, surface_transform,
                    present_mode);
  // CREATE IMAGE VIEWS
  const auto &swap_chain_images = swapchain_->images();
  for (const auto &swap_chain_image : swap_chain_images)
    swapchain_image_views_.emplace_back(&swap_chain_image,
                                        VK_IMAGE_VIEW_TYPE_2D, image_format,
                                        VK_IMAGE_ASPECT_COLOR_BIT);
  return true;
}

void RenderEngine::resize(uint32_t width, uint32_t height) {
  framebuffer_resized_ = true;
  width_ = width;
  height_ = height;
}

void RenderEngine::destroy() {
  destroySwapchain();
  render_finished_semaphores_.clear();
  image_available_semaphores_.clear();
  in_flight_fences_.clear();
  images_in_flight_.clear();
  draw_command_pool_.reset();
}

void RenderEngine::setSurface(VkSurfaceKHR surface) { vk_surface_ = surface; }

Swapchain *RenderEngine::swapchain() {
  if (!swapchain_)
    if (!setupSwapChain()) {
      std::cerr << "Could not setup the swapchain!\n";
      exit(-1);
    }
  return swapchain_.get();
}

VkSurfaceFormatKHR RenderEngine::swapchainSurfaceFormat() const {
  return surface_format_;
}

const std::vector<Image::View> &RenderEngine::swapchainImageViews() {
  if (!swapchain_)
    setupSwapChain();
  return swapchain_image_views_;
}

std::vector<CommandBuffer> &RenderEngine::commandBuffers() {
  if (draw_command_buffers_.empty()) {
    swapchain();
    if (swapchain_image_views_.empty())
      exit(-1);
    draw_command_pool_->allocateCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                               swapchain_image_views_.size(),
                                               draw_command_buffers_);
  }
  return draw_command_buffers_;
}

void RenderEngine::init() {
  recreateSwapchain();
  images_in_flight_.resize(swapchain_image_views_.size(), VK_NULL_HANDLE);
}

void RenderEngine::draw(VkQueue graphics_queue, VkQueue presentation_queue) {
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

  if (prepare_frame_callback)
    prepare_frame_callback(image_index);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkCommandBuffer command_buffer = draw_command_buffers_[image_index].handle();
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

  VkResult result = vkQueueSubmit(graphics_queue, 1, &submitInfo,
                                  in_flight_fences_[current_frame].handle());
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

  result = vkQueuePresentKHR(presentation_queue, &presentInfo);

  if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR ||
      framebuffer_resized_) {
    recreateSwapchain();
    framebuffer_resized_ = false;
  } else if (next_image_result != VK_SUCCESS) {
    INFO("error on presenting swapchain image!")
    return;
  }

  vkQueueWaitIdle(presentation_queue);

  current_frame = (current_frame + 1) % max_frames_in_flight;
}

} // namespace circe::vk
