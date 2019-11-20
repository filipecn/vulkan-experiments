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
#include "logging.h"
#include "vulkan_debug.h"

namespace circe {

namespace vk {

Buffer::Buffer(const LogicalDevice *logical_device, VkDeviceSize size,
               VkBufferUsageFlags usage, const void *data,
               VkSharingMode sharingMode)
    : logical_device_(logical_device) {
  info_.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  info_.pNext = nullptr;
  info_.flags = 0;
  info_.size = size;
  info_.usage = usage;
  info_.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  info_.queueFamilyIndexCount = 0;
  info_.pQueueFamilyIndices = nullptr;
  CHECK_VULKAN(
      vkCreateBuffer(logical_device->handle(), &info_, nullptr, &vk_buffer_));
  if (vk_buffer_ == VK_NULL_HANDLE)
    INFO("Could not create buffer.");
  data_ = new char[size];
  if (data)
    setData(data, size);
}

Buffer::~Buffer() {
  if (data_)
    delete[] data_;
  if (VK_NULL_HANDLE != vk_buffer_)
    vkDestroyBuffer(logical_device_->handle(), vk_buffer_, nullptr);
}

VkBuffer Buffer::handle() const { return vk_buffer_; }

bool Buffer::good() const { return vk_buffer_ != VK_NULL_HANDLE; }

const LogicalDevice *Buffer::device() const { return logical_device_; }

VkDeviceSize Buffer::size() const { return info_.size; }

const void *Buffer::data() const { return data_; }

bool Buffer::memoryRequirements(
    VkMemoryRequirements &memory_requirements) const {
  vkGetBufferMemoryRequirements(logical_device_->handle(), vk_buffer_,
                                &memory_requirements);
  return true;
}

void Buffer::setData(const void *data, uint32_t size) {
  ASSERT(size == info_.size);
  memcpy(data_, data, size);
}

Buffer::View::View(const Buffer &buffer, VkFormat format,
                   VkDeviceSize memory_offset, VkDeviceSize memory_range)
    : buffer_(buffer) {
  VkBufferViewCreateInfo buffer_view_create_info = {
      VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO, // VkStructureType sType
      nullptr,         // const void               * pNext
      0,               // VkBufferViewCreateFlags    flags
      buffer.handle(), // VkBuffer                   buffer
      format,          // VkFormat                   format
      memory_offset,   // VkDeviceSize               offset
      memory_range     // VkDeviceSize               range
  };

  CHECK_VULKAN(vkCreateBufferView(buffer.device()->handle(),
                                  &buffer_view_create_info, nullptr,
                                  &vk_buffer_view_));
  if (vk_buffer_view_ == VK_NULL_HANDLE)
    INFO("Could not create buffer view.");
}

Buffer::View::~View() {
  if (VK_NULL_HANDLE != vk_buffer_view_)
    vkDestroyBufferView(buffer_.device()->handle(), vk_buffer_view_, nullptr);
}

} // namespace vk

} // namespace circe