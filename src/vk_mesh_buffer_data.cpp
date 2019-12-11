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
/// \file vk_mesh_buffer_data.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-12-10
///
/// \brief

#include "vk_mesh_buffer_data.h"
#include "vk_command_buffer.h"

namespace circe::vk {

MeshBufferData::MeshBufferData(const LogicalDevice *logical_device,
                               VkDeviceSize vertex_buffer_size,
                               const void *vertex_data,
                               VkDeviceSize index_buffer_size,
                               const void *index_data,
                               uint32_t queue_family_index,
                               VkQueue queue)
    : logical_device_(logical_device) {
  Buffer staging_buffer
      (logical_device_,
       vertex_buffer_size,
       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
       vertex_data);
  DeviceMemory staging_buffer_memory(logical_device_, staging_buffer,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  staging_buffer_memory.bind(staging_buffer);
  staging_buffer_memory.copy(staging_buffer);

  Buffer index_staging_buffer(logical_device_, index_buffer_size,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT, index_data);
  DeviceMemory
      index_staging_buffer_memory(logical_device_, index_staging_buffer,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  index_staging_buffer_memory.bind(index_staging_buffer);
  index_staging_buffer_memory.copy(index_staging_buffer);

  buffer_ = std::make_unique<Buffer>(logical_device_, vertex_buffer_size,
                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  buffer_memory_ = std::make_unique<DeviceMemory>(logical_device_,
                                                  *buffer_,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  buffer_memory_->bind(*buffer_);
  index_buffer_ = std::make_unique<Buffer>(logical_device, index_buffer_size,
                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                               VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  index_buffer_memory_ = std::make_unique<DeviceMemory>(logical_device_,
                                                        *index_buffer_,
                                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  index_buffer_memory_->bind(*index_buffer_);

  CommandPool::submitCommandBuffer(logical_device_,
                                   queue_family_index,
                                   queue,
                                   [&](circe::vk::CommandBuffer &cb) {
                                     cb.copy(staging_buffer,
                                             0,
                                             *buffer_,
                                             0,
                                             vertex_buffer_size);
                                     cb.copy(index_staging_buffer,
                                             0,
                                             *index_buffer_,
                                             0,
                                             index_buffer_size);
                                   });
}

const Buffer *MeshBufferData::vertexBuffer() const {
  return buffer_.get();
}

const Buffer *MeshBufferData::indexBuffer() const {
  return index_buffer_.get();
}

} // circe::vk namespace
