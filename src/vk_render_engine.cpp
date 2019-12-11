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

bool RenderEngine::selectNumberOfSwapchainImages(VkSurfaceCapabilitiesKHR const &surface_capabilities,
                                                 uint32_t &number_of_images) {
  number_of_images = surface_capabilities.minImageCount + 1;
  if ((surface_capabilities.maxImageCount > 0) &&
      (number_of_images > surface_capabilities.maxImageCount)) {
    number_of_images = surface_capabilities.maxImageCount;
  }
  return true;
}

bool RenderEngine::chooseSizeOfSwapchainImages(VkSurfaceCapabilitiesKHR const &surface_capabilities,
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
  uniform_buffers_.clear();
  uniform_buffer_memories_.clear();
  descriptor_pool_.reset();
  framebuffers_.clear();
  command_pool_->freeCommandBuffers(command_buffers_);
  pipeline_->destroy();
  pipeline_layout_.reset();
  renderpass_->destroy();
  swapchain_image_views_.clear();
  swapchain_->destroy();
}

void RenderEngine::recreateSwapchain() {
  // graphics_display_->waitForValidWindowSize();
  vkDeviceWaitIdle(logical_device_->handle());
  destroySwapchain();
  if (framebuffer_resized_ && resize_callback)
    resize_callback(width_,
                    height_);
  setupSwapChain();
  graphicsPipeline()->setLayout(pipeline_layout_.get());
  commandBuffers();
  if (record_command_buffer_callback)
    for (size_t i = 0; i < framebuffers_.size(); ++i)
      record_command_buffer_callback(command_buffers_[i], framebuffers_[i],
                                     !descriptor_sets_.empty()
                                     ? descriptor_sets_[i] : VK_NULL_HANDLE);
}

RenderEngine::RenderEngine() = default;

RenderEngine::RenderEngine(
    const PhysicalDevice *physical_device,
    const LogicalDevice *logical_device,
    uint32_t queue_family_index) {
  setDeviceInfo(physical_device, logical_device, queue_family_index);
}

RenderEngine::~RenderEngine() = default;

void RenderEngine::setDeviceInfo(const PhysicalDevice *physical_device,
                                 const LogicalDevice *logical_device,
                                 uint32_t queue_family_index) {
  physical_device_ = physical_device;
  logical_device_ = logical_device;
  command_pool_ = std::make_unique<CommandPool>(
      logical_device_, 0,
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
        logical_device_, vk_surface_, number_of_images, surface_format,
        swap_chain_image_size, image_usage, surface_transform, present_mode);
  else
    swapchain_->set(vk_surface_, number_of_images, surface_format,
                    swap_chain_image_size, image_usage, surface_transform,
                    present_mode);
  // CREATE IMAGE VIEWS
  const auto &swap_chain_images = swapchain_->images();
  for (const auto &swap_chain_image : swap_chain_images)
    swapchain_image_views_.emplace_back(&swap_chain_image,
                                        VK_IMAGE_VIEW_TYPE_2D, image_format,
                                        VK_IMAGE_ASPECT_COLOR_BIT);
  // setup framebuffers
  for (auto &image_view : swapchain_image_views_) {
    circe::vk::Framebuffer framebuffer(logical_device_, renderpass(),
                                       swapchain_->imageSize().width,
                                       swapchain_->imageSize().height, 1);
    framebuffer.addAttachment(image_view);
    framebuffers_.emplace_back(framebuffer);
  }
  // setup uniform buffers (if any)
  if (uniform_buffer_size_callback)
    for (size_t i = 0; i < swapchain_image_views_.size(); ++i) {
      uniform_buffers_.emplace_back(logical_device_,
                                    uniform_buffer_size_callback(),
                                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      uniform_buffer_memories_.emplace_back(logical_device_,
                                            uniform_buffers_[i],
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                                | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      uniform_buffer_memories_[i].bind(uniform_buffers_[i]);
    }

  // setup descriptor pool
  if (!descriptor_pool_)
    descriptor_pool_ = std::make_unique<DescriptorPool>(logical_device_,
                                                        swapchain_image_views_.size());
  descriptor_pool_->setPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                swapchain_image_views_.size());
  descriptor_pool_->setPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                swapchain_image_views_.size());
  if (descriptor_set_layout_callback) {
    for (size_t i = 0; i < swapchain_image_views_.size(); ++i)
      descriptor_set_layout_callback(pipelineLayout()->descriptorSetLayout(
          pipeline_layout_->createLayoutSet(i)));
  }
  // setup descriptor sets
  descriptor_pool_->allocate(pipelineLayout()->descriptorSetLayouts(),
                             descriptor_sets_);
  if (update_descriptor_set_callback)
    for (size_t i = 0; i < swapchain_image_views_.size(); ++i)
      update_descriptor_set_callback(descriptor_sets_[i],
                                     uniform_buffers_[i].handle());
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
  command_pool_.reset();
}

void RenderEngine::setSurface(VkSurfaceKHR surface) {
  vk_surface_ = surface;
}

Swapchain *RenderEngine::swapchain() {
  if (!swapchain_)
    setupSwapChain();
  return swapchain_.get();
}

GraphicsPipeline *RenderEngine::graphicsPipeline() {
  if (!pipeline_)
    pipeline_ = std::make_unique<GraphicsPipeline>(
        logical_device_, pipelineLayout(), renderpass(), 0);
  return pipeline_.get();
}

PipelineLayout *RenderEngine::pipelineLayout() {
  if (!pipeline_layout_)
    pipeline_layout_ = std::make_unique<PipelineLayout>(logical_device_);
  return pipeline_layout_.get();
}

RenderPass *RenderEngine::renderpass() {
  if (!renderpass_)
    renderpass_ = std::make_unique<RenderPass>(logical_device_);
  return renderpass_.get();
}

const std::vector<Image::View> &RenderEngine::swapchainImageViews() {
  if (!swapchain_)
    setupSwapChain();
  return swapchain_image_views_;
}

std::vector<CommandBuffer> &RenderEngine::commandBuffers() {
  if (command_buffers_.empty()) {
    swapchain();
    if (framebuffers_.empty())
      exit(-1);
    command_pool_->allocateCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                          framebuffers_.size(),
                                          command_buffers_);
  }
  return command_buffers_;
}

std::vector<Framebuffer> &RenderEngine::framebuffers() {
  swapchain();
  return framebuffers_;
}

void RenderEngine::init() {
  recreateSwapchain();
  images_in_flight_.resize(framebuffers_.size(), VK_NULL_HANDLE);
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

  // update uniform buffer
  if (update_uniform_buffer_callback) {
    update_uniform_buffer_callback(uniform_buffers_[image_index]);
    uniform_buffer_memories_[image_index].copy(uniform_buffers_[image_index]);
  }

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
      vkQueueSubmit(graphics_queue, 1,
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
      presentation_queue, &presentInfo);

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

} // circe::vk namespace

