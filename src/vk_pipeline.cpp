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
///\file vk_pipeline.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-11-01
///
///\brief

#include "vk_pipeline.h"
#include "vulkan_debug.h"
#include <fstream>

namespace circe {

namespace vk {

DescriptorSetLayout::DescriptorSetLayout(const LogicalDevice &logical_device)
    : logical_device_(logical_device) {}

DescriptorSetLayout::~DescriptorSetLayout() {
  if (vk_descriptor_set_layout_ != VK_NULL_HANDLE)
    vkDestroyDescriptorSetLayout(logical_device_.handle(),
                                 vk_descriptor_set_layout_, nullptr);
}

VkDescriptorSetLayout DescriptorSetLayout::handle() {
  if (vk_descriptor_set_layout_) {
    VkDescriptorSetLayoutCreateInfo info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
        bindings_.size(), bindings_.data()};
    VkResult result = vkCreateDescriptorSetLayout(
        logical_device_.handle(), &info, nullptr, &vk_descriptor_set_layout_);
    CHECK_VULKAN(result);
    if (result != VK_SUCCESS)
      vk_descriptor_set_layout_ = VK_NULL_HANDLE;
  }
  return vk_descriptor_set_layout_;
}

void DescriptorSetLayout::addLayoutBinding(uint32_t binding,
                                           VkDescriptorType descriptor_type,
                                           uint32_t descriptor_count,
                                           VkShaderStageFlags stage_flags) {
  VkDescriptorSetLayoutBinding layout_binding = {
      binding, descriptor_type, descriptor_count, stage_flags, nullptr};
  bindings_.emplace_back(layout_binding);
}

DescriptorSetLayout &PipelineLayout::descriptorSetLayout(uint32_t id) {
  return descriptor_sets_[id];
}

PipelineLayout::PipelineLayout(const LogicalDevice &logical_device)
    : logical_device_(logical_device) {}

PipelineLayout::~PipelineLayout() {
  if (vk_pipeline_layout_ != VK_NULL_HANDLE)
    vkDestroyPipelineLayout(logical_device_.handle(), vk_pipeline_layout_,
                            nullptr);
}

VkPipelineLayout PipelineLayout::handle() {
  if (vk_pipeline_layout_ == VK_NULL_HANDLE) {
    std::vector<VkDescriptorSetLayout> layout_handles;
    for (auto &ds : descriptor_sets_)
      layout_handles.emplace_back(ds.handle());
    VkPipelineLayoutCreateInfo info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        layout_handles.size(),
        layout_handles.data(),
        vk_push_constant_ranges_.size(),
        vk_push_constant_ranges_.data()};
    VkResult result = vkCreatePipelineLayout(logical_device_.handle(), &info,
                                             nullptr, &vk_pipeline_layout_);
    CHECK_VULKAN(result);
    if (result != VK_SUCCESS)
      vk_pipeline_layout_ = VK_NULL_HANDLE;
  }
  return vk_pipeline_layout_;
}

uint32_t PipelineLayout::createLayoutSet(uint32_t id) {
  descriptor_sets_.emplace_back(logical_device_);
  return descriptor_sets_.size() - 1;
}

void PipelineLayout::addPushConstantRange(VkShaderStageFlags stage_flags,
                                          uint32_t offset, uint32_t size) {
  VkPushConstantRange pc = {stage_flags, offset, size};
  vk_push_constant_ranges_.emplace_back(pc);
}

DescriptorPool::DescriptorPool(const LogicalDevice &logical_device,
                               uint32_t max_sets)
    : max_sets_(max_sets), logical_device_(logical_device) {}

DescriptorPool::~DescriptorPool() {
  if (vk_descriptor_pool_ != VK_NULL_HANDLE)
    vkDestroyDescriptorPool(logical_device_.handle(), vk_descriptor_pool_,
                            nullptr);
}

