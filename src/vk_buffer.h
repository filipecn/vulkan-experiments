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
/// \file vk_buffer.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-09-04
///
/// \brief

#ifndef CIRCE_VULKAN_BUFFER_H
#define CIRCE_VULKAN_BUFFER_H

#include "vulkan_logical_device.h"

namespace circe {

namespace vk {

/// \brief Holds a vulkan buffer object.
/// Buffers represent linear arrays of data, just like C arrays, and is one of
/// the types of resources provided by Vulkan. Buffers can be used as storage
/// buffers, textel buffers, used as a source of data for vertex attributes, and
/// various other purposes. The purpose of a buffer and its size are defined on
/// its construction.
class Buffer final {
public:
  /// Buffer views allow us to define how buffer's memory is accessed and
  /// interpreted. For example, we can choose to look at the buffer as a uniform
  /// texel buffer or as a storage texel buffer.
  class View final {
  public:
    /// Creates a buffer view of a portion of the given buffer
    /// \param buffer **[in]** buffer
    /// \param format **[in]** how buffer contents should be interpreted
    /// \param memory_offset **[in]** view's starting point
    /// \param memory_range **[in]** size of the view
    View(const Buffer &buffer, VkFormat format, VkDeviceSize memory_offset,
         VkDeviceSize memory_range);
    ~View();

  private:
    const Buffer &buffer_;
    VkBufferView vk_buffer_view_ = VK_NULL_HANDLE;
  };

  /// VK_BUFFER_USAGE_TRANSFER_SRC_BIT specifies that the buffer can be a source
  /// of data for copy operations
  /// VK_BUFFER_USAGE_TRANSFER_DST_BIT specifies that we can copy data to the
  /// buffer
  /// VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT indicates that the buffer can be
  /// used in shaders as a uniform texel buffer
  /// VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT specifies that the buffer can be
  /// used in shaders as a storage texel buffer
  /// VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT indicates that the buffer can be used
  /// in shaders as a source of values for uniform variables
  /// VK_BUFFER_USAGE_STORAGE_BUFFER_BIT indicates that we can store data in the
  /// buffer from within shaders
  /// VK_BUFFER_USAGE_INDEX_BUFFER_BIT specifies that the buffer can be used as
  /// a source of vertex indices during drawing
  /// VK_BUFFER_USAGE_VERTEX_BUFFER_BIT indicates that the buffer can be a
  /// source of data for vertex attributes specified during drawing
  /// VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT indicates that the buffer can contain
  /// data that will be used during indirect drawing
  /// \brief Construct a new Buffer object

  /// \param logical_device **[in]** logical device handle
  /// \param size **[in]** buffer size (in bytes)
  /// \param usage **[in]** buffer usage type
  /// \param sharingMode **[in | optional = VK_SHARING_MODE_EXCLUSIVE]**
  /// specifies whether queues from multiple families can access the buffer at
  /// the same time.
  Buffer(const LogicalDevice *logical_device, VkDeviceSize size,
         VkBufferUsageFlags usage, const void *data = nullptr,
         VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);
  ~Buffer();
  ///\return VkBuffer vulkan handle object
  VkBuffer handle() const;
  ///\brief
  ///
  ///\return bool
  bool good() const;
  ///\return const LogicalDevice& device owner of its resouce
  const LogicalDevice *device() const;
  VkDeviceSize size() const;
  const void *data() const;
  ///\brief Information about the type of memory and how much of it the
  /// image
  /// resource requires.
  ///\param memory_requirements **[out]**
  ///\return bool true if success
  bool memoryRequirements(VkMemoryRequirements &memory_requirements) const;
  void setData(const void *data, uint32_t size);
  template <typename T> const T &at(uint32_t i) const {
    return (reinterpret_cast<const T *>(data_))[i];
  }

private:
  const LogicalDevice *logical_device_ = nullptr;
  VkBufferCreateInfo info_ = {};
  VkBuffer vk_buffer_ = VK_NULL_HANDLE;
  void *data_ = nullptr;
};

} // namespace vk

} // namespace circe

#endif