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
///\file vk_command_buffer.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-10-25
///
///\brief

#include "vk_command_buffer.h"
#include "logging.h"
#include "vulkan_debug.h"

namespace circe::vk {

RenderPassBeginInfo::RenderPassBeginInfo(RenderPass *renderpass,
                                         Framebuffer *framebuffer) {
  info_.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  info_.pNext = nullptr;
  info_.renderPass = renderpass->handle();
  info_.framebuffer = framebuffer->handle();
  info_.renderArea.offset.x = info_.renderArea.offset.y = 0;
  info_.renderArea.extent.width = framebuffer->width();
  info_.renderArea.extent.height = framebuffer->height();
  info_.clearValueCount = 0;
  info_.pClearValues = nullptr;
}

void RenderPassBeginInfo::setRenderArea(int32_t x, int32_t y, uint32_t width,
                                        uint32_t height) {
  info_.renderArea.offset.x = x;
  info_.renderArea.offset.y = y;
  info_.renderArea.extent.width = width;
  info_.renderArea.extent.height = height;
}

void RenderPassBeginInfo::addClearColorValuef(float r, float g, float b,
                                              float a) {
  VkClearValue v;
  v.color.float32[0] = r;
  v.color.float32[1] = g;
  v.color.float32[2] = b;
  v.color.float32[3] = a;
  clear_values_.emplace_back(v);
  info_.clearValueCount = clear_values_.size();
  info_.pClearValues = clear_values_.data();
}

void RenderPassBeginInfo::addClearColorValuei(int32_t r, int32_t g, int32_t b,
                                              int32_t a) {
  VkClearValue v;
  v.color.int32[0] = r;
  v.color.int32[1] = g;
  v.color.int32[2] = b;
  v.color.int32[3] = a;
  clear_values_.emplace_back(v);
  info_.clearValueCount = clear_values_.size();
  info_.pClearValues = clear_values_.data();
}

void RenderPassBeginInfo::addClearColorValueu(uint32_t r, uint32_t g,
                                              uint32_t b, uint32_t a) {
  VkClearValue v;
  v.color.uint32[0] = r;
  v.color.uint32[0] = g;
  v.color.uint32[0] = b;
  v.color.uint32[0] = a;
  clear_values_.emplace_back(v);
  info_.clearValueCount = clear_values_.size();
  info_.pClearValues = clear_values_.data();
}

void RenderPassBeginInfo::addClearDepthStencilValue(float depth,
                                                    uint32_t stencil) {
  VkClearValue v;
  v.depthStencil.depth = depth;
  v.depthStencil.stencil = stencil;
  clear_values_.emplace_back(v);
  info_.clearValueCount = clear_values_.size();
  info_.pClearValues = clear_values_.data();
}

const VkRenderPassBeginInfo *RenderPassBeginInfo::info() const {
  return &info_;
}

CommandBuffer::CommandBuffer(VkCommandBuffer vk_command_buffer_)
    : vk_command_buffer_(vk_command_buffer_) {}

VkCommandBuffer CommandBuffer::handle() const { return vk_command_buffer_; }

bool CommandBuffer::begin(VkCommandBufferUsageFlags flags) const {
  VkCommandBufferBeginInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                   nullptr, flags, nullptr};
  R_CHECK_VULKAN(vkBeginCommandBuffer(vk_command_buffer_, &info))
  return true;
}

bool CommandBuffer::end() const {
  R_CHECK_VULKAN(vkEndCommandBuffer(vk_command_buffer_))
  return true;
}

bool CommandBuffer::reset(VkCommandBufferResetFlags flags) const {
  R_CHECK_VULKAN(vkResetCommandBuffer(vk_command_buffer_, flags))
  return true;
}

bool CommandBuffer::submit(VkQueue queue, VkFence fence) const {
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &vk_command_buffer_;
  vkQueueSubmit(queue, 1, &submit_info, fence);
  vkQueueWaitIdle(queue);
  return true;
}

void CommandBuffer::copy(const Buffer &src_buffer, VkDeviceSize src_offset,
                         const Buffer &dst_buffer, VkDeviceSize dst_offset,
                         VkDeviceSize size) const {
  const VkBufferCopy copy_region = {src_offset, dst_offset, size};
  vkCmdCopyBuffer(vk_command_buffer_, src_buffer.handle(), dst_buffer.handle(),
                  1, &copy_region);
}

void CommandBuffer::copy(const Image &src_image, VkImageLayout layout,
                         Buffer &dst_buffer,
                         const std::vector<VkBufferImageCopy> &regions) const {
  vkCmdCopyImageToBuffer(vk_command_buffer_, src_image.handle(), layout,
                         dst_buffer.handle(), regions.size(), regions.data());
}

