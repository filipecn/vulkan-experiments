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
///\file vulkan_logical_device.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-10-18
///
///\brief

#include "vulkan_logical_device.h"
#include "logging.h"
#include "vulkan_debug.h"

namespace circe {

namespace vk {

LogicalDevice::LogicalDevice(
    const PhysicalDevice &physical_device,
    std::vector<char const *> const &desired_extensions,
    VkPhysicalDeviceFeatures *desired_features, QueueFamilies &queue_infos)
    : physical_device_(physical_device) {
  for (auto &extension : desired_extensions)
    if (!physical_device.isExtensionSupported(extension)) {
      INFO(concat("Extension named '", extension,
                  "' is not supported by a physical device.\n"));
      return;
    }
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  for (auto &info : queue_infos.families()) {
    queue_create_infos.push_back({
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // VkStructureType sType
        nullptr,                   // const void                     * pNext
        0,                         // VkDeviceQueueCreateFlags         flags
        info.family_index.value(), // uint32_t queueFamilyIndex
        static_cast<uint32_t>(info.priorities.size()), // uint32_t queueCount
        info.priorities.data() // const float  * pQueuePriorities
    });
  };
  VkDeviceCreateInfo device_create_info = {
      VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, // VkStructureType sType
      nullptr, // const void                     * pNext
      0,       // VkDeviceCreateFlags              flags
      static_cast<uint32_t>(
          queue_create_infos.size()), // uint32_t queueCreateInfoCount
      queue_create_infos
          .data(), // const VkDeviceQueueCreateInfo  * pQueueCreateInfos
      0,           // uint32_t                         enabledLayerCount
      nullptr,     // const char * const             * ppEnabledLayerNames
      static_cast<uint32_t>(
          desired_extensions.size()), // uint32_t enabledExtensionCount
      desired_extensions
          .data(), // const char * const             * ppEnabledExtensionNames
      desired_features // const VkPhysicalDeviceFeatures * pEnabledFeatures
  };
  CHECK_VULKAN(vkCreateDevice(physical_device.handle(), &device_create_info,
                              nullptr, &vk_device_));
  if (vk_device_ == VK_NULL_HANDLE)
    INFO("Could not create logical device.");

  for (auto &info : queue_infos.families()) {
    for (size_t i = 0; i < info.priorities.size(); ++i) {
      vkGetDeviceQueue(vk_device_, info.family_index.value(), i,
                       &info.vk_queues[i]);
      if (info.vk_queues[i] == VK_NULL_HANDLE)
        INFO("Could not get device queue");
    }
  }
} // namespace vk

LogicalDevice::~LogicalDevice() {
  if (vk_device_)
    vkDestroyDevice(vk_device_, nullptr);
}

VkDevice LogicalDevice::handle() const { return vk_device_; }

bool LogicalDevice::good() const { return vk_device_ != VK_NULL_HANDLE; }

uint32_t
LogicalDevice::chooseHeap(const VkMemoryRequirements &memory_requirements,
                          VkMemoryPropertyFlags required_flags,
                          VkMemoryPropertyFlags preferred_flags) const {
  return physical_device_.chooseHeap(memory_requirements, required_flags,
                                     preferred_flags);
}

bool LogicalDevice::waitIdle() const {
  R_CHECK_VULKAN(vkDeviceWaitIdle(vk_device_));
  return true;
}

} // namespace vk

} // namespace circe