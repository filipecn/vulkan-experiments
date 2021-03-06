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
///\file vk_device_memory.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-10-24
///
///\brief

#include "vk_device_memory.h"
#include "logging.h"
#include "vulkan_debug.h"

namespace circe {

namespace vk {

DeviceMemoryPool::DeviceMemoryPool(const LogicalDevice &device,
                                   VkDeviceSize size, uint32_t type)
    : device_(device) {
  VkMemoryAllocateInfo buffer_memory_allocate_info = {
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // VkStructureType    sType
      nullptr,                                // const void       * pNext
      size, // VkDeviceSize       allocationSize
      type  // uint32_t           memoryTypeIndex
  };
  CHECK_VULKAN(vkAllocateMemory(device_.handle(), &buffer_memory_allocate_info,
                                nullptr, &vk_device_memory_));

  if (vk_device_memory_ == VK_NULL_HANDLE)
    INFO("Could not create device memory pool.");
}

DeviceMemoryPool::~DeviceMemoryPool() {
  if (VK_NULL_HANDLE != vk_device_memory_)
    vkFreeMemory(device_.handle(), vk_device_memory_, nullptr);
}

DeviceMemory::DeviceMemory(const Image &image,
                           VkMemoryPropertyFlags required_flags,
                           VkMemoryPropertyFlags preferred_flags)
    : device_(image.device()) {
  VkMemoryRequirements memory_requirements{};
  if (!image.memoryRequirements(memory_requirements))
    return;
  allocate(memory_requirements, required_flags, preferred_flags);
}

DeviceMemory::DeviceMemory(const Buffer &buffer,
                           VkMemoryPropertyFlags required_flags,
                           VkMemoryPropertyFlags preferred_flags)
    : device_(buffer.device()) {
  VkMemoryRequirements memory_requirements{};
  if (!buffer.memoryRequirements(memory_requirements))
    return;
  allocate(memory_requirements, required_flags, preferred_flags);
}

DeviceMemory::DeviceMemory(DeviceMemory &other)
    : device_(other.device_), vk_device_memory_(other.vk_device_memory_) {
  other.vk_device_memory_ = VK_NULL_HANDLE;
}

DeviceMemory::DeviceMemory(DeviceMemory &&other) noexcept
    : device_(other.device_), vk_device_memory_(other.vk_device_memory_) {
  other.vk_device_memory_ = VK_NULL_HANDLE;
}

DeviceMemory::~DeviceMemory() { destroy(); }

void DeviceMemory::destroy() {
  if (device_ && VK_NULL_HANDLE != vk_device_memory_)
    vkFreeMemory(device_->handle(), vk_device_memory_, nullptr);
}

bool DeviceMemory::allocate(VkMemoryRequirements memory_requirements,
                            VkMemoryPropertyFlags required_flags,
                            VkMemoryPropertyFlags preferred_flags) {
  if (!device_)
    return false;
  destroy();
  uint32_t heap_index = device_->chooseMemoryType(
      memory_requirements, required_flags, preferred_flags);
  // try to allocate memory
  VkMemoryAllocateInfo buffer_memory_allocate_info = {
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // VkStructureType    sType
      nullptr,                                // const void       * pNext
      memory_requirements.size, // VkDeviceSize       allocationSize
      heap_index                // uint32_t           memoryTypeIndex
  };
  R_CHECK_VULKAN(vkAllocateMemory(device_->handle(),
                                  &buffer_memory_allocate_info, nullptr,
                                  &vk_device_memory_));
  return true;
}

bool DeviceMemory::bind(const Buffer &buffer, VkDeviceSize offset) {
  R_CHECK_VULKAN(vkBindBufferMemory(device_->handle(), buffer.handle(),
                                    vk_device_memory_, offset));
  return true;
}

bool DeviceMemory::bind(const Image &image, VkDeviceSize offset) {
  R_CHECK_VULKAN(vkBindImageMemory(device_->handle(), image.handle(),
                                   vk_device_memory_, offset));
  return true;
}

bool DeviceMemory::copy(const void *data, VkDeviceSize size,
                        VkDeviceSize offset) {
  void *d_data;
  CHECK_VULKAN(vkMapMemory(device_->handle(), vk_device_memory_, offset, size,
                           0, &d_data));
  memcpy(d_data, data, (size_t)size);
  vkUnmapMemory(device_->handle(), vk_device_memory_);
  return true;
}

bool DeviceMemory::map(VkDeviceSize size, VkDeviceSize offset) {
  CHECK_VULKAN(vkMapMemory(device_->handle(), vk_device_memory_, offset, size,
                           0, &mapped_));
  return true;
}

void *DeviceMemory::mapped() { return mapped_; }

void DeviceMemory::unmap() {
  if (mapped_) {
    vkUnmapMemory(device_->handle(), vk_device_memory_);
    mapped_ = nullptr;
  }
}

bool DeviceMemory::flush(VkDeviceSize size, VkDeviceSize offset) {
  VkMappedMemoryRange mappedRange = {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = vk_device_memory_;
  mappedRange.offset = offset;
  mappedRange.size = size;
  CHECK_VULKAN(vkFlushMappedMemoryRanges(device_->handle(), 1, &mappedRange));
  return true;
}

bool DeviceMemory::invalidate(VkDeviceSize size, VkDeviceSize offset) {
  VkMappedMemoryRange mappedRange = {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = vk_device_memory_;
  mappedRange.offset = offset;
  mappedRange.size = size;
  CHECK_VULKAN(
      vkInvalidateMappedMemoryRanges(device_->handle(), 1, &mappedRange));
  return true;
}

const LogicalDevice *DeviceMemory::device() const { return device_; }

void DeviceMemory::setDevice(const LogicalDevice *logical_device) {
  device_ = logical_device;
}

} // namespace vk

} // namespace circe