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
///\file vk_command_buffer.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-10-25
///
///\brief

#ifndef CIRCE_VULKAN_COMMAND_BUFFER_H
#define CIRCE_VULKAN_COMMAND_BUFFER_H

#include "vk_buffer.h"
#include "vk_image.h"
#include "vk_pipeline.h"
#include "vulkan_logical_device.h"

namespace circe {

namespace vk {

// Command buffers record operations and are submitted to the hardware. They
// can be recorded in multiple threads and also can be saved and reused.
// Synchronization is very important on this part, because the operations
// submitted need to be processed properly.
// Before allocating command buffers, we need to allocate command pools,
// from which the command buffers acquire memory. Command pools are also
// responsible for informing the driver on how to deal with the command
// buffers memory allocated from them (for example, whether the command
// buffer will have a short life or if they need to be reset or freed).
// Command pools also control the queues that receive the command buffers.
// Command buffers are separate in two groups (levels):
// 1. Primary - can be directly submitted to queues and call secondary
// command
//    buffers.
// 2. Secondary - can only be executed from primary command buffers.
// When recording commands to command buffers, we need to set the state for
// the operations as well (for example, vertex attributes use other buffers
// to work). When calling a secondary command buffer, the state of the
// caller primary command buffer is not preserved (unless it is a render
// pass).

class CommandBuffer {
public:
  ///\brief Construct a new Command Buffer object
  ///
  ///\param vk_command_buffer **[in]**
  CommandBuffer(VkCommandBuffer vk_command_buffer_);
  ~CommandBuffer() = default;
  ///\brief
  ///
  ///\param flags **[in]**
  ///\return bool
  bool begin(VkCommandBufferUsageFlags flags) const;
  ///\brief
  ///
  ///\return bool
  bool end() const;
  ///\brief
  ///
  ///\param flags **[in]**
  ///\return bool
  bool reset(VkCommandBufferResetFlags flags) const;
  ///\brief
  ///
  ///\param src_buffer **[in]**
  ///\param src_offset **[in]**
  ///\param dst_buffer **[in]**
  ///\param dst_offset **[in]**
  ///\param size **[in]**
  ///\return bool
  bool copy(const Buffer &src_buffer, VkDeviceSize src_offset,
            const Buffer &dst_buffer, VkDeviceSize dst_offset,
            VkDeviceSize size) const;
  ///\brief
  ///
  ///\param src_buffer **[in]**
  ///\param dst_image **[in/out]**
  ///\param layout **[in]** layout that the image is expected to be in when the
  /// copy command is executed. Accepted layouts are VK_IMAGE_LAYOUT_[GENERAL |
  /// TRANSFER_DST_OPTIMAL]. To clear images in different layouts you need to
  /// move them to one of these two layouts using a pipeline barrier before the
  /// clear command.
  ///\param regions **[in]**
  ///\return bool
  bool copy(const Buffer &src_buffer, Image &dst_image, VkImageLayout layout,
            const std::vector<VkBufferImageCopy> &regions) const;
  ///\brief
  ///
  ///\param src_image **[in]**
  ///\param dst_buffer **[in/out]**
  ///\param layout **[in]** layout that the image is expected to be in when the
  /// copy command is executed. Accepted layouts are VK_IMAGE_LAYOUT_[GENERAL |
  /// TRANSFER_DST_OPTIMAL]. To clear images in different layouts you need to
  /// move them to one of these two layouts using a pipeline barrier before the
  /// clear command.
  ///\param regions **[in]**
  ///\return bool
  bool copy(const Image &src_image, VkImageLayout layout, Buffer &dst_buffer,
            const std::vector<VkBufferImageCopy> &regions) const;
  ///\brief
  ///
  ///\param src_image **[in]**
  ///\param src_layout **[in]**
  ///\param dst_image **[in/out]**
  ///\param dst_layout **[in]**
  ///\param regions **[in]**
  ///\return bool
  bool copy(const Image &src_image, VkImageLayout src_layout, Image &dst_image,
            VkImageLayout dst_layout,
            const std::vector<VkImageCopy> &regions) const;