void DescriptorPool::setPoolSize(VkDescriptorType type,
                                 uint32_t descriptor_count) {
  VkDescriptorPoolSize dps = {type, descriptor_count};
  pool_sizes_.emplace_back(dps);
}

VkDescriptorPool DescriptorPool::handle() {
  if (vk_descriptor_pool_ == VK_NULL_HANDLE) {
    VkDescriptorPoolCreateInfo info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        nullptr,
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        max_sets_,
        pool_sizes_.size(),
        pool_sizes_.data()};
    VkResult result = vkCreateDescriptorPool(logical_device_.handle(), &info,
                                             nullptr, &vk_descriptor_pool_);
    CHECK_VULKAN(result);
    if (result != VK_SUCCESS)
      vk_descriptor_pool_ = VK_NULL_HANDLE;
  }
  return vk_descriptor_pool_;
}

bool DescriptorPool::allocate(
    std::vector<DescriptorSetLayout> &descriptor_set_layouts,
    std::vector<VkDescriptorSet> &descriptor_sets) {
  std::vector<VkDescriptorSetLayout> dsls;
  for (auto &dsl : descriptor_set_layouts)
    dsls.emplace_back(dsl.handle());
  VkDescriptorSetAllocateInfo allocate_info = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
      vk_descriptor_pool_, dsls.size(), dsls.data()};
  VkDescriptorSet *descriptor_sets_ptr = nullptr;
  R_CHECK_VULKAN(vkAllocateDescriptorSets(logical_device_.handle(),
                                          &allocate_info, descriptor_sets_ptr));
  descriptor_sets.clear();
  for (size_t i = 0; i < descriptor_set_layouts.size(); ++i)
    descriptor_sets.emplace_back(descriptor_sets_ptr[i]);
  return true;
}

bool DescriptorPool::free(const std::vector<VkDescriptorSet> &descriptor_sets) {
  R_CHECK_VULKAN(
      vkFreeDescriptorSets(logical_device_.handle(), vk_descriptor_pool_,
                           descriptor_sets.size(), descriptor_sets.data()));
  return true;
}

bool DescriptorPool::reset() {
  R_CHECK_VULKAN(
      vkResetDescriptorPool(logical_device_.handle(), vk_descriptor_pool_, 0));
  return true;
}

Pipeline::Pipeline(const LogicalDevice &logical_device)
    : logical_device_(logical_device) {}

Pipeline::~Pipeline() {
  if (vk_pipeline_ != VK_NULL_HANDLE)
    vkDestroyPipeline(logical_device_.handle(), vk_pipeline_, nullptr);
}

bool Pipeline::saveCache(const std::string &path) {
  size_t cache_data_size;
  // Determine the size of the cache data
  R_CHECK_VULKAN(vkGetPipelineCacheData(
      logical_device_.handle(), vk_pipeline_cache_, &cache_data_size, nullptr));
  VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;
  if (cache_data_size != 0) {
    void *data = new char[cache_data_size];
    if (data) {
      // Retrieve the actual data from the cache
      result = vkGetPipelineCacheData(
          logical_device_.handle(), vk_pipeline_cache_, &cache_data_size, data);
      CHECK_VULKAN(result);
      if (result == VK_SUCCESS) {
        std::ofstream ofile(path, std::ios::binary);
        if (ofile.good()) {
          ofile.write((char *)&data, cache_data_size);
          ofile.close();
        }
      }
      delete[] data;
    }
  }
  return result == VK_SUCCESS;
}

VkPipeline Pipeline::handle() const { return vk_pipeline_; }

GraphicsPipeline::GraphicsPipeline(const LogicalDevice &logical_device,
                                   PipelineLayout &layout,
                                   RenderPass &renderpass, uint32_t subpass,
                                   VkPipelineCreateFlags flags,
                                   GraphicsPipeline *base_pipeline,
                                   uint32_t base_pipeline_index)
    : Pipeline(logical_device), flags_(flags) {}

} // namespace vk

} // namespace circe
