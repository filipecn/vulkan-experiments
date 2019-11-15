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

#include "vk_renderpass.h"
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
  DescriptorSetLayout(const LogicalDevice *logical_device);
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
  const LogicalDevice *logical_device_ = nullptr;
  VkDescriptorSetLayout vk_descriptor_set_layout_ = VK_NULL_HANDLE;
  std::vector<VkDescriptorSetLayoutBinding> bindings_;
};

/// Groups descriptor sets to be used by a pipeline.
class PipelineLayout {
public:
  ///\brief Construct a new Pipeline Layout object
  ///
  ///\param logical_device **[in]**
  PipelineLayout(const LogicalDevice *logical_device);
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
  const LogicalDevice *logical_device_ = nullptr;
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

/// Holds information about a shader
class PipelineShaderStage {
public:
  ///\param stage **[in]**
  ///\param module **[in]**
  ///\param name **[in]** entry point in the shader module
  ///\param specialization_info_data **[in]**
  ///\param specialization_info_data_size **[in]**
  PipelineShaderStage(VkShaderStageFlagBits stage, const ShaderModule &module,
                      std::string name, const void *specialization_info_data,
                      size_t specialization_info_data_size);
  ///\param constant_id **[in]** match the constant ID used in the shader module
  ///(see constant_id layout qualifier in GLSL)
  ///\param offset **[in]** offset of the raw data (passed in constructor)
  ///\param size **[in]** size of the raw data (passed in constructor)
  void addSpecializationMapEntry(uint32_t constant_id, uint32_t offset,
                                 size_t size);
  VkShaderStageFlagBits stage() const;
  VkShaderModule module() const;
  const std::string &name() const;
  const VkSpecializationInfo *specializationInfo() const;

private:
  VkSpecializationInfo specialization_info_;
  VkShaderStageFlagBits stage_;
  VkShaderModule module_;
  std::string name_;
  std::vector<VkSpecializationMapEntry> map_entries_;
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
  ///\brief Construct a new Pipeline object
  ///
  ///\param logical_device **[in]**
  Pipeline(const LogicalDevice *logical_device);
  virtual ~Pipeline();
  ///\brief
  ///
  ///\param stage **[in]**
  void addShaderStage(const PipelineShaderStage &stage);
  ///\brief
  ///
  ///\param path **[in]**
  ///\return bool
  bool saveCache(const std::string &path);
  VkPipeline handle() const;
  VkPipelineCache cache() const;

protected:
  const LogicalDevice *logical_device_ = nullptr;
  VkPipeline vk_pipeline_ = VK_NULL_HANDLE;
  VkPipelineCache vk_pipeline_cache_ = VK_NULL_HANDLE;
  std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos_;
};

class ComputePipeline : public Pipeline {
public:
  ////\brief Construct a new Compute Pipeline object
  ///
  ///\param logical_device **[in]**
  ///\param stage **[in]**
  ///\param layout **[in]**
  ///\param cache **[in]**
  ///\param base_pipeline **[in]**
  ///\param base_pipeline_index **[in]**
  ComputePipeline(const LogicalDevice *logical_device,
                  const PipelineShaderStage &stage, PipelineLayout &layout,
                  Pipeline *cache = nullptr,
                  ComputePipeline *base_pipeline = nullptr,
                  uint32_t base_pipeline_index = 0);
};

class GraphicsPipeline : public Pipeline {
public:
  class VertexInputState {
  public:
    VertexInputState();
    ~VertexInputState() = default;
    ///\brief
    ///
    ///\param binding **[in]**
    ///\param stride **[in]**
    ///\param input_rate **[in]**
    void addBindingDescription(uint32_t binding, uint32_t stride,
                               VkVertexInputRate input_rate);
    ///\brief
    ///
    ///\param location **[in]**
    ///\param binding **[in]**
    ///\param format **[in]**
    ///\param offset **[in]**
    void addAttributeDescription(uint32_t location, uint32_t binding,
                                 VkFormat format, uint32_t offset);

    const VkPipelineVertexInputStateCreateInfo *info() const;

  private:
    VkPipelineVertexInputStateCreateInfo info_;
    std::vector<VkVertexInputBindingDescription> binding_descriptions_;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions_;
  };
  class ViewportState {
  public:
    ViewportState();
    ~ViewportState() = default;
    ///\brief
    ///
    ///\param x **[in]**
    ///\param y **[in]**
    ///\param width **[in]**
    ///\param height **[in]**
    ///\param min_depth **[in]**
    ///\param max_depth **[in]**
    void addViewport(float x, float y, float width, float height,
                     float min_depth, float max_depth);
    ///\brief
    ///
    ///\param x **[in]**
    ///\param y **[in]**
    ///\param width **[in]**
    ///\param height **[in]**
    void addScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);
    const VkPipelineViewportStateCreateInfo *info() const;

  private:
    VkPipelineViewportStateCreateInfo info_;
    std::vector<VkViewport> viewports_;
    std::vector<VkRect2D> scissors_;
  };
  class ColorBlendState {
  public:
    ColorBlendState();
    ~ColorBlendState() = default;

