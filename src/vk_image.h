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
/// \file vk_image.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-09-08
///
/// \brief

#ifndef CIRCE_VK_IMAGE_H
#define CIRCE_VK_IMAGE_H

#include "vulkan_library.h"

namespace circe {

namespace vk {

/// \brief Holds a vulkan image object
/// Images represent data that can have multiple dimensions. They also have
/// mipmap levels and layers. Each texel can have multiple samples. Images
/// can serve as source of data for copy operations and work similar to OpenGL
/// textures.
class Image {
  // VK_IMAGE_USAGE_TRANSFER_SRC_BIT specifies that the image can be used as a
  // source of data for copy operations
  // VK_IMAGE_USAGE_TRANSFER_DST_BIT specifies that we can copy data to the
  // image
  // VK_IMAGE_USAGE_SAMPLED_BIT indicates that we can sample data from the image
  // inside shaders
  // VK_IMAGE_USAGE_STORAGE_BIT specifies that the image can be used as a
  // storage image inside shaders
  // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT specifies that we can render into an
  // image (use it as a color render target/attachment in a framebuffer)
  // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT indicates that the image
  // can be used as a depth and/or stencil buffer (as a depth render
  // target/attachment in a framebuffer)
  // VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT indicates that the memory bound to
  // the image will be allocated lazily (on demand)
  // VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT specifies that the image can be used as
  // an input attachment inside shaders

  /// \param logical_device **[in]** logical device handle (on which the image
  /// will be created)
  /// \param type **[in]** number of dimensions of the image
  /// \param format **[in]** number of components and number of bits of a texel
  /// in the image
  /// \param size **[in]** image's dimensions (in texels)
  /// \param num_mipmaps **[in]** number of mipmap levels
  /// \param num_layers **[in]** number of layers
  /// \param samples **[in]** number of samples
  /// \param usage_scenarios **[in]**
  /// \param cubemap **[in]**
  Image(VkDevice logical_device, VkImageType type, VkFormat format,
        VkExtent3D size, uint32_t num_mipmaps, uint32_t num_layers,
        VkSampleCountFlagBits samples, VkImageUsageFlags usage_scenarios,
        bool cubemap);

private:
  VkImage image;
};

} // namespace vk

} // namespace circe

#endif