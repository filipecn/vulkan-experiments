/// Copyright (c) 2020, FilipeCN.
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
///\file example_base.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2020-04-28
///
///\brief

#ifndef EXAMPLE_BASE_H
#define EXAMPLE_BASE_H

#include <core/vk_app.h>
#include <ponos/common/defs.h>
#include <chrono>

class ExampleBase {
public:
  template<typename... Args> explicit ExampleBase(Args &&... args) {
    // setup app
    // The app represents the window in which we display our graphics
    app_ = std::make_unique<circe::vk::App>(std::forward<Args>(args)...);
    app_->setValidationLayers({"VK_LAYER_KHRONOS_validation"});
    // In order to setup the window we first need to connect to the vulkan
    // library. Here we could pass a list of vulkan instance extensions needed by
    // the application. The App automatically handles the basic extensions
    // required by the glfw library, so we don't need any extra extension.
    // app.setInstance(...);
    // A important step is to choose the hardware we want our application to use.
    // The pickPhysicalDevice gives us the chance to analyse the available
    // hardware and to pick the one that suits best to our needs. This is done by
    // checking the available vulkan queue families that present the features we
    // need, in this example we need just a queue with graphics and
    // presentation capabilities. The presentation capabilities is already checked
    // automatically, so we just need to check graphics.
    // app.pickPhysicalDevice([&](const circe::vk::PhysicalDevice &d,
    //                           circe::vk::QueueFamilies &q) -> uint32_t {
    //  std::cerr << d;
    //  if (d.properties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    //    return 1000;
    //  return 1;
    //});
    // After picking the hardware, we can create its digital representation. As
    // creating the vulkan instance, here we can also choose extra logical device
    // extensions our application need. The swap chain extension is added
    // automatically, so we need no extra extension.
    // ASSERT(app.createLogicalDevice());
    // The swapchain is the mechanism responsible for representing images in our
    // display, here we also need to configure it by choosing image format and
    // color space.
    // ASSERT(app.render_engine.setupSwapChain(VK_FORMAT_R8G8B8A8_UNORM,
    //                          VK_COLOR_SPACE_SRGB_NONLINEAR_KHR));
    // retrieve queue for buffer upload operations
    graphics_queue_family_index_ =
        app_->queueFamilies().family("graphics").family_index.value();
    graphics_queue_ = app_->queueFamilies().family("graphics").vk_queues[0];
    // init render pass object
    renderpass_ = std::make_unique<RenderPass>(app_->logicalDevice());
    // swapchain callbacks
    app_->render_engine.destroy_swapchain_callback = [&]() {
      color_image_view_.reset();
      color_image_.reset();
      color_image_memory_.reset();
      depth_image_view_.reset();
      depth_image_.reset();
      depth_image_memory_.reset();
      framebuffers_.clear();
    };
    app_->render_engine.create_swapchain_callback = [&]() { this->setupFramebuffers(); };
    // compute depth format
    app_->physicalDevice()->findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
         VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
        depth_format_);
    // setup anti-aliasing
    msaa_samples_ = app_->physicalDevice()->maxUsableSampleCount();
    // update uniform buffer callback
    app_->render_engine.prepare_frame_callback = [&](uint32_t index) {
      prepareFrameImage(index);
    };
  }
  virtual ~ExampleBase();
  void run();
  /// Render and compute fps
  void nextFrame();
  virtual void render() = 0;
  virtual void prepareFrameImage(uint32_t index) = 0;

  ///  Last frame time measured using a high performance timer (if available)
  float frame_timer = 1.0f;

protected:
  /// Prepare resources required by the example
  virtual void prepare();
  /// Setup renderpass for framebuffer writes
  virtual void prepareRenderpass();
  // Setup frame buffer objects (same count of swapchain images)
  virtual void setupFramebuffers();

  // Anti-Aliasing
  VkSampleCountFlagBits msaa_samples_{VK_SAMPLE_COUNT_1_BIT};
  // color buffer
  std::unique_ptr<circe::vk::Image> color_image_;
  std::unique_ptr<circe::vk::Image::View> color_image_view_;
  std::unique_ptr<circe::vk::DeviceMemory> color_image_memory_;
  // depth buffer
  VkFormat depth_format_;
  std::unique_ptr<circe::vk::Image> depth_image_;
  std::unique_ptr<circe::vk::Image::View> depth_image_view_;
  std::unique_ptr<circe::vk::DeviceMemory> depth_image_memory_;
  // app
  std::unique_ptr<circe::vk::App> app_; //!< window display
  VkQueue graphics_queue_{nullptr}; //!< device queue
  u32 graphics_queue_family_index_{0}; //!< device queue family index
  std::unique_ptr<circe::vk::RenderPass> renderpass_; //!< renderpass for framebuffer writes
  std::vector<circe::vk::Framebuffer> framebuffers_; //!< available frame buffers

  // Frame counter to display fps
  uint32_t frame_counter_ = 0;
  uint32_t last_FPS_ = 0;
  std::chrono::time_point<std::chrono::high_resolution_clock> last_timestamp_;
};

#endif //EXAMPLE_BASE_H