void CommandBuffer::copy(const Buffer &src_buffer, Image &dst_image,
                         VkImageLayout layout,
                         const std::vector<VkBufferImageCopy> &regions) const {
  vkCmdCopyBufferToImage(vk_command_buffer_, src_buffer.handle(),
                         dst_image.handle(), layout, regions.size(),
                         regions.data());
}

void CommandBuffer::copy(const Image &src_image, VkImageLayout src_layout,
                         Image &dst_image, VkImageLayout dst_layout,
                         const std::vector<VkImageCopy> &regions) const {
  vkCmdCopyImage(vk_command_buffer_, src_image.handle(), src_layout,
                 dst_image.handle(), dst_layout, regions.size(),
                 regions.data());
}

void CommandBuffer::clear(const Image &image, VkImageLayout layout,
                          const std::vector<VkImageSubresourceRange> &ranges,
                          const VkClearColorValue &color) const {
  vkCmdClearColorImage(vk_command_buffer_, image.handle(), layout, &color,
                       ranges.size(), ranges.data());
}

void CommandBuffer::clear(const Image &image, VkImageLayout layout,
                          const std::vector<VkImageSubresourceRange> &ranges,
                          const VkClearDepthStencilValue &value) const {
  vkCmdClearDepthStencilImage(vk_command_buffer_, image.handle(), layout,
                              &value, ranges.size(), ranges.data());
}

void CommandBuffer::bind(const ComputePipeline &compute_pipeline) const {
  vkCmdBindPipeline(vk_command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE,
                    compute_pipeline.handle());
}

void CommandBuffer::bind(GraphicsPipeline *graphics_pipeline) const {
  vkCmdBindPipeline(vk_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    graphics_pipeline->handle());
}

void CommandBuffer::bind(VkPipelineBindPoint pipeline_bind_point,
                         PipelineLayout *layout, uint32_t first_set,
                         const std::vector<VkDescriptorSet> &descriptor_sets,
                         const std::vector<uint32_t> &dynamic_offsets) {
  vkCmdBindDescriptorSets(
      vk_command_buffer_, pipeline_bind_point, layout->handle(), first_set,
      descriptor_sets.size(), descriptor_sets.data(), dynamic_offsets.size(),
      (dynamic_offsets.size()) ? dynamic_offsets.data() : nullptr);
}

void CommandBuffer::bind(VkPipelineBindPoint pipeline_bind_point,
                         PipelineLayout &pipeline_layout,
                         const std::vector<VkDescriptorSet> &descriptor_sets,
                         const std::vector<uint32_t> &dynamic_offsets,
                         uint32_t first_set,
                         uint32_t descriptor_set_count) const {
  if (!descriptor_set_count)
    descriptor_set_count = descriptor_sets.size() - first_set;
  vkCmdBindDescriptorSets(vk_command_buffer_, pipeline_bind_point,
                          pipeline_layout.handle(), first_set,
                          descriptor_set_count, descriptor_sets.data(),
                          dynamic_offsets.size(), dynamic_offsets.data());
}

void CommandBuffer::dispatch(uint32_t x, uint32_t y, uint32_t z) const {
  vkCmdDispatch(vk_command_buffer_, x, y, z);
}

void CommandBuffer::dispatch(const Buffer &buffer, VkDeviceSize offset) const {
  vkCmdDispatchIndirect(vk_command_buffer_, buffer.handle(), offset);
}

void CommandBuffer::pushConstants(PipelineLayout &pipeline_layout,
                                  VkShaderStageFlags stage_flags,
                                  uint32_t offset, uint32_t size,
                                  const void *values) const {
  vkCmdPushConstants(vk_command_buffer_, pipeline_layout.handle(), stage_flags,
                     offset, size, values);
}

void CommandBuffer::beginRenderPass(const RenderPassBeginInfo &info,
                                    VkSubpassContents contents) const {
  vkCmdBeginRenderPass(vk_command_buffer_, info.info(), contents);
}

void CommandBuffer::endRenderPass() const {
  vkCmdEndRenderPass(vk_command_buffer_);
}

void CommandBuffer::bindVertexBuffers(
    uint32_t first_binding, const std::vector<VkBuffer> &buffers,
    const std::vector<VkDeviceSize> &offsets) const {
  vkCmdBindVertexBuffers(vk_command_buffer_, first_binding, buffers.size(),
                         buffers.data(), offsets.data());
}

void CommandBuffer::bindIndexBuffer(const Buffer &buffer, VkDeviceSize offset,
                                    VkIndexType type) const {
  vkCmdBindIndexBuffer(vk_command_buffer_, buffer.handle(), offset, type);
}

