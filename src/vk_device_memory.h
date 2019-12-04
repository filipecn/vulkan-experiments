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
///\file vk_device_memory.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-10-24
///
///\brief

#ifndef CIRCE_VULKAN_DEVICE_MEMORY_H
#define CIRCE_VULKAN_DEVICE_MEMORY_H

#include "vk_buffer.h"
#include "vk_image.h"

namespace circe {

namespace vk {

class DeviceMemoryPool {
public:
  DeviceMemoryPool(const LogicalDevice &device, VkDeviceSize size,
                   uint32_t type);
  ~DeviceMemoryPool();

private:
  const LogicalDevice &device_;
  VkDeviceMemory vk_device_memory_ = VK_NULL_HANDLE;
};

class DeviceMemory final {
public:
  DeviceMemory(const LogicalDevice *device, const Buffer &buffer,
               VkMemoryPropertyFlags required_flags);
  DeviceMemory(const LogicalDevice *device, const Buffer &buffer,
               VkMemoryPropertyFlags required_flags,
               VkMemoryPropertyFlags preferred_flags);
  DeviceMemory(const LogicalDevice *device, const Image &image,
               VkMemoryPropertyFlags required_flags,
               VkMemoryPropertyFlags preferred_flags);
  DeviceMemory(const DeviceMemory &other) = delete;
  DeviceMemory(const DeviceMemory &&other) = delete;
  DeviceMemory(DeviceMemory &other);
  DeviceMemory(DeviceMemory &&other) noexcept;
  ~DeviceMemory();
  bool bind(const Buffer &buffer);
  bool bind(const Image &image);
  /// Unfortunately the driver may not immediately copy the data into the buffer
  /// memory, for example because of caching. It is also possible that writes to
  /// the buffer are not visible in the mapped memory yet. There are two ways to
  /// deal with that problem:
  /// - Use a memory heap that is host coherent, indicated with
  /// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
  /// - Call vkFlushMappedMemoryRanges to after writing to the mapped memory,
  /// and call vkInvalidateMappedMemoryRanges before reading from the mapped
  /// memory
  bool copy(const Buffer &buffer);

private:
  const LogicalDevice *device_ = nullptr;
  VkDeviceMemory vk_device_memory_ = VK_NULL_HANDLE;
};

} // namespace vk

} // namespace circe

#endif