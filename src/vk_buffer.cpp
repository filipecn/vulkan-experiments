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
/// \file vk_buffer.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-09-04
///
/// \brief

#include "vk_buffer.h"
#include "vulkan_debug.h"

namespace circe {

namespace vk {

Buffer::Buffer(VkDevice logical_device, VkDeviceSize size,
               VkBufferUsageFlags usage, VkSharingMode sharingMode) {
  VkBufferCreateInfo buffer_create_info = {
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, // sType
      nullptr,                              // pNext
      0,                                    // flags
      size,                                 // size
      usage,                                // usage
      VK_SHARING_MODE_EXCLUSIVE,            // sharing mode
      0,                                    // queue family index count
      nullptr                               // queue family indices pointer
  };
  ASSERT_VULKAN(
      vkCreateBuffer(logical_device, &buffer_create_info, nullptr, &buffer_));
}

} // namespace vk

} // namespace circe