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
/// \file vk_image.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-09-08
///
/// \brief

#include "vk_image.h"
#include "logging.h"
#include "vulkan_debug.h"

namespace circe {

namespace vk {

Image::View::View(const Image &image, VkImageViewType view_type,
                  VkFormat format, VkImageAspectFlags aspect)
    : image_(image) {
  VkImageViewCreateInfo image_view_create_info = {
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // VkStructureType sType
      nullptr,        // const void               * pNext
      0,              // VkImageViewCreateFlags     flags
      image.handle(), // VkImage                    image
      view_type,      // VkImageViewType            viewType
      format,         // VkFormat                   format
      {
          // VkComponentMapping         components
          VK_COMPONENT_SWIZZLE_IDENTITY, // VkComponentSwizzle         r
          VK_COMPONENT_SWIZZLE_IDENTITY, // VkComponentSwizzle         g
          VK_COMPONENT_SWIZZLE_IDENTITY, // VkComponentSwizzle         b
          VK_COMPONENT_SWIZZLE_IDENTITY  // VkComponentSwizzle         a
      },
      {
          // VkImageSubresourceRange    subresourceRange
          aspect,                   // VkImageAspectFlags         aspectMask
          0,                        // uint32_t                   baseMipLevel
          VK_REMAINING_MIP_LEVELS,  // uint32_t                   levelCount
          0,                        // uint32_t                   baseArrayLayer
          VK_REMAINING_ARRAY_LAYERS // uint32_t                   layerCount
      }};

  CHECK_VULKAN(vkCreateImageView(image.device().handle(),
                                 &image_view_create_info, nullptr,
                                 &vk_image_view_));
}

Image::View::View(Image::View &&other) : image_(other.image_) {
  vk_image_view_ = other.vk_image_view_;
  other.vk_image_view_ = VK_NULL_HANDLE;
}

Image::View::~View() {
  if (VK_NULL_HANDLE != vk_image_view_)
    vkDestroyImageView(image_.device().handle(), vk_image_view_, nullptr);
}

VkImageView Image::View::handle() const { return vk_image_view_; }

Image::Image(const LogicalDevice &logical_device, VkImageType type,
             VkFormat format, VkExtent3D size, uint32_t num_mipmaps,
             uint32_t num_layers, VkSampleCountFlagBits samples,
             VkImageUsageFlags usage_scenarios, bool cubemap)
    : logical_device_(logical_device), do_not_destroy_(false) {
  VkImageCreateInfo image_create_info = {
      VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, // VkStructureType          sType
      nullptr,                             // const void             * pNext
      cubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
              : 0u, // VkImageCreateFlags       flags
      type,         // VkImageType              imageType
      format,       // VkFormat                 format
      size,         // VkExtent3D               extent
      num_mipmaps,  // uint32_t                 mipLevels
      cubemap ? 6 * num_layers : num_layers, // uint32_t arrayLayers
      samples,                               // VkSampleCountFlagBits    samples
      VK_IMAGE_TILING_OPTIMAL,               // VkImageTiling            tiling
      usage_scenarios,                       // VkImageUsageFlags        usage
      VK_SHARING_MODE_EXCLUSIVE, // VkSharingMode            sharingMode
      0,       // uint32_t                 queueFamilyIndexCount
      nullptr, // const uint32_t         * pQueueFamilyIndices
      VK_IMAGE_LAYOUT_UNDEFINED // VkImageLayout            initialLayout
  };
  CHECK_VULKAN(vkCreateImage(logical_device.handle(), &image_create_info,
                             nullptr, &vk_image_));
  if (vk_image_ == VK_NULL_HANDLE)
    INFO("Could not create image.");
}

Image::Image(const LogicalDevice &logical_device, VkImage handle)
    : logical_device_(logical_device), vk_image_(handle),
      do_not_destroy_(true) {}

Image::~Image() {
  if (VK_NULL_HANDLE != vk_image_ && !do_not_destroy_)
    vkDestroyImage(logical_device_.handle(), vk_image_, nullptr);
}

VkImage Image::handle() const { return vk_image_; }

const LogicalDevice &Image::device() const { return logical_device_; }

bool Image::good() const { return vk_image_ != VK_NULL_HANDLE; }

bool Image::subresourceLayout(VkImageAspectFlags aspect_mask,
                              uint32_t mip_level, uint32_t array_layer,
                              VkSubresourceLayout &subresource_layout) const {
  VkImageSubresource subresource{};
  subresource.aspectMask = aspect_mask;
  subresource.mipLevel = mip_level;
  subresource.arrayLayer = array_layer;
  vkGetImageSubresourceLayout(logical_device_.handle(), vk_image_, &subresource,
                              &subresource_layout);
  return true;
}

bool Image::memoryRequirements(
    VkMemoryRequirements &memory_requirements) const {
  vkGetImageMemoryRequirements(logical_device_.handle(), vk_image_,
                               &memory_requirements);
  return true;
}

} // namespace vk

} // namespace circe