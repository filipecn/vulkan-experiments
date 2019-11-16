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

#include "vk_sync.h"
#include "vulkan_debug.h"

namespace circe {

namespace vk {

Fence::Fence(const LogicalDevice *logical_device, VkFenceCreateFlags flags)
    : logical_device_(logical_device) {
  VkFenceCreateInfo info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr,
                            flags};
  VkResult result =
      vkCreateFence(logical_device->handle(), &info, nullptr, &vk_fence_);
  CHECK_VULKAN(result);
  if (result != VK_SUCCESS)
    vk_fence_ = VK_NULL_HANDLE;
}

Fence::Fence(Fence &other)
    : logical_device_(other.logical_device_), vk_fence_(other.vk_fence_) {
  other.vk_fence_ = VK_NULL_HANDLE;
}

Fence::Fence(Fence &&other)
    : logical_device_(other.logical_device_), vk_fence_(other.vk_fence_) {
  other.vk_fence_ = VK_NULL_HANDLE;
}

Fence::~Fence() {
  if (vk_fence_ != VK_NULL_HANDLE)
    vkDestroyFence(logical_device_->handle(), vk_fence_, nullptr);
}

VkFence Fence::handle() const { return vk_fence_; }

VkResult Fence::status() const {
  return vkGetFenceStatus(logical_device_->handle(), vk_fence_);
}

void Fence::wait() const {
  vkWaitForFences(logical_device_->handle(), 1, &vk_fence_, VK_TRUE,
                  UINT64_MAX);
}

void Fence::reset() const {
  vkResetFences(logical_device_->handle(), 1, &vk_fence_);
}

Semaphore::Semaphore(const LogicalDevice *logical_device,
                     VkSemaphoreCreateFlags flags)
    : logical_device_(logical_device) {
  VkSemaphoreCreateInfo info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                                nullptr, flags};
  VkResult result = vkCreateSemaphore(logical_device->handle(), &info, nullptr,
                                      &vk_semaphore_);
  CHECK_VULKAN(result);
  if (result != VK_SUCCESS)
    vk_semaphore_ = VK_NULL_HANDLE;
}

Semaphore::Semaphore(Semaphore &other)
    : logical_device_(other.logical_device_),
      vk_semaphore_(other.vk_semaphore_) {
  other.vk_semaphore_ = VK_NULL_HANDLE;
}

Semaphore::Semaphore(Semaphore &&other)
    : logical_device_(other.logical_device_),
      vk_semaphore_(other.vk_semaphore_) {
  other.vk_semaphore_ = VK_NULL_HANDLE;
}

Semaphore::~Semaphore() {
  if (vk_semaphore_ != VK_NULL_HANDLE)
    vkDestroySemaphore(logical_device_->handle(), vk_semaphore_, nullptr);
}

VkSemaphore Semaphore::handle() const { return vk_semaphore_; }

} // namespace vk

} // namespace circe