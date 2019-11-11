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
///\file vk_fence.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-11-09
///
///\brief

#include "vk_fence.h"
#include "vulkan_debug.h"

namespace circe {

namespace vk {

Fence::Fence(const LogicalDevice &logical_device, VkFenceCreateFlags flags)
    : logical_device_(logical_device) {
  VkFenceCreateInfo info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr,
                            flags};
  VkResult result =
      vkCreateFence(logical_device.handle(), &info, nullptr, &vk_fence_);
  CHECK_VULKAN(result);
  if (result != VK_SUCCESS)
    vk_fence_ = VK_NULL_HANDLE;
}

Fence::~Fence() {
  if (vk_fence_ != VK_NULL_HANDLE)
    vkDestroyFence(logical_device_.handle(), vk_fence_, nullptr);
}

VkResult Fence::status() const {
  return vkGetFenceStatus(logical_device_.handle(), vk_fence_);
}

} // namespace vk

} // namespace circe