    void setLogicOp(VkLogicOp logic_op);
    ///\brief
    ///
    ///\param blend_enable **[in]**
    ///\param src_color_blend_factor **[in]**
    ///\param dst_color_blend_factor **[in]**
    ///\param color_blend_op **[in]**
    ///\param src_alpha_blend_factor **[in]**
    ///\param dst_alpha_blend_factor **[in]**
    ///\param alpha_blend_op **[in]**
    ///\param color_write_mask **[in]**
    void addAttachmentState(VkBool32 blend_enable,
                            VkBlendFactor src_color_blend_factor,
                            VkBlendFactor dst_color_blend_factor,
                            VkBlendOp color_blend_op,
                            VkBlendFactor src_alpha_blend_factor,
                            VkBlendFactor dst_alpha_blend_factor,
                            VkBlendOp alpha_blend_op,
                            VkColorComponentFlags color_write_mask);
    void setBlendConstants(float r, float g, float b, float a);
    const VkPipelineColorBlendStateCreateInfo *info() const;

  private:
    VkPipelineColorBlendStateCreateInfo info_;
    std::vector<VkPipelineColorBlendAttachmentState> attachments_;
  };
  ///\brief Construct a new Graphics Pipeline object
  ///
  ///\param logical_device **[in]**
  ///\param layout **[in]**
  ///\param renderpass **[in]**
  ///\param subpass **[in]**
  ///\param flags **[in]**
  ///\param base_pipeline **[in]**
  ///\param base_pipeline_index **[in]**
  GraphicsPipeline(const LogicalDevice *logical_device, PipelineLayout &layout,
                   RenderPass &renderpass, uint32_t subpass,
                   VkPipelineCreateFlags flags = 0,
                   GraphicsPipeline *base_pipeline = nullptr,
                   uint32_t base_pipeline_index = 0);

  VkPipeline handle();

  ///\brief Set the Input State object
  ///
  ///\param topology **[in]**
  ///\param primitive_restart_enable **[in]**
  void setInputState(VkPrimitiveTopology topology,
                     VkBool32 primitive_restart_enable = VK_FALSE);
  ///\brief Set the Tesselation State object
  ///
  ///\param patch_control_points **[in]**
  void setTesselationState(uint32_t patch_control_points);
  ///\brief Set the Rasterization State object
  ///
  ///\param depth_clamp_enable **[in]**
  ///\param rasterizer_discard_enable **[in]**
  ///\param polygon_mode **[in]**
  ///\param cull_mode **[in]**
  ///\param front_face **[in]**
  ///\param depth_bias_enable **[in]**
  ///\param depth_bias_constant_factor **[in]**
  ///\param depth_bias_clamp **[in]**
  ///\param depth_bias_slope_factor **[in]**
  ///\param line_width **[in]**
  void setRasterizationState(VkBool32 depth_clamp_enable,
                             VkBool32 rasterizer_discard_enable,
                             VkPolygonMode polygon_mode,
                             VkCullModeFlags cull_mode, VkFrontFace front_face,
                             VkBool32 depth_bias_enable,
                             float depth_bias_constant_factor,
                             float depth_bias_clamp,
                             float depth_bias_slope_factor, float line_width);
  ///\brief Set the Multisample State object
  ///
  ///\param rasterization_samples **[in]**
  ///\param sample_shading_enable **[in]**
  ///\param min_sample_shading **[in]**
  ///\param sample_mask **[in]**
  ///\param alpha_to_coverage_enable **[in]**
  ///\param alpha_to_one_enable **[in]**
  void setMultisampleState(VkSampleCountFlagBits rasterization_samples,
                           VkBool32 sample_shading_enable,
                           float min_sample_shading,
                           const std::vector<VkSampleMask> &sample_mask,
                           VkBool32 alpha_to_coverage_enable,
                           VkBool32 alpha_to_one_enable);
  ///\brief Set the Depth Stencil State object
  ///
  ///\param depth_test_enable **[in]**
  ///\param depth_write_enable **[in]**
  ///\param depth_compare_op **[in]**
  ///\param depth_bounds_test_enable **[in]**
  ///\param stencil_test_enable **[in]**
  ///\param front **[in]**
  ///\param back **[in]**
  ///\param min_depth_bounds **[in]**
  ///\param max_depth_bounds **[in]**
  void setDepthStencilState(VkBool32 depth_test_enable,
                            VkBool32 depth_write_enable,
                            VkCompareOp depth_compare_op,
                            VkBool32 depth_bounds_test_enable,
                            VkBool32 stencil_test_enable,
                            VkStencilOpState front, VkStencilOpState back,
                            float min_depth_bounds, float max_depth_bounds);
  ///\brief
  ///
  ///\param dynamic_state **[in]**
  void addDynamicState(VkDynamicState dynamic_state);

  VertexInputState vertex_input_state;
  ViewportState viewport_state;
  ColorBlendState color_blend_state;

private:
  VkPipelineCreateFlags flags_ = 0;
  std::unique_ptr<VkPipelineInputAssemblyStateCreateInfo> input_assembly_state_;
  std::unique_ptr<VkPipelineTessellationStateCreateInfo> tesselation_state_;
  std::unique_ptr<VkPipelineRasterizationStateCreateInfo> rasterization_state_;
  std::unique_ptr<VkPipelineMultisampleStateCreateInfo> multisample_state_;
  std::unique_ptr<VkPipelineDepthStencilStateCreateInfo> depth_stencil_state_;
  std::unique_ptr<VkPipelineDynamicStateCreateInfo> dynamic_state_;
  std::vector<VkDynamicState> dynamic_states_;
  VkGraphicsPipelineCreateInfo info_ = {};
};

} // namespace vk

} // namespace circe

#endif // CIRCE_VULKAN_PIPELINE_H