void CommandBuffer::draw(uint32_t vertex_count, uint32_t instance_count,
                         uint32_t first_vertex, uint32_t first_instance) const {
  vkCmdDraw(vk_command_buffer_, vertex_count, instance_count, first_vertex,
            first_instance);
}

void CommandBuffer::drawIndexed(uint32_t index_count, uint32_t instance_count,
                                uint32_t first_index, int32_t vertex_offset,
                                uint32_t first_instance) const {
  vkCmdDrawIndexed(vk_command_buffer_, index_count, instance_count, first_index,
                   vertex_offset, first_instance);
}

void CommandBuffer::transitionImageLayout(
    const ImageMemoryBarrier &barrier, VkPipelineStageFlags src_stages,
    VkPipelineStageFlags dst_stages) const {
  auto barrier_handle = barrier.handle();
  vkCmdPipelineBarrier(vk_command_buffer_, src_stages, dst_stages, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier_handle);
}

void CommandBuffer::blit(const Image &src_image, VkImageLayout src_image_layout,
                         const Image &dst_image, VkImageLayout dst_image_layout,
                         const std::vector<VkImageBlit> &regions,
                         VkFilter filter) const {
  vkCmdBlitImage(vk_command_buffer_, src_image.handle(), src_image_layout,
                 dst_image.handle(), dst_image_layout, regions.size(),
                 &regions[0], filter);
}

CommandPool::CommandPool(const LogicalDevice *logical_device,
                         VkCommandPoolCreateFlags parameters,
                         uint32_t queue_family)
    : logical_device_(logical_device) {
  VkCommandPoolCreateInfo command_pool_create_info = {
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, // VkStructureType sType
      nullptr,     // const void                 * pNext
      parameters,  // VkCommandPoolCreateFlags     flags
      queue_family // uint32_t                     queueFamilyIndex
  };
  CHECK_VULKAN(vkCreateCommandPool(logical_device->handle(),
                                   &command_pool_create_info, nullptr,
                                   &vk_command_pool_));
  if (vk_command_pool_ == VK_NULL_HANDLE)
    INFO("Could not create the command buffer.");
}

CommandPool::~CommandPool() {
  if (vk_command_pool_ != VK_NULL_HANDLE)
    vkDestroyCommandPool(logical_device_->handle(), vk_command_pool_, nullptr);
}

bool CommandPool::allocateCommandBuffers(
    VkCommandBufferLevel level, uint32_t count,
    std::vector<CommandBuffer> &command_buffers) const {
  VkCommandBufferAllocateInfo command_buffer_allocate_info = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, // VkStructureType sType
      nullptr,          // const void             * pNext
      vk_command_pool_, // VkCommandPool            commandPool
      level,            // VkCommandBufferLevel     level
      count             // uint32_t                 commandBufferCount
  };
  std::vector<VkCommandBuffer> vk_command_buffers(count);
  R_CHECK_VULKAN(vkAllocateCommandBuffers(logical_device_->handle(),
                                          &command_buffer_allocate_info,
                                          vk_command_buffers.data()));
  command_buffers.clear();
  for (auto cb : vk_command_buffers)
    command_buffers.emplace_back(cb);
  return true;
}

bool CommandPool::freeCommandBuffers(
    std::vector<CommandBuffer> &command_buffers) const {
  if (!command_buffers.size())
    return true;
  std::vector<VkCommandBuffer> vk_command_buffers(command_buffers.size());
  for (size_t i = 0; i < vk_command_buffers.size(); ++i)
    vk_command_buffers[i] = command_buffers[i].handle();
  vkFreeCommandBuffers(logical_device_->handle(), vk_command_pool_,
                       static_cast<uint32_t>(vk_command_buffers.size()),
                       vk_command_buffers.data());
  command_buffers.clear();
  return true;
}

bool CommandPool::reset(VkCommandPoolResetFlags flags) const {
  R_CHECK_VULKAN(
      vkResetCommandPool(logical_device_->handle(), vk_command_pool_, flags));
  return true;
}

void CommandPool::submitCommandBuffer(
    const LogicalDevice *logical_device, uint32_t family_index, VkQueue queue,
    const std::function<void(CommandBuffer &)> &record_callback) {
  circe::vk::CommandPool short_living_command_pool(
      logical_device, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, family_index);
  std::vector<circe::vk::CommandBuffer> short_living_command_buffers;
  short_living_command_pool.allocateCommandBuffers(
      VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, short_living_command_buffers);
  short_living_command_buffers[0].begin(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  record_callback(short_living_command_buffers[0]);
  short_living_command_buffers[0].end();
  short_living_command_buffers[0].submit(queue);
  short_living_command_pool.freeCommandBuffers(short_living_command_buffers);
}

} // namespace circe::vk