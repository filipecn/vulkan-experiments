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
///\file vk_swap_chain.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-10-30
///
///\brief

#include "vk_swap_chain.h"
#include "logging.h"
#include "vulkan_debug.h"

namespace circe {

namespace vk {

Swapchain::Swapchain(const LogicalDevice *logical_device,
                     VkSurfaceKHR presentation_surface, uint32_t image_count,
                     VkSurfaceFormatKHR surface_format, VkExtent2D image_size,
                     VkImageUsageFlags image_usage,
                     VkSurfaceTransformFlagBitsKHR surface_transform,
                     VkPresentModeKHR present_mode)
    : logical_device_(logical_device) {

  info_.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  info_.pNext = nullptr;
  info_.flags = 0;
  set(presentation_surface, image_count, surface_format, image_size,
      image_usage, surface_transform, present_mode);
}

Swapchain::~Swapchain() { destroy(); }

void Swapchain::destroy() {
  if (vk_swapchain_ != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(logical_device_->handle());
    vkDestroySwapchainKHR(logical_device_->handle(), vk_swapchain_, nullptr);
    vk_swapchain_ = VK_NULL_HANDLE;
  }
}

VkSwapchainKHR Swapchain::handle() {
  if (vk_swapchain_ == VK_NULL_HANDLE) {
    VkResult result = vkCreateSwapchainKHR(logical_device_->handle(), &info_,
                                           nullptr, &vk_swapchain_);
    CHECK_VULKAN(result);
    if (result != VK_SUCCESS) {
      INFO("Could not create a swapchain.");
      return VK_NULL_HANDLE;
    }
    std::vector<VkImage> images;
    uint32_t images_count = 0;
    result = vkGetSwapchainImagesKHR(logical_device_->handle(), vk_swapchain_,
                                     &images_count, nullptr);
    CHECK_VULKAN(result);
    if (0 == images_count) {
      INFO("Could not get the number of swapchain images.");
      return VK_NULL_HANDLE;
    }
    images.resize(images_count);
    result = vkGetSwapchainImagesKHR(logical_device_->handle(), vk_swapchain_,
                                     &images_count, images.data());
    CHECK_VULKAN(result);
    if (0 == images_count) {
      INFO("Could not enumerate swapchain images.");
      return VK_NULL_HANDLE;
    }
    images_.clear();
    for (auto &image : images)
      images_.emplace_back(logical_device_, image);
  }

  return vk_swapchain_;
}

void Swapchain::set(VkSurfaceKHR presentation_surface, uint32_t image_count,
                    VkSurfaceFormatKHR surface_format, VkExtent2D image_size,
                    VkImageUsageFlags image_usage,
                    VkSurfaceTransformFlagBitsKHR surface_transform,
                    VkPresentModeKHR present_mode) {
  info_.surface = presentation_surface;
  info_.minImageCount = image_count;
  info_.imageFormat = surface_format.format;
  info_.imageColorSpace = surface_format.colorSpace;
  info_.imageExtent = image_size;
  info_.imageArrayLayers = 1;
  info_.imageUsage = image_usage;
  info_.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  info_.queueFamilyIndexCount = 0;
  info_.pQueueFamilyIndices = nullptr;
  info_.preTransform = surface_transform;
  info_.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  info_.presentMode = present_mode;
  info_.clipped = VK_TRUE;
  info_.oldSwapchain = VK_NULL_HANDLE;

  image_size_ = image_size;
  surface_format_ = surface_format;
}

VkResult Swapchain::nextImage(VkSemaphore semaphore, VkFence fence,
                              uint32_t &image_index) const {
  return vkAcquireNextImageKHR(logical_device_->handle(), vk_swapchain_,
                               2000000000, semaphore, fence, &image_index);
}

const std::vector<Image> &Swapchain::images() {
  handle();
  return images_;
}

VkExtent2D Swapchain::imageSize() const { return image_size_; }

VkSurfaceFormatKHR Swapchain::surfaceFormat() const { return surface_format_; }

} // namespace vk

} // namespace circe