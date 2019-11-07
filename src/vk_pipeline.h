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
///\file vk_pipeline.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-11-01
///
///\brief

#ifndef CIRCE_VULKAN_PIPELINE_H
#define CIRCE_VULKAN_PIPELINE_H

#include "vk_shader_module.h"

namespace circe {

namespace vk {

/// A descriptor set is a set of resources that are bound into the pipeline
/// as a group. Multiple sets can be bound to a pipeline at a time. Each set
/// has layout, which describes the order and types of resources in the set.
class DescriptorSetLayout {
public:
  ///\brief Construct a new Descriptor Set object
  ///
  ///\param logical_device **[in]**
  DescriptorSetLayout(const LogicalDevice &logical_device);
  ~DescriptorSetLayout();

  ///\brief
  ///
  ///\return VkDescriptorSetLayout
  VkDescriptorSetLayout handle();
  ///\brief
  /// Resources are bound to binding points in the descriptor set. Each binding
  /// is described by the parameters of this method.
  ///\param binding **[in]** Each resource accessible to a shader is given a
  /// binding number.
  ///\param descriptor_type **[in]**
  ///\param descriptor_count **[in]**
  ///\param stage_flags **[in]**
  void addLayoutBinding(uint32_t binding, VkDescriptorType descriptor_type,
                        uint32_t descriptor_count,
                        VkShaderStageFlags stage_flags);

private:
  const LogicalDevice &logical_device_;
  VkDescriptorSetLayout vk_descriptor_set_layout_ = VK_NULL_HANDLE;
  std::vector<VkDescriptorSetLayoutBinding> bindings_;
};

/// Groups descriptor sets to be used by a pipeline.
class PipelineLayout {
public:
  ///\brief Construct a new Pipeline Layout object
  ///
  ///\param logical_device **[in]**
  PipelineLayout(const LogicalDevice &logical_device);
  ~PipelineLayout();
  VkPipelineLayout handle();
  ///\brief Create a Layout Set object
  ///
  ///\param id **[in]**
  ///\return uint32_t
  uint32_t createLayoutSet(uint32_t id);
  ///\brief
  ///
  ///\param id **[in]**
  ///\return DescriptorSet&
  DescriptorSetLayout &descriptorSetLayout(uint32_t id);
  ///\brief
  /// A push constant is a uniform variable in a shader that can be used just
  /// like a member of a uniform block, but has faster access.
  ///\param stage_flags **[in]**
  ///\param offset **[in]**
  ///\param size **[in]**
  void addPushConstantRange(VkShaderStageFlags stage_flags, uint32_t offset,
                            uint32_t size);

private:
  const LogicalDevice &logical_device_;
  VkPipelineLayout vk_pipeline_layout_ = VK_NULL_HANDLE;
  std::vector<DescriptorSetLayout> descriptor_sets_;
  std::vector<VkPushConstantRange> vk_push_constant_ranges_;
};

/// Resources are represented by descriptors anda are bound to the pipeline by
/// first bibnding their descriptors into sets and then binding those descriptor
/// sets to then pipeline. The descriptors are allocated from pools.
class DescriptorPool {
public:
  ///\brief Construct a new Descriptor Pool object
  ///
  ///\param logical_device **[in]**
  ///\param max_sets **[in]** specifies the maximum total number of sets that
  /// may be allocated from the pool
  DescriptorPool(const LogicalDevice &logical_device, uint32_t max_sets);
  /// Completely free all the resources associated with the descriptor pool, it
  /// is not necessary to explicitly free the descriptor sets allocated before
  /// destroying the pool
  ~DescriptorPool();
  /// \brief Add support to allocation for a specific type of resource
  /// descriptor
  ///\param type **[in]** type of resource
  ///\param descriptor_count **[in]** the number of descriptors of that type
  /// that can be stored in the pool
  void setPoolSize(VkDescriptorType type, uint32_t descriptor_count);
  VkDescriptorPool handle();
  ///\brief Allocates blocks of descriptor from the pool
  ///\param descriptor_set_layouts **[in]**
  ///\param descriptor_sets **[out]**
  ///\return bool
  bool allocate(std::vector<DescriptorSetLayout> &descriptor_set_layouts,
                std::vector<VkDescriptorSet> &descriptor_sets);
  ///\brief Returns a list of descriptor sets to the pool by freeing them.
  /// When descriptor sets are freed, their resources are returned to the pool
  /// and may be allocated to a new set in the future
  ///\param descriptor_sets **[in]**
  ///\return bool
  bool free(const std::vector<VkDescriptorSet> &descriptor_sets);
  ///\brief Recycles all resources from all sets allocated
  /// With this command, it is not necessary to explicitly specify every set
  /// allocated from the pool
  ///\return bool
  bool reset();

private:
  uint32_t max_sets_;
  const LogicalDevice &logical_device_;
  VkDescriptorPool vk_descriptor_pool_ = VK_NULL_HANDLE;
  std::vector<VkDescriptorPoolSize> pool_sizes_;
};

// The operations recorded in command buffers are processed by the hardware in
// a pipeline. Pipeline objects control the way in which computations are
// performed. Different from OpenGL though, the whole pipeline state is stored
// in a single object (In OpenGL we have a state machine where we can activate
// pieces separately and switch between resources during execution). In
// Vulkan, each configuration will require its own pipeline object (for
// example, if we want to switch between shaders, we need to prepare the whole
// pipeline for the new set of shaders).
// The computations executed inside the pipeline are performed by shaders.
// Shaders are represented by Shader Modules and must be provided to Vulkan as
// SPIR-V assembly code. A single module may contain code for multiple shader
// stages.
// The interface between shader stages and shader resources is specified
// through pipeline layouts (for example, the same address needs to be used in
// shaders).
// There are two types of pipelines:
// - Graphics pipelines
//    Are used for drawing when binded to the command buffer before recording
//    a drawing command. Can be bounded only inside render passes.
// - Compute pipelines
//    Consisted of a single compute shader stage, compute pipelines are used
//    to perform mathematical operations.

class Pipeline {
public:
  Pipeline(const LogicalDevice &logical_device);
  virtual ~Pipeline();

  bool saveCache(const std::string &path);
  VkPipeline handle() const;

protected:
  const LogicalDevice &logical_device_;
  VkPipeline vk_pipeline_ = VK_NULL_HANDLE;
  VkPipelineCache vk_pipeline_cache_ = VK_NULL_HANDLE;
};

class ComputePipeline : public Pipeline {};
class GraphicsPipeline : public Pipeline {};

} // namespace vk

} // namespace circe

#endif // CIRCE_VULKAN_PIPELINE_H