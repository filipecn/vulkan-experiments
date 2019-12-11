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
/// \file vk_texture_image.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-12-09
///
/// \brief

#include "vk_texture_image.h"
#include "vk_command_buffer.h"
#include "vk_sync.h"
#include "vulkan_debug.h"
#include "vk_buffer.h"
#include "logging.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace circe::vk {

Texture::Texture(const LogicalDevice *logical_device,
                 const std::string &filename,
                 uint32_t queue_family_index,
                 VkQueue queue)
    : logical_device_(logical_device) {
  int tex_width, tex_height, tex_channels;
  stbi_uc *pixels = stbi_load(filename.c_str(),
                              &tex_width,
                              &tex_height,
                              &tex_channels,
                              STBI_rgb_alpha);
  if (!pixels) {
    INFO("could not load texture image file!");
    return;
  }
  VkDeviceSize image_size = tex_width * tex_height * 4;

  Buffer staging_buffer
      (logical_device_, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
       pixels);
  DeviceMemory staging_buffer_memory(logical_device_,
                                     staging_buffer,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                         | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  staging_buffer_memory.bind(staging_buffer);
  staging_buffer_memory.copy(staging_buffer);
  stbi_image_free(pixels);
  // Allocate image data on device
  VkExtent3D size = {};
  size.width = tex_width;
  size.height = tex_height;
  size.depth = 1;
  image_ = std::make_unique<Image>(logical_device_,
                                   VK_IMAGE_TYPE_2D,
                                   VK_FORMAT_R8G8B8A8_UNORM,
                                   size,
                                   1,
                                   1,
                                   VK_SAMPLE_COUNT_1_BIT,
                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT
                                       | VK_IMAGE_USAGE_SAMPLED_BIT,
                                   false);
  image_memory_ = std::make_unique<DeviceMemory>(logical_device_,
                                                 *image_,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  image_memory_->bind(*image_);
  // copy data to device
  CommandPool::submitCommandBuffer(logical_device_,
                                   queue_family_index,
                                   queue,
                                   [&](CommandBuffer &cb) {
                                     ImageMemoryBarrier barrier(*image_,
                                                                VK_IMAGE_LAYOUT_UNDEFINED,
                                                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                                     cb.transitionImageLayout(barrier,
                                                              VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                              VK_PIPELINE_STAGE_TRANSFER_BIT);
                                     VkBufferImageCopy region = {};
                                     region.bufferOffset = 0;
                                     region.bufferRowLength = 0;
                                     region.bufferImageHeight = 0;
                                     region.imageSubresource.aspectMask =
                                         VK_IMAGE_ASPECT_COLOR_BIT;
                                     region.imageSubresource.mipLevel = 0;
                                     region.imageSubresource.baseArrayLayer = 0;
                                     region.imageSubresource.layerCount = 1;
                                     region.imageOffset = {0, 0, 0};
                                     region.imageExtent = {
                                         size.width,
                                         size.height,
                                         1
                                     };
                                     cb.copy(staging_buffer,
                                             *image_,
                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                             {region});
                                     ImageMemoryBarrier after_barrier(*image_,
                                                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                                     cb.transitionImageLayout(after_barrier,
                                                              VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                                   });

}

const Image *Texture::image() const {
  return image_.get();
}

}
