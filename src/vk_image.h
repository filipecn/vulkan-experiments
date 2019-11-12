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

#include "vulkan_logical_device.h"

namespace circe {

namespace vk {

/// \brief Holds a vulkan image object
/// Images represent data that can have multiple dimensions. They also have
/// mipmap levels and layers. Each texel can have multiple samples. Images
/// can serve as source of data for copy operations and work similar to OpenGL
/// textures.
class Image final {
public:
  /// An image view is a collection of properties and a reference to the image
  /// resouce. It allows all or part of an existing image to be seen as a
  /// different format.
  class View {
  public:
    /// Creates a iamge view of a portion of the given image
    /// \param view_type **[in]**
    /// \param format **[in]** data format
    /// \param aspect **[in]** context: color, depth or stencil
    /// \param image_view **[out]** image view object
    /// \return bool true if success
    View(const Image &image, VkImageViewType view_type, VkFormat format,
         VkImageAspectFlags aspect);
    View(const View &&other) = delete;
    View(View &&other);
    ~View();
    VkImageView handle() const;

  private:
    const Image &image_;
    VkImageView vk_image_view_ = VK_NULL_HANDLE;
  };

  // VK_IMAGE_USAGE_TRANSFER_SRC_BIT specifies that the image can be used as
  // a source of data for copy operations VK_IMAGE_USAGE_TRANSFER_DST_BIT
  // specifies that we can copy data to the image VK_IMAGE_USAGE_SAMPLED_BIT
  // indicates that we can sample data from the image inside shaders
  // VK_IMAGE_USAGE_STORAGE_BIT specifies that the image can be used as a
  // storage image inside shaders
  // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT specifies that we can render into
  // an image (use it as a color render target/attachment in a framebuffer)
  // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT indicates that the image
  // can be used as a depth and/or stencil buffer (as a depth render
  // target/attachment in a framebuffer)
  // VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT indicates that the memory bound
  // to the image will be allocated lazily (on demand)
  // VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT specifies that the image can be
  // used as an input attachment inside shaders

  /// \param logical_device **[in]** logical device (on which the image
  /// will be created)
  /// \param type **[in]** number of dimensions of the image
  /// \param format **[in]** number of components and number of bits of a
  /// texel in the image \param size **[in]** image's dimensions (in texels)
  /// \param num_mipmaps **[in]** number of mipmap levels
  /// \param num_layers **[in]** number of layers
  /// \param samples **[in]** number of samples
  /// \param usage_scenarios **[in]**
  /// \param cubemap **[in]**
  Image(const LogicalDevice &logical_device, VkImageType type, VkFormat format,
        VkExtent3D size, uint32_t num_mipmaps, uint32_t num_layers,
        VkSampleCountFlagBits samples, VkImageUsageFlags usage_scenarios,
        bool cubemap);
  Image(const LogicalDevice &logical_device, VkImage handle);
  ~Image();
  ///\return const LogicalDevice& device owner of its resouce
  const LogicalDevice &device() const;
  ///\return VkImage vulkan handle object
  VkImage handle() const;
  ///\brief
  ///
  ///\return bool
  bool good() const;
  ///\brief Retrieves information about a given subresource in the image.
  /// Images subresources are mipmap levels, depth or stencil components, array
  /// layers.
  ///\param aspect_mask **[in]** one or a combination of
  /// VK_IMAGE_ASPECT_[COLOR|DEPTH|STENCIL]_BIT
  /// ex : VK_IMAGE_ASPECT_COLOR_BIT for color images
  ///\param mip_level **[in]** the mipmap level for which the parameters
  /// are to be returned.
  ///\param array_layer **[in]** generally 0, as parameters are not expect
  /// to change across layers.
  ///\param subresource_layout **[out]**
  ///\return bool true if success
  bool subresourceLayout(VkImageAspectFlags aspect_mask, uint32_t mip_level,
                         uint32_t array_layer,
                         VkSubresourceLayout &subresource_layout) const;
  ///\brief Information about the type of memory and how much of it the image
  /// resource requires.
  ///\param memory_requirements **[out]**
  ///\return bool true if success
  bool memoryRequirements(VkMemoryRequirements &memory_requirements) const;

private:
  const LogicalDevice &logical_device_;
  VkImage vk_image_ = VK_NULL_HANDLE;
  bool do_not_destroy_ = false;
};

} // namespace vk

} // namespace circe

#endif