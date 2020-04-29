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

void ExampleBase::prepareRenderpass() {
  auto *renderpass = app_->render_engine.renderpass();
  auto &subpass_desc = renderpass->newSubpassDescription();
  { // COLOR ATTACHMENT
    renderpass->addAttachment(
        app_->render_engine.swapchain()->surfaceFormat().format,
        app_->render_engine.msaaSamples(), VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
        // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    subpass_desc.addColorAttachmentRef(
        0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    renderpass->addSubpassDependency(
        VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
        // VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
  }
  { // DEPTH ATTACHMENT
    renderpass->addAttachment(
        app_->render_engine.depthFormat(), app_->render_engine.msaaSamples(),
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    subpass_desc.setDepthStencilAttachmentRef(
        1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  }
  // since we are using multi-sampling, the first color attachment is cannot be
  // presented directly, first we need to resolve it into a proper image
  { // COLOR RESOLVE ATTACHMENT RESOLVE
    renderpass->addAttachment(
        app_->render_engine.swapchain()->surfaceFormat().format,
        VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    subpass_desc.addResolveAttachmentRef(
        2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  }
}

void ExampleBase::preparePipeline() {
  auto &pipeline = *app_->render_engine.graphicsPipeline();
  pipeline.setInputState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  pipeline.viewport_state.addViewport(0, 0, 800, 800, 0.f, 1.f);
  pipeline.viewport_state.addScissor(0, 0, 800, 800);
  pipeline.setRasterizationState(
      VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT,
      VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.f, 0.f, 0.f, 1.0f);
  pipeline.setMultisampleState(app_->render_engine.msaaSamples(), VK_FALSE,
                               1.f, std::vector<VkSampleMask>(), VK_FALSE,
                               VK_FALSE);
  pipeline.color_blend_state.addAttachmentState(
      VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
      VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
  pipeline.setDepthStencilState(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS,
                                VK_FALSE, VK_FALSE, {}, {}, 0.0, 1.0);
  // pipeline.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
  // pipeline.addDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
}
