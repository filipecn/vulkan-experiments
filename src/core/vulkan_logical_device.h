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
#include <map>

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
    for (size_t i = 0; i < families_.size(); ++i)
      if (families_[i].family_index == family_index) {
        family_info_indices_[name] = i;
        for (auto p : priorities) {
          families_[i].priorities.emplace_back(p);
          families_[i].vk_queues.push_back(VK_NULL_HANDLE);
        }
        return;
      }
    size_t family_info_index = families_.size();
    family_info_indices_[name] = family_info_index;
    families_.emplace_back();
    families_[family_info_index].family_index = family_index;
    families_[family_info_index].priorities = priorities;
    families_[family_info_index].name = name;
    families_[family_info_index].vk_queues.resize(priorities.size(),
                                                  VK_NULL_HANDLE);
  }
  const std::vector<QueueFamilyInfo> &families() const { return families_; }
  std::vector<QueueFamilyInfo> &families() { return families_; }
  const QueueFamilyInfo &family(std::string name) {
    auto it = family_info_indices_.find(name);
    if (it != family_info_indices_.end())
      return families_[family_info_indices_[name]];
    return families_[0];
  }

private:
  std::map<std::string, size_t> family_info_indices_;
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
  LogicalDevice(const PhysicalDevice *physical_device,
                std::vector<char const *> const &desired_extensions,
                VkPhysicalDeviceFeatures *desired_features,
                QueueFamilies &queue_infos,
                const std::vector<const char *> &validation_layers =
                std::vector<const char *>());
  ///\brief Default destructor
  ~LogicalDevice();
  ///\brief
  ///
  ///\return VkDevice
  [[nodiscard]] VkDevice handle() const;
  ///\return const PhysicalDevice&
  [[nodiscard]] const PhysicalDevice *physicalDevice() const;
  /// Checks physical device object construction success
  ///\return bool true if this can be used
  [[nodiscard]] bool good() const;
  ///\brief Selects the index of memory type trying to satisfy the preferred
  /// requirements.
  ///\param memory_requirements **[in]** memory requirements for a particular
  /// resource.
  ///\param required_flags **[in]** hard requirements
  ///\param preferred_flags **[in]** soft requirements
  ///\return uint32_t memory type
  uint32_t chooseMemoryType(const VkMemoryRequirements &memory_requirements,
                            VkMemoryPropertyFlags required_flags,
                            VkMemoryPropertyFlags preferred_flags) const;
  bool waitIdle() const;

private:
  const PhysicalDevice *physical_device_{nullptr};
  VkDevice vk_device_{VK_NULL_HANDLE};
};

} // namespace vk

} // namespace circe

#endif