  ///\brief
  /// Fills a buffer with a fixed value.
  ///\tparam T
  ///\param buffer **[in]**
  ///\param data **[in]**
  ///\param offset **[in]** multiple of 4
  ///\param length **[in]**
  ///\return bool
  template <typename T>
  bool fill(const Buffer &buffer, T data, VkDeviceSize offset = 0,
            VkDeviceSize length = VK_WHOLE_SIZE) const {
    vkCmdFillBuffer(vk_command_buffer_, buffer.handle(), offset, length,
                    *(const uint32_t *)&data);
    return true;
  }
  ///\brief
  /// Copies data directly from host memory into a buffer oject.
  ///\tparam T
  ///\param buffer **[in]**
  ///\param data **[in]**
  ///\param offset **[in]** multiple of 4
  ///\param length **[in]**
  ///\return bool
  template <typename T>
  bool update(const Buffer &buffer, const T *data, VkDeviceSize offset,
              VkDeviceSize length) const {
    vkCmdUpdateBuffer(vk_command_buffer_, buffer.handle(), offset, length,
                      (const uint32_t *)data);
    return true;
  }
  ///\brief
  /// Clears an image to a fixed color value
  ///\param image **[in]**
  ///\param layout **[in]** layout that the image is expected to be in when the
  /// clear command is executed. Accepted layouts are VK_IMAGE_LAYOUT_[GENERAL |
  /// TRANSFER_DST_OPTIMAL]. To clear images in different layouts you need to
  /// move them to one of these two layouts using a pipeline barrier before the
  /// clear command.
  ///\param ranges **[in]** regions to be cleared. Note: aspectMask must be set
  /// to VK_IMAGE_ASPECT_COLOR_BIT.
  ///\param color **[in]** Note: it is up to the application to fill the color
  /// union object correctly, no conversion is performed by the clear command.
  ///\return bool true if success
  bool clear(const Image &image, VkImageLayout layout,
             const std::vector<VkImageSubresourceRange> &ranges,
             const VkClearColorValue &color) const;
  ///\brief
  /// Clears a depth stencil image to a fixed value
  ///\param image **[in]**
  ///\param layout **[in]** layout that the image is expected to be in when the
  /// clear command is executed. Accepted layouts are VK_IMAGE_LAYOUT_[GENERAL |
  /// TRANSFER_DST_OPTIMAL]. To clear images in different layouts you need to
  /// move them to one of these two layouts using a pipeline barrier before the
  /// clear command.
  ///\param ranges **[in]** regions to be cleared. Note: aspectMask must be set
  /// to VK_IMAGE_ASPECT_[DEPTH_BIT or/and STENCIL_BIT].
  ///\param value **[in]**
  ///\return bool true if success
  bool clear(const Image &image, VkImageLayout layout,
             const std::vector<VkImageSubresourceRange> &ranges,
             const VkClearDepthStencilValue &value) const;
  ///\brief
  ///
  ///\param compute_pipeline **[in]**
  ///\return bool
  bool bind(const ComputePipeline &compute_pipeline) const;
  ///\brief
  ///
  ///\param graphics_pipeline **[in]**
  ///\return bool
  bool bind(const GraphicsPipeline &graphics_pipeline) const;
  ///\brief
  ///
  ///\param pipeline_bind_point **[in]** VK_PIPELINE_BIND_POINT_[COMPUTE |
  /// GRAPHICS]
  ///\param pipeline_layout **[in]** the layout that will be used by pipelines
  /// that will access the descriptors
  ///\param descriptor_sets **[in]**
  ///\param dynamic_offsets **[in | default = empty vector]** offsets used in
  /// dynamic uniform or shader storage bindings
  ///\param first_set **[in | default = 0]** index of the first set to bind
  ///\param descriptor_set_count **[in | default = descriptor_sets.size()]**
  ///\return bool
  bool bind(VkPipelineBindPoint pipeline_bind_point,
            PipelineLayout &pipeline_layout,
            const std::vector<VkDescriptorSet> &descriptor_sets,
            const std::vector<uint32_t> &dynamic_offsets, uint32_t first_set,
            uint32_t descriptor_set_count) const;
  ///\brief Dispatches a glocal work group
  /// Note: A valid ComputePipeline must be bound to the command buffer
  ///\param x **[in]** number of local work groups in x
  ///\param y **[in]** number of local work groups in y
  ///\param z **[in]** number of local work groups in z
  ///\return bool
  bool dispatch(uint32_t x, uint32_t y, uint32_t z) const;
  ///\brief Performs an indirect dispatch
  /// The size of the dispatch in work groups is sourced from a buffer object
  ///\param buffer **[in]** buffer storing the x, y and z values contiguously
  ///\param offset **[in]** location of the workgroup sizes in the buffer (in
  /// bytes)
  /// \return bool
  bool dispatch(const Buffer &buffer, VkDeviceSize offset) const;
  ///\brief
  /// The content of each push constant lives at an offset from the beginning of
  /// the block.
  ///\param pipeline_layout **[in]**
  ///\param stage_flags **[in]** that stages that will need to see the updated
  /// constants
  ///\param offset **[in]**
  ///\param size **[in]** (in bytes)
  ///\param values **[in]**
  ///\return bool
  bool pushConstants(PipelineLayout &pipeline_layout,
                     VkShaderStageFlags stage_flags, uint32_t offset,
                     uint32_t size, const void *values) const;
  ///\brief Generates vertices and push them into the current graphics pipeline.
  ///\param vertex_count **[in]**
  ///\param instance_count **[in]**
  ///\param first_vertex **[in]**
  ///\param first_instance **[in]**
  ///\return bool
  bool draw(uint32_t vertex_count, uint32_t instance_count = 1,
            uint32_t first_vertex = 0, uint32_t first_instance = 0) const;

private:
  VkCommandBuffer vk_command_buffer_ = VK_NULL_HANDLE;
};

/// Command pools cannot be used concurrently, we must create a separate
/// command pool for each thread.
class CommandPool {
public:
  /// \brief Create a Command Pool object
  /// \param logical_device **[in]** logical device handle
  /// \param parameters **[in]** flags that define how the command pool will
  /// handle command buffer creation, memory, life span, etc.
  /// \param queue_family **[in]** queue family to which command buffers will
  /// be submitted.
  CommandPool(const LogicalDevice &logical_device,
              VkCommandPoolCreateFlags parameters, uint32_t queue_family);
  ~CommandPool();
  ///\brief
  ///
  ///\param level **[in]**
  ///\param count **[in]**
  ///\param command_buffers **[in]**
  ///\return bool
  bool
  allocateCommandBuffers(VkCommandBufferLevel level, uint32_t count,
                         std::vector<CommandBuffer> &command_buffers) const;
  ///\brief
  ///
  ///\param flags **[in]**
  ///\return bool
  bool reset(VkCommandPoolResetFlags flags) const;

private:
  const LogicalDevice &logical_device_;
  VkCommandPool vk_command_pool_ = VK_NULL_HANDLE;
};

} // namespace vk

} // namespace circe

#endif