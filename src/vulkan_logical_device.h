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
///\file vulkan_logical_device.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-10-18
///
///\brief

#ifndef CIRCE_VULKAN_LOGICAL_DEVICE_H
#define CIRCE_VULKAN_LOGICAL_DEVICE_H

#include "vulkan_physical_device.h"

namespace circe {

namespace vk {
/// Stores information about queues requested to a logical device and the list
/// of priorities assifned to each queue
struct QueueFamilyInfo {
  std::string name;
  std::optional<uint32_t> family_index; //!< queue family index
  std::vector<float> priorities;        //!< list of queue priorities, [0.0,1.0]
  std::vector<VkQueue> vk_queues;
};
struct QueueFamilies {
  void add(uint32_t family_index, std::string name,
           std::vector<float> priorities = {1.f}) {
    families_.emplace_back();
    families_[families_.size() - 1].family_index = family_index;
    families_[families_.size() - 1].priorities = priorities;
    families_[families_.size() - 1].name = name;
    families_[families_.size() - 1].vk_queues.resize(priorities.size(),
                                                     VK_NULL_HANDLE);
  }
  const std::vector<QueueFamilyInfo> &families() const { return families_; }
  std::vector<QueueFamilyInfo> &families() { return families_; }
  const QueueFamilyInfo &family(std::string name) const {
    for (const auto &f : families_)
      if (f.name == name)
        return f;
    return families_[0];
  }

private:
  std::vector<QueueFamilyInfo> families_;
};
/// The logical device makes the interface of the application and the physical
/// device. Represents the hardware, along with the extensions and features
/// enabled for it and all the queues requested from it. The logical device
/// allows us to record commands, submit them to queues and acquire the
/// results.
/// Device queues need to be requested on device creation, we cannot create or
/// destroy queues explicitly. They are created/destroyed on logical device
/// creation/destruction.
class LogicalDevice {
public:
  ///\brief Construct a new Logical Device object
  ///\param physical_device **[in]** physical device object
  /// \param desired_extensions **[in]** desired extensions
  /// \param desired_features **[in]** desired features
  /// \param queue_infos **[in]** queues description
  LogicalDevice(const PhysicalDevice &physical_device,
                std::vector<char const *> const &desired_extensions,
                VkPhysicalDeviceFeatures *desired_features,
                QueueFamilies &queue_infos);
  ///\brief Default destructor
  ~LogicalDevice();
  ///\brief
  ///
  ///\return VkDevice
  VkDevice handle() const;
  /// Checks with physical device object construction succeded
  ///\return bool true if this can be used
  bool good() const;
  ///\brief Selects the index of memory type trying to satisfy the preferred
  /// requirements.
  ///\param memory_requirements **[in]** memory requirements for a particular
  /// resource.
  ///\param required_flags **[in]** hard requirements
  ///\param preferred_flags **[in]** soft requirements
  ///\return uint32_t memory type
  uint32_t chooseHeap(const VkMemoryRequirements &memory_requirements,
                      VkMemoryPropertyFlags required_flags,
                      VkMemoryPropertyFlags preferred_flags) const;
  bool waitIdle() const;

private:
  const PhysicalDevice &physical_device_;
  VkDevice vk_device_ = VK_NULL_HANDLE;
};

} // namespace vk

} // namespace circe

#endif