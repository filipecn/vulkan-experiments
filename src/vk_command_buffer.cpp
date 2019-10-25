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

namespace circe {

namespace vk {

CommandPool::CommandPool(const LogicalDevice &logical_device,
                         VkCommandPoolCreateFlags parameters,
                         uint32_t queue_family)
    : logical_device_(logical_device) {
  VkCommandPoolCreateInfo command_pool_create_info = {
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, // VkStructureType sType
      nullptr,     // const void                 * pNext
      parameters,  // VkCommandPoolCreateFlags     flags
      queue_family // uint32_t                     queueFamilyIndex
  };
  CHECK_VULKAN(vkCreateCommandPool(logical_device.handle(),
                                   &command_pool_create_info, nullptr,
                                   &vk_command_pool_));
  if (vk_command_pool_ == VK_NULL_HANDLE)
    INFO("Could not create the command buffer.");
}

CommandPool::~CommandPool() {
  if (vk_command_pool_ != VK_NULL_HANDLE)
    vkDestroyCommandPool(logical_device_.handle(), vk_command_pool_, nullptr);
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
  R_CHECK_VULKAN(vkAllocateCommandBuffers(logical_device_.handle(),
                                          &command_buffer_allocate_info,
                                          vk_command_buffers.data()));
  command_buffers.clear();
  for (auto cb : vk_command_buffers)
    command_buffers.emplace_back(cb);
  return true;
}

} // namespace vk

} // namespace circe