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
///\file vk_renderpass.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-11-06
///
///\brief

#include "vk_renderpass.h"
#include "vulkan_debug.h"

namespace circe {

namespace vk {

SubpassDescription::SubpassDescription()
    : depth_stencil_attachment_set_(false) {}

uint32_t SubpassDescription::addInputAttachmentRef(uint32_t attachment,
                                                   VkImageLayout layout) {
  VkAttachmentReference ar = {attachment, layout};
  vk_input_attachments_.emplace_back(ar);
  return vk_input_attachments_.size() - 1;
}

uint32_t SubpassDescription::addColorAttachmentRef(uint32_t attachment,
                                                   VkImageLayout layout) {
  VkAttachmentReference ar = {attachment, layout};
  vk_color_attachments_.emplace_back(ar);
  return vk_color_attachments_.size() - 1;
}

uint32_t
SubpassDescription::addResolveAttachmentRef(uint32_t resolve_attachment,
                                            VkImageLayout resolve_layout) {
  VkAttachmentReference rar = {resolve_attachment, resolve_layout};
  vk_resolve_attachments_.emplace_back(rar);
  return vk_resolve_attachments_.size() - 1;
}

void SubpassDescription::setDepthStencilAttachmentRef(uint32_t attachment,
                                                      VkImageLayout layout) {
  vk_depth_stencil_attachment_.attachment = attachment;
  vk_depth_stencil_attachment_.layout = layout;
  depth_stencil_attachment_set_ = true;
}

void SubpassDescription::preserveAttachment(uint32_t attachment) {
  preserve_attachments_.emplace_back(attachment);
}

const VkAttachmentReference &
SubpassDescription::depthStencilAttachmentRef() const {
  return vk_depth_stencil_attachment_;
}

const std::vector<VkAttachmentReference> &
SubpassDescription::inputAttachmentRefs() const {
  return vk_input_attachments_;
}

const std::vector<VkAttachmentReference> &
SubpassDescription::colorAttachmentRefs() const {
  return vk_color_attachments_;
}

const std::vector<VkAttachmentReference> &
SubpassDescription::resolveAttachmentRefs() const {
  return vk_resolve_attachments_;
}

const std::vector<uint32_t> &
SubpassDescription::preserveAttachmentRefs() const {
  return preserve_attachments_;
}

bool SubpassDescription::hasDepthStencilAttachmentRef() const {
  return depth_stencil_attachment_set_;
}

RenderPass::RenderPass(const LogicalDevice *logical_device)
    : logical_device_(logical_device) {}

RenderPass::~RenderPass() { destroy(); }

void RenderPass::destroy() {
  if (vk_renderpass_ != VK_NULL_HANDLE) {
    vkDestroyRenderPass(logical_device_->handle(), vk_renderpass_, nullptr);
    vk_renderpass_ = VK_NULL_HANDLE;
  }
}

uint32_t RenderPass::addAttachment(
    VkFormat format, VkSampleCountFlagBits samples, VkAttachmentLoadOp load_op,
    VkAttachmentStoreOp store_op, VkAttachmentLoadOp stencil_load_op,
    VkAttachmentStoreOp stencil_store_op, VkImageLayout initial_layout,
    VkImageLayout final_layout) {
  VkAttachmentDescription d = {
      0,           format,          samples,          load_op,
      store_op,    stencil_load_op, stencil_store_op, initial_layout,
      final_layout};
  vk_attachments_.emplace_back(d);
  return vk_attachments_.size() - 1;
}

void RenderPass::addSubpassDependency(uint32_t src_subpass,
                                      uint32_t dst_subpass,
                                      VkPipelineStageFlags src_stage_mask,
                                      VkPipelineStageFlags dst_stage_mask,
                                      VkAccessFlags src_access,
                                      VkAccessFlags dst_access) {
  VkSubpassDependency sd = {src_subpass,
                            dst_subpass,
                            src_stage_mask,
                            dst_stage_mask,
                            src_access,
                            dst_access,
                            0};
  vk_subpass_dependencies_.emplace_back(sd);
}

SubpassDescription &RenderPass::newSubpassDescription(uint32_t *id) {
  if (id)
    *id = subpass_descriptions_.size();
  subpass_descriptions_.emplace_back();
  return subpass_descriptions_[subpass_descriptions_.size() - 1];
}

VkRenderPass RenderPass::handle() {
  if (vk_renderpass_ == VK_NULL_HANDLE) {
    std::vector<VkSubpassDescription> subpass_descriptions;
    for (auto &sd : subpass_descriptions_) {
      VkSubpassDescription info = {
          0,
          VK_PIPELINE_BIND_POINT_GRAPHICS,
          static_cast<uint32_t>(sd.inputAttachmentRefs().size()),
          (sd.inputAttachmentRefs().size() ? sd.inputAttachmentRefs().data()
                                           : nullptr),
          static_cast<uint32_t>(sd.colorAttachmentRefs().size()),
          (sd.colorAttachmentRefs().size() ? sd.colorAttachmentRefs().data()
                                           : nullptr),
          (sd.resolveAttachmentRefs().size() ? sd.resolveAttachmentRefs().data()
                                             : nullptr),
          (sd.hasDepthStencilAttachmentRef() ? &sd.depthStencilAttachmentRef()
                                             : nullptr),
          static_cast<uint32_t>(sd.preserveAttachmentRefs().size()),
          (sd.preserveAttachmentRefs().size()
               ? sd.preserveAttachmentRefs().data()
               : nullptr)};
      subpass_descriptions.emplace_back(info);
    }
    VkRenderPassCreateInfo info = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t>(vk_attachments_.size()),
        vk_attachments_.data(),
        static_cast<uint32_t>(subpass_descriptions.size()),
        subpass_descriptions.data(),
        static_cast<uint32_t>(vk_subpass_dependencies_.size()),
        vk_subpass_dependencies_.data()};
    VkResult result = vkCreateRenderPass(logical_device_->handle(), &info,
                                         nullptr, &vk_renderpass_);
    CHECK_VULKAN(result);
    if (result != VK_SUCCESS)
      vk_renderpass_ = VK_NULL_HANDLE;
  }
  return vk_renderpass_;
}

Framebuffer::Framebuffer(const LogicalDevice *logical_device,
                         RenderPass *renderpass, uint32_t width,
                         uint32_t height, uint32_t layers)
    : logical_device_(logical_device), width_(width), height_(height),
      layers_(layers), renderpass_(renderpass) {}

Framebuffer::Framebuffer(Framebuffer &other)
    : Framebuffer(other.logical_device_, other.renderpass_, other.width_,
                  other.height_, other.layers_) {
  attachments_ = std::move(other.attachments_);
  vk_framebuffer_ = other.vk_framebuffer_;
  other.vk_framebuffer_ = VK_NULL_HANDLE;
}

Framebuffer::Framebuffer(Framebuffer &&other) noexcept
    : Framebuffer(other.logical_device_, other.renderpass_, other.width_,
                  other.height_, other.layers_) {
  attachments_ = std::move(other.attachments_);
  vk_framebuffer_ = other.vk_framebuffer_;
  other.vk_framebuffer_ = VK_NULL_HANDLE;
}

Framebuffer::~Framebuffer() {
  if (vk_framebuffer_ != VK_NULL_HANDLE)
    vkDestroyFramebuffer(logical_device_->handle(), vk_framebuffer_, nullptr);
}

void Framebuffer::addAttachment(const Image::View &image_view) {
  attachments_.emplace_back(image_view.handle());
}

uint32_t Framebuffer::width() const { return width_; }

uint32_t Framebuffer::height() const { return height_; }

uint32_t Framebuffer::layers() const { return layers_; }

VkFramebuffer Framebuffer::handle() {
  if (vk_framebuffer_ == VK_NULL_HANDLE) {
    VkFramebufferCreateInfo info = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        nullptr,
        0,
        renderpass_->handle(),
        static_cast<uint32_t>(attachments_.size()),
        (attachments_.size() ? attachments_.data() : nullptr),
        width_,
        height_,
        layers_};
    VkResult result = vkCreateFramebuffer(logical_device_->handle(), &info,
                                          nullptr, &vk_framebuffer_);
    CHECK_VULKAN(result);
    if (result != VK_SUCCESS)
      vk_framebuffer_ = VK_NULL_HANDLE;
  }
  return vk_framebuffer_;
}

} // namespace vk

} // namespace circe