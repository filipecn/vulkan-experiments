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

Swapchain::Swapchain(const LogicalDevice &logical_device,
                     VkSurfaceKHR presentation_surface, uint32_t image_count,
                     VkSurfaceFormatKHR surface_format, VkExtent2D image_size,
                     VkImageUsageFlags image_usage,
                     VkSurfaceTransformFlagBitsKHR surface_transform,
                     VkPresentModeKHR present_mode)
    : logical_device_(logical_device) {
  create(presentation_surface, image_count, surface_format, image_size,
         image_usage, surface_transform, present_mode);
}

Swapchain::~Swapchain() {
  if (vk_swapchain_) {
    vkDeviceWaitIdle(logical_device_.handle());
    vkDestroySwapchainKHR(logical_device_.handle(), vk_swapchain_, nullptr);
  }
}

bool Swapchain::set(VkSurfaceKHR presentation_surface, uint32_t image_count,
                    VkSurfaceFormatKHR surface_format, VkExtent2D image_size,
                    VkImageUsageFlags image_usage,
                    VkSurfaceTransformFlagBitsKHR surface_transform,
                    VkPresentModeKHR present_mode) {
  return create(presentation_surface, image_count, surface_format, image_size,
                image_usage, surface_transform, present_mode);
}

bool Swapchain::nextImage(VkSemaphore semaphore, VkFence fence,
                          uint32_t &image_index) {
  VkResult result;
  result = vkAcquireNextImageKHR(logical_device_.handle(), vk_swapchain_,
                                 2000000000, semaphore, fence, &image_index);
  switch (result) {
  case VK_SUCCESS:
  case VK_SUBOPTIMAL_KHR:
    return true;
  default:
    return false;
  }
}

bool Swapchain::create(VkSurfaceKHR presentation_surface, uint32_t image_count,
                       VkSurfaceFormatKHR surface_format, VkExtent2D image_size,
                       VkImageUsageFlags image_usage,
                       VkSurfaceTransformFlagBitsKHR surface_transform,
                       VkPresentModeKHR present_mode) {
  VkSwapchainCreateInfoKHR swapchain_create_info = {
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, // VkStructureType sType
      nullptr,               // const void                     * pNext
      0,                     // VkSwapchainCreateFlagsKHR        flags
      presentation_surface,  // VkSurfaceKHR                     surface
      image_count,           // uint32_t                         minImageCount
      surface_format.format, // VkFormat                         imageFormat
      surface_format.colorSpace, // VkColorSpaceKHR imageColorSpace
      image_size,                // VkExtent2D                       imageExtent
      1,           // uint32_t                         imageArrayLayers
      image_usage, // VkImageUsageFlags                imageUsage
      VK_SHARING_MODE_EXCLUSIVE, // VkSharingMode imageSharingMode
      0,       // uint32_t                         queueFamilyIndexCount
      nullptr, // const uint32_t                 * pQueueFamilyIndices
      surface_transform, // VkSurfaceTransformFlagBitsKHR    preTransform
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, // VkCompositeAlphaFlagBitsKHR
                                         // compositeAlpha
      present_mode, // VkPresentModeKHR                 presentMode
      VK_TRUE,      // VkBool32                         clipped
      vk_swapchain_ // VkSwapchainKHR                   oldSwapchain
  };
  R_CHECK_VULKAN(vkCreateSwapchainKHR(logical_device_.handle(),
                                      &swapchain_create_info, nullptr,
                                      &vk_swapchain_));
  CHECK_INFO(VK_NULL_HANDLE == vk_swapchain_, "Could not create a swapchain.");

  {
    uint32_t images_count = 0;
    R_CHECK_VULKAN(vkGetSwapchainImagesKHR(
        logical_device_.handle(), vk_swapchain_, &images_count, nullptr));
    D_RETURN_FALSE_IF_NOT(0 != images_count,
                          "Could not get the number of swapchain images.");
    images_.resize(images_count);
    R_CHECK_VULKAN(vkGetSwapchainImagesKHR(logical_device_.handle(),
                                           vk_swapchain_, &images_count,
                                           images_.data()));
    D_RETURN_FALSE_IF_NOT(0 != images_count,
                          "Could not enumerate swapchain images.");
  }

  return true;
}

} // namespace vk

} // namespace circe