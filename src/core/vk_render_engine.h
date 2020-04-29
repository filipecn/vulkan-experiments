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
/// \file vk_render_engine.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-12-06
///
/// \brief

#ifndef CIRCE_VK_RENDER_ENGINE_H
#define CIRCE_VK_RENDER_ENGINE_H

#include "vk_command_buffer.h"
#include "vk_device_memory.h"
#include "vk_image.h"
#include "vk_pipeline.h"
#include "vk_renderpass.h"
#include "vk_swap_chain.h"
#include "vk_sync.h"

namespace circe::vk {

class RenderEngine {
public:
  RenderEngine();
  /// \param logical_device
  /// \param queue_family_index
  explicit RenderEngine(const PhysicalDevice *physical_device,
                        const LogicalDevice *logical_device,
                        uint32_t queue_family_index);
  ~RenderEngine();
  ///
  /// \param physical_device
  /// \param logical_device
  void setDeviceInfo(const PhysicalDevice *physical_device,
                     const LogicalDevice *logical_device,
                     uint32_t queue_family_index);
  ///
  /// \param surface
  void setSurface(VkSurfaceKHR surface);
  /// Setups the swapchain structure, that is responsible for image presentation
  /// on screen. It is configured with image format, color space and other
  /// settings. If the swap chain is succefully created, the method retrieves
  /// the list of swap chain images.
  /// \param format **[in]** desired image format
  /// \param color_space **[in]** desired color space
  /// \return bool true if success
  bool setupSwapChain(
      VkFormat format = VK_FORMAT_B8G8R8A8_UNORM,
      VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
  void resize(uint32_t width, uint32_t height);
  void destroy();
  GraphicsPipeline *graphicsPipeline();
  PipelineLayout *pipelineLayout();
  RenderPass *renderpass();
  Swapchain *swapchain();
  DescriptorPool *descriptorPool();
  const std::vector<Image::View> &swapchainImageViews();
  std::vector<CommandBuffer> &commandBuffers();
  std::vector<Framebuffer> &framebuffers();
  VkFormat depthFormat();
  [[nodiscard]] VkSampleCountFlagBits msaaSamples() const;
  void init();
  void draw(VkQueue graphics_queue, VkQueue presentation_queue);

  std::function<void(uint32_t width, uint32_t height)> resize_callback;
  std::function<void(CommandBuffer &, Framebuffer &, VkDescriptorSet)>
      record_command_buffer_callback;
  std::function<uint32_t()> uniform_buffer_size_callback;
  std::function<void(DeviceMemory &)> update_uniform_buffer_callback;
  std::function<void(DescriptorSetLayout &)> descriptor_set_layout_callback;
  std::function<void(VkDescriptorSet, VkBuffer)> update_descriptor_set_callback;

private:
  // auxiliary methods for swapchain creation
  ///
  /// \param surface_capabilities
  /// \param number_of_images
  /// \return
  static bool selectNumberOfSwapchainImages(
      VkSurfaceCapabilitiesKHR const &surface_capabilities,
      uint32_t &number_of_images);
  ///
  /// \param surface_capabilities
  /// \param size_of_images
  /// \return
  static bool chooseSizeOfSwapchainImages(
      VkSurfaceCapabilitiesKHR const &surface_capabilities,
      VkExtent2D &size_of_images);
  void destroySwapchain();
  void recreateSwapchain();

  const PhysicalDevice *physical_device_ = nullptr;
  const LogicalDevice *logical_device_ = nullptr;
  const size_t max_frames_in_flight = 2;
  VkSurfaceKHR vk_surface_ = VK_NULL_HANDLE;
  // swapchain information
  std::unique_ptr<Swapchain> swapchain_;
  std::vector<Image::View> swapchain_image_views_;
  std::unique_ptr<RenderPass> renderpass_;
  std::unique_ptr<PipelineLayout> pipeline_layout_;
  std::unique_ptr<GraphicsPipeline> pipeline_;
  std::unique_ptr<CommandPool> command_pool_;
  std::vector<CommandBuffer> command_buffers_;
  std::vector<Framebuffer> framebuffers_;
  std::vector<Buffer> uniform_buffers_;
  std::vector<DeviceMemory> uniform_buffer_memories_;
  std::unique_ptr<DescriptorPool> descriptor_pool_;
  std::vector<VkDescriptorSet> descriptor_sets_;
  // Anti-Aliasing
  VkSampleCountFlagBits msaa_samples_{VK_SAMPLE_COUNT_1_BIT};
  std::unique_ptr<Image> color_image_;
  std::unique_ptr<Image::View> color_image_view_;
  std::unique_ptr<DeviceMemory> color_image_memory_;
  // depth buffer information
  VkFormat depth_format_;
  std::unique_ptr<Image> depth_image_;
  std::unique_ptr<Image::View> depth_image_view_;
  std::unique_ptr<DeviceMemory> depth_image_memory_;
  // synchronization
  std::vector<Semaphore> render_finished_semaphores_;
  std::vector<Semaphore> image_available_semaphores_;
  std::vector<Fence> in_flight_fences_;
  std::vector<VkFence> images_in_flight_;
  // resize info
  bool framebuffer_resized_ = false;
  uint32_t width_ = 0, height_ = 0;
};

} // namespace circe::vk

#endif