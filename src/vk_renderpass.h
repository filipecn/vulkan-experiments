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
///\file vk_renderpass.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-11-06
///
///\brief

#ifndef CIRCE_VULKAN_RENDERPASS_H
#define CIRCE_VULKAN_RENDERPASS_H

#include "vk_image.h"
#include "vulkan_logical_device.h"

namespace circe {

namespace vk {

/// A pass within the renderpass is called a subpass. Each subpass references a
/// number of attachments (from the order added on the RenderPass object) as
/// input and outputs.
class SubpassDescription {
public:
  SubpassDescription();
  ~SubpassDescription() = default;
  ///\brief Adds an input attachment reference
  /// Input attachments are attachments from which the subpass can read from.
  ///\param attachment **[in]** index into the array of attachments created in
  /// the RenderPass object
  ///\param layout **[in]** image layout that the attachment is expected to be
  /// in at this subpass
  ///\return uint32_t index of the added attachment reference
  uint32_t addInputAttachmentRef(uint32_t attachment, VkImageLayout layout);
  ///\brief Adds an color attachment reference
  /// Color attachments are attachments from which the subpass write to.
  /// Resolve attachments are attachments that correspond to color attachments,
  /// and are used to resove multisample image data
  ///\param attachment **[in]** index into the array of attachments created in
  /// the RenderPass object
  ///\param layout **[in]** image layout that the attachment is expected to be
  /// in at this subpass
  ///\return uint32_t index of the added attachment reference
  uint32_t addColorAttachmentRef(uint32_t attachment, VkImageLayout layout);
  ///\param resolve_attachment **[in]** index into the array of attachments
  /// created in the RenderPass object
  ///\param resolve_layout **[in]** image layout that the attachment is expected
  /// to be in at this subpass
  ///\return uint32_t index of the added attachment reference
  uint32_t addResolveAttachmentRef(uint32_t resolve_attachment,
                                   VkImageLayout resolve_layout);
  ///\brief Sets the depth-stencil attachment reference
  /// The depth/stencil attachment is the attachment used as a depth and stencil
  /// buffer.
  ///\param attachment **[in]** index into the array of attachments created in
  /// the RenderPass object
  ///\param layout **[in]** image layout that the attachment is expected to be
  /// in at this subpass
  void setDepthStencilAttachmentRef(uint32_t attachment, VkImageLayout layout);
  /// Preserved attachments live across the subpass and prevent Vulkan from
  /// making any optimizations that might disturb its contents.
  ///\param attachment **[in]** index into the array of attachments created in
  /// the RenderPass object
  void preserveAttachment(uint32_t attachment);
  const VkAttachmentReference &depthStencilAttachmentRef() const;
  const std::vector<VkAttachmentReference> &inputAttachmentRefs() const;
  const std::vector<VkAttachmentReference> &colorAttachmentRefs() const;
  const std::vector<VkAttachmentReference> &resolveAttachmentRefs() const;
  const std::vector<uint32_t> &preserveAttachmentRefs() const;
  bool hasDepthStencilAttachmentRef() const;

private:
  bool depth_stencil_attachment_set_ = false;
  VkAttachmentReference vk_depth_stencil_attachment_;
  std::vector<VkAttachmentReference> vk_input_attachments_;
  std::vector<VkAttachmentReference> vk_color_attachments_;
  std::vector<VkAttachmentReference> vk_resolve_attachments_;
  std::vector<uint32_t> preserve_attachments_;
};

/// A single renderpass object encapsulates multiple passes or rendering phases
/// over a single set of output images. Renderpass objects can contain multiple
/// subpasses.
/// Vulkan can figure out which subpasses are dependent on one another. However,
/// there are cases in which dependencies cannot be figured out automatically
/// (for example, when a subpass writes directly to a resource and a subsequent
/// subpass reads data back). Then dependencies must be explicitly defined.
class RenderPass {
public:
  RenderPass(const LogicalDevice *logical_device);
  ~RenderPass();
  ///\brief
  /// The load/store ops parameters specify what to do with the attachment at
  /// the beginning and end of the render pass
  ///\param format **[in]** format of the attachment (must match the format of
  /// the image attached)
  ///\param samples **[in]** indicates the number of samples used for
  /// multisampling (ex: VK_SAMPLE_COUNT_1_BIT)
  ///\param load_op **[in]**
  ///\param store_op **[in]**
  ///\param stencil_load_op **[in]**
  ///\param stencil_store_op **[in]**
  ///\param initial_layout **[in]** what layout to expect the image to be in
  /// when the renderpass begins
  ///\param final_layout **[in]** what layout to leave when the renderpass ends
  void addAttachement(VkFormat format, VkSampleCountFlagBits samples,
                      VkAttachmentLoadOp load_op, VkAttachmentStoreOp store_op,
                      VkAttachmentLoadOp stencil_load_op,
                      VkAttachmentStoreOp stencil_store_op,
                      VkImageLayout initial_layout, VkImageLayout final_layout);
  ///\brief Defines a dependency between subpasses.
  ///\param src_subpass **[in]** source index in the subpass array
  ///\param dst_subpass **[in]** destination index in the subpass array
  ///\param src_stage_mask **[in]** specifies which pipeline stages produced
  /// data on the source subpass
  ///\param dst_stage_mask **[in]** specifies which pipeline stages will consume
  /// the data in the destination subpass
  ///\param src_access **[in]** specifies how the source subpass access the data
  ///\param dst_access **[in]** specifies how the destination subpass access the
  /// data
  void addSubpassDependency(uint32_t src_subpass, uint32_t dst_subpass,
                            VkPipelineStageFlags src_stage_mask,
                            VkPipelineStageFlags dst_stage_mask,
                            VkAccessFlags src_access, VkAccessFlags dst_access);
  ///\param id **[out | optional]** receives the subpass index
  ///\return SubpassDescription&
  SubpassDescription &newSubpassDescription(uint32_t *id = nullptr);
  VkRenderPass handle();

private:
  const LogicalDevice *logical_device_ = nullptr;
  VkRenderPass vk_renderpass_ = CALLBACK_NULL;
  std::vector<VkAttachmentDescription> vk_attachments_;
  std::vector<VkSubpassDependency> vk_subpass_dependencies_;
  std::vector<SubpassDescription> subpass_descriptions_;
};

/// A framebuffer is an object that represents the set of images that graphics
/// pipelines render into. It is created by using a reference to a renderpass
/// and can be used with any renderpass that has a similar arrengement of
/// attachments.
class Framebuffer {
public:
  ///\param logical_device **[in]**
  ///\param renderpass **[in]** A compatible renderpass
  ///\param width **[in]**
  ///\param height **[in]**
  ///\param layers **[in]**
  Framebuffer(const LogicalDevice &logical_device, RenderPass &renderpass,
              uint32_t width, uint32_t height, uint32_t layers);
  ~Framebuffer();
  ///\brief Bounds an image into the framebuffer
  /// The passes comprising the renderpass make references to the image
  /// attachments, and those refrences are specified as indices of the array
  /// constructed from theses additions. In order to make the framebuffer
  /// compatible to a renderpass, you are allowed to add image views with
  /// a null handle (VkNullHandle).
  ///\param image_view **[in]**
  void addAttachment(const Image::View &image_view);
  VkFramebuffer handle();
  uint32_t width() const;
  uint32_t height() const;
  uint32_t layers() const;

private:
  const LogicalDevice &logical_device_;
  uint32_t width_, height_, layers_;
  RenderPass &renderpass_;
  VkFramebuffer vk_framebuffer_ = VK_NULL_HANDLE;
  std::vector<VkImageView> attachments_;
};

} // namespace vk

} // namespace circe

#endif // CIRCE_VULKAN_RENDERPASS_H