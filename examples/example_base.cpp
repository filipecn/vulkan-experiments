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
///\file example_base.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2020-04-28
///
///\brief

#include "example_base.h"

using namespace circe::vk;

ExampleBase::~ExampleBase() = default;

void ExampleBase::run() {
  app_->run([&]() { this->nextFrame(); });
}

void ExampleBase::nextFrame() {
  // start frame time
  auto t_start = std::chrono::high_resolution_clock::now();
  // update scene
  render();
  frame_counter_++;
  auto t_end = std::chrono::high_resolution_clock::now();
  auto t_diff = std::chrono::duration<double, std::milli>(t_end - t_start).count();
  frame_timer = (float) t_diff / 1000.0f;
  // Convert to clamped timer value
  float fps_timer = (float) (std::chrono::duration<double, std::milli>(t_end - last_timestamp_).count());
  if (fps_timer > 1000.0f) {
    last_FPS_ = static_cast<uint32_t>((float) frame_counter_ * (1000.0f / fps_timer));
    frame_counter_ = 0;
    last_timestamp_ = t_end;
  }
}

void ExampleBase::prepare() {
  prepareRenderpass();
  setupFramebuffers();
}

void ExampleBase::prepareRenderpass() {
  auto &subpass_desc = renderpass_->newSubpassDescription();
  { // COLOR ATTACHMENT
    renderpass_->addAttachment(
        app_->render_engine.swapchain()->surfaceFormat().format,
        msaa_samples_, VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
        // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    subpass_desc.addColorAttachmentRef(
        0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    renderpass_->addSubpassDependency(
        VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
        // VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
  }
  { // DEPTH ATTACHMENT
    renderpass_->addAttachment(
        depth_format_, msaa_samples_,
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    subpass_desc.setDepthStencilAttachmentRef(
        1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  }
  // since we are using multi-sampling, the first color attachment cannot be
  // presented directly, first we need to resolve it into a proper image
  { // COLOR RESOLVE ATTACHMENT RESOLVE
    renderpass_->addAttachment(
        app_->render_engine.swapchain()->surfaceFormat().format,
        VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    subpass_desc.addResolveAttachmentRef(
        2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  }
}

void ExampleBase::setupFramebuffers() {
  auto swapchain = app_->render_engine.swapchain();
  // COLOR RESOURCES (anti-aliasing)
  color_image_.reset(new Image(
      app_->logicalDevice(), VK_IMAGE_TYPE_2D, app_->render_engine.swapchainSurfaceFormat().format,
      {swapchain->imageSize().width, swapchain->imageSize().height, 1}, 1, 1,
      msaa_samples_,
      VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      false));
  color_image_memory_ = std::make_unique<DeviceMemory>(
      *color_image_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  color_image_memory_->bind(*color_image_);
  color_image_view_.reset(new Image::View(color_image_.get(),
                                          VK_IMAGE_VIEW_TYPE_2D, app_->render_engine.swapchainSurfaceFormat().format,
                                          VK_IMAGE_ASPECT_COLOR_BIT));
  // DEPTH BUFFER
  depth_image_.reset(new Image(
      app_->logicalDevice(), VK_IMAGE_TYPE_2D, depth_format_,
      {swapchain->imageSize().width, swapchain->imageSize().height, 1}, 1, 1,
      msaa_samples_, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, false));
  depth_image_memory_ = std::make_unique<DeviceMemory>(
      *depth_image_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  depth_image_memory_->bind(*depth_image_);
  depth_image_view_ =
      std::make_unique<Image::View>(depth_image_.get(), VK_IMAGE_VIEW_TYPE_2D,
                                    depth_format_, VK_IMAGE_ASPECT_DEPTH_BIT);
  // setup framebuffers
  auto &swapchain_image_views = app_->render_engine.swapchainImageViews();
  for (auto &image_view : swapchain_image_views) {
    circe::vk::Framebuffer framebuffer(app_->logicalDevice(), renderpass_.get(),
                                       swapchain->imageSize().width,
                                       swapchain->imageSize().height, 1);
    // this order must be the same as the renderpass attachments
    framebuffer.addAttachment(*color_image_view_);
    framebuffer.addAttachment(*depth_image_view_);
    framebuffer.addAttachment(image_view);
    framebuffers_.emplace_back(framebuffer);
  }
}

