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
  CommandBuffer(VkCommandBuffer vk_command_buffer);
  ~CommandBuffer() = default;

private:
  VkCommandBuffer vk_command_buffer = VK_NULL_HANDLE;
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
  bool
  allocateCommandBuffers(VkCommandBufferLevel level, uint32_t count,
                         std::vector<CommandBuffer> &command_buffers) const;

private:
  const LogicalDevice &logical_device_;
  VkCommandPool vk_command_pool_ = VK_NULL_HANDLE;
};

} // namespace vk

} // namespace circe

#endif