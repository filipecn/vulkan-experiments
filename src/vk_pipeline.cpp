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

DescriptorSetLayout::DescriptorSetLayout(const LogicalDevice *logical_device)
    : logical_device_(logical_device) {}

DescriptorSetLayout::~DescriptorSetLayout() {
  if (vk_descriptor_set_layout_ != VK_NULL_HANDLE)
    vkDestroyDescriptorSetLayout(logical_device_->handle(),
                                 vk_descriptor_set_layout_, nullptr);
}

VkDescriptorSetLayout DescriptorSetLayout::handle() {
  if (vk_descriptor_set_layout_) {
    VkDescriptorSetLayoutCreateInfo info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
        bindings_.size(), (bindings_.size()) ? bindings_.data() : nullptr};
    VkResult result = vkCreateDescriptorSetLayout(
        logical_device_->handle(), &info, nullptr, &vk_descriptor_set_layout_);
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

PipelineLayout::PipelineLayout(const LogicalDevice *logical_device)
    : logical_device_(logical_device) {}

PipelineLayout::~PipelineLayout() {
  if (vk_pipeline_layout_ != VK_NULL_HANDLE)
    vkDestroyPipelineLayout(logical_device_->handle(), vk_pipeline_layout_,
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
    VkResult result = vkCreatePipelineLayout(logical_device_->handle(), &info,
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

PipelineShaderStage::PipelineShaderStage(VkShaderStageFlagBits stage,
                                         const ShaderModule &module,
                                         std::string name,
                                         const void *specialization_info_data,
                                         size_t specialization_info_data_size)
    : stage_(stage), module_(module.handle()), name_(name) {
  specialization_info_.pData = specialization_info_data;
  specialization_info_.dataSize = specialization_info_data_size;
  specialization_info_.mapEntryCount = 0;
  specialization_info_.pMapEntries = nullptr;
}

void PipelineShaderStage::addSpecializationMapEntry(uint32_t constant_id,
                                                    uint32_t offset,
                                                    size_t size) {
  VkSpecializationMapEntry map_entry = {constant_id, offset, size};
  map_entries_.emplace_back(map_entry);
  specialization_info_.mapEntryCount = map_entries_.size();
  specialization_info_.pMapEntries = map_entries_.data();
}

VkShaderStageFlagBits PipelineShaderStage::stage() const { return stage_; }

VkShaderModule PipelineShaderStage::module() const { return module_; }

const std::string &PipelineShaderStage::name() const { return name_; }

const VkSpecializationInfo *PipelineShaderStage::specializationInfo() const {
  return &specialization_info_;
}

Pipeline::Pipeline(const LogicalDevice *logical_device)
    : logical_device_(logical_device) {}

Pipeline::~Pipeline() {
  if (vk_pipeline_ != VK_NULL_HANDLE)
    vkDestroyPipeline(logical_device_->handle(), vk_pipeline_, nullptr);
}

bool Pipeline::saveCache(const std::string &path) {
  size_t cache_data_size;
  // Determine the size of the cache data
  R_CHECK_VULKAN(vkGetPipelineCacheData(logical_device_->handle(),
                                        vk_pipeline_cache_, &cache_data_size,
                                        nullptr));
  VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;
  if (cache_data_size != 0) {
    void *data = new char[cache_data_size];
    if (data) {
      // Retrieve the actual data from the cache
      result =
          vkGetPipelineCacheData(logical_device_->handle(), vk_pipeline_cache_,
                                 &cache_data_size, data);
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

VkPipelineCache Pipeline::cache() const { return vk_pipeline_cache_; }

VkPipeline Pipeline::handle() const { return vk_pipeline_; }

void Pipeline::addShaderStage(const PipelineShaderStage &stage) {
  VkPipelineShaderStageCreateInfo info = {
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      nullptr,
      0,
      stage.stage(),
      stage.module(),
      stage.name().c_str(),
      stage.specializationInfo()};
  shader_stage_infos_.emplace_back(info);
}

ComputePipeline::ComputePipeline(const LogicalDevice *logical_device,
                                 const PipelineShaderStage &stage,
                                 PipelineLayout &layout, Pipeline *cache,
                                 ComputePipeline *base_pipeline,
                                 uint32_t base_pipeline_index)
    : Pipeline(logical_device) {
  VkComputePipelineCreateInfo info = {
      VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      nullptr,
      0,
      this->shader_stage_infos_[0],
      layout.handle(),
      (base_pipeline ? base_pipeline->handle() : VK_NULL_HANDLE),
      base_pipeline_index};
  vkCreateComputePipelines(this->logical_device_->handle(),
                           (cache ? cache->cache() : VK_NULL_HANDLE), 1, &info,
                           nullptr, &this->vk_pipeline_);
}

GraphicsPipeline::VertexInputState::VertexInputState() {
  info_.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  info_.pNext = nullptr;
  info_.flags = 0;
  info_.pVertexAttributeDescriptions = nullptr;
  info_.pVertexBindingDescriptions = nullptr;
  info_.vertexAttributeDescriptionCount = 0;
  info_.vertexBindingDescriptionCount = 0;
}

void GraphicsPipeline::VertexInputState::addBindingDescription(
    uint32_t binding, uint32_t stride, VkVertexInputRate input_rate) {
  VkVertexInputBindingDescription bd = {binding, stride, input_rate};
  binding_descriptions_.emplace_back(bd);
  info_.vertexBindingDescriptionCount = binding_descriptions_.size();
  info_.pVertexBindingDescriptions = binding_descriptions_.data();
}

void GraphicsPipeline::VertexInputState::addAttributeDescription(
    uint32_t location, uint32_t binding, VkFormat format, uint32_t offset) {
  VkVertexInputAttributeDescription ad = {location, binding, format, offset};
  attribute_descriptions_.emplace_back(ad);
  info_.vertexAttributeDescriptionCount = attribute_descriptions_.size();
  info_.pVertexAttributeDescriptions = attribute_descriptions_.data();
}

const VkPipelineVertexInputStateCreateInfo *
GraphicsPipeline::VertexInputState::info() const {
  return &info_;
}

GraphicsPipeline::ViewportState::ViewportState() {
  info_.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  info_.pNext = nullptr;
  info_.flags = 0;
  info_.scissorCount = info_.viewportCount = 0;
  info_.pScissors = nullptr;
  info_.pViewports = nullptr;
}

void GraphicsPipeline::ViewportState::addViewport(float x, float y, float width,
                                                  float height, float min_depth,
                                                  float max_depth) {
  VkViewport viewport = {x, y, width, height, min_depth, max_depth};
  viewports_.emplace_back(viewport);
  info_.pViewports = viewports_.data();
  info_.viewportCount = viewports_.size();
}

void GraphicsPipeline::ViewportState::addScissor(int32_t x, int32_t y,
                                                 uint32_t width,
                                                 uint32_t height) {
  VkRect2D scissor = {{x, y}, {width, height}};
  info_.pScissors = scissors_.data();
  info_.scissorCount = scissors_.size();
}

const VkPipelineViewportStateCreateInfo *
GraphicsPipeline::ViewportState::info() const {
  return &info_;
}

GraphicsPipeline::ColorBlendState::ColorBlendState() {
  info_.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  info_.pNext = nullptr;
  info_.flags = 0;
  info_.logicOpEnable = VK_FALSE;
  info_.attachmentCount = 0;
  info_.pAttachments = nullptr;
}

void GraphicsPipeline::ColorBlendState::setLogicOp(VkLogicOp logic_op) {
  info_.logicOpEnable = VK_TRUE;
  info_.logicOp = logic_op;
}

void GraphicsPipeline::ColorBlendState::addAttachmentState(
    VkBool32 blend_enable, VkBlendFactor src_color_blend_factor,
    VkBlendFactor dst_color_blend_factor, VkBlendOp color_blend_op,
    VkBlendFactor src_alpha_blend_factor, VkBlendFactor dst_alpha_blend_factor,
    VkBlendOp alpha_blend_op, VkColorComponentFlags color_write_mask) {
  VkPipelineColorBlendAttachmentState as = {
      blend_enable,   src_color_blend_factor, dst_color_blend_factor,
      color_blend_op, src_alpha_blend_factor, dst_alpha_blend_factor,
      alpha_blend_op, color_write_mask};
  attachments_.emplace_back(as);
  info_.pAttachments = attachments_.data();
  info_.attachmentCount = attachments_.size();
}

void GraphicsPipeline::ColorBlendState::setBlendConstants(float r, float g,
                                                          float b, float a) {
  info_.blendConstants[0] = r;
  info_.blendConstants[1] = g;
  info_.blendConstants[2] = b;
  info_.blendConstants[3] = a;
}

const VkPipelineColorBlendStateCreateInfo *
GraphicsPipeline::ColorBlendState::info() const {
  return &info_;
}

GraphicsPipeline::GraphicsPipeline(const LogicalDevice *logical_device,
                                   PipelineLayout &layout,
                                   RenderPass &renderpass, uint32_t subpass,
                                   VkPipelineCreateFlags flags,
                                   GraphicsPipeline *base_pipeline,
                                   uint32_t base_pipeline_index)
    : Pipeline(logical_device), flags_(flags) {
  info_.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  info_.flags = flags;
  info_.layout = layout.handle();
  info_.renderPass = renderpass.handle();
  info_.subpass = subpass;
  info_.basePipelineHandle =
      base_pipeline ? base_pipeline->handle() : VK_NULL_HANDLE;
  info_.basePipelineIndex = base_pipeline_index;
}

VkPipeline GraphicsPipeline::handle() {
  if (this->vk_pipeline_ == VK_NULL_HANDLE) {
    info_.stageCount = this->shader_stage_infos_.size();
    info_.pStages = this->shader_stage_infos_.data();
    info_.pVertexInputState = vertex_input_state.info();
    info_.pInputAssemblyState = input_assembly_state_.get();
    info_.pTessellationState = tesselation_state_.get();
    info_.pViewportState = viewport_state.info();
    info_.pRasterizationState = rasterization_state_.get();
    info_.pMultisampleState = multisample_state_.get();
    info_.pDepthStencilState = depth_stencil_state_.get();
    info_.pColorBlendState = color_blend_state.info();
    VkPipelineDynamicStateCreateInfo d_info = {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, // VkStructureType
        nullptr,                                              // pNext
        0,                      // VkPipelineDynamicStateCreateFlags
        dynamic_states_.size(), // dynamicStateCount;
        (dynamic_states_.size()) ? dynamic_states_.data() : nullptr};
    info_.pDynamicState = &d_info;

    VkResult result = vkCreateGraphicsPipelines(this->logical_device_->handle(),
                                                VK_NULL_HANDLE, 1, &info_,
                                                nullptr, &this->vk_pipeline_);
    CHECK_VULKAN(result);
  }
  return this->vk_pipeline_;
} // namespace vk

void GraphicsPipeline::setInputState(VkPrimitiveTopology topology,
                                     VkBool32 primitive_restart_enable) {
  input_assembly_state_ =
      std::make_unique<VkPipelineInputAssemblyStateCreateInfo>();
  input_assembly_state_->sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly_state_->pNext = nullptr;
  input_assembly_state_->flags = 0;
  input_assembly_state_->topology = topology;
  input_assembly_state_->primitiveRestartEnable = primitive_restart_enable;
}

void GraphicsPipeline::setTesselationState(uint32_t patch_control_points) {
  tesselation_state_ =
      std::make_unique<VkPipelineTessellationStateCreateInfo>();
  tesselation_state_->sType =
      VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
  tesselation_state_->pNext = nullptr;
  tesselation_state_->flags = 0;
  tesselation_state_->patchControlPoints = patch_control_points;
}

void GraphicsPipeline::setRasterizationState(
    VkBool32 depth_clamp_enable, VkBool32 rasterizer_discard_enable,
    VkPolygonMode polygon_mode, VkCullModeFlags cull_mode,
    VkFrontFace front_face, VkBool32 depth_bias_enable,
    float depth_bias_constant_factor, float depth_bias_clamp,
    float depth_bias_slope_factor, float line_width) {
  rasterization_state_ =
      std::make_unique<VkPipelineRasterizationStateCreateInfo>();
  rasterization_state_->sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterization_state_->pNext = nullptr;
  rasterization_state_->flags = 0;
  rasterization_state_->depthClampEnable = depth_clamp_enable;
  rasterization_state_->rasterizerDiscardEnable = rasterizer_discard_enable;
  rasterization_state_->polygonMode = polygon_mode;
  rasterization_state_->cullMode = cull_mode;
  rasterization_state_->frontFace = front_face;
  rasterization_state_->depthBiasEnable = depth_bias_enable;
  rasterization_state_->depthBiasConstantFactor = depth_bias_constant_factor;
  rasterization_state_->depthBiasClamp = depth_bias_clamp;
  rasterization_state_->depthBiasSlopeFactor = depth_bias_slope_factor;
  rasterization_state_->lineWidth = line_width;
}

void GraphicsPipeline::setMultisampleState(
    VkSampleCountFlagBits rasterization_samples, VkBool32 sample_shading_enable,
    float min_sample_shading, const std::vector<VkSampleMask> &sample_mask,
    VkBool32 alpha_to_coverage_enable, VkBool32 alpha_to_one_enable) {
  multisample_state_ = std::make_unique<VkPipelineMultisampleStateCreateInfo>();
  multisample_state_->sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample_state_->pNext = nullptr;
  multisample_state_->flags = 0;
  multisample_state_->rasterizationSamples = rasterization_samples;
  multisample_state_->sampleShadingEnable = sample_shading_enable;
  multisample_state_->minSampleShading = min_sample_shading;
  multisample_state_->pSampleMask = sample_mask.data();
  multisample_state_->alphaToCoverageEnable = alpha_to_coverage_enable;
  multisample_state_->alphaToOneEnable = alpha_to_one_enable;
}

void GraphicsPipeline::setDepthStencilState(
    VkBool32 depth_test_enable, VkBool32 depth_write_enable,
    VkCompareOp depth_compare_op, VkBool32 depth_bounds_test_enable,
    VkBool32 stencil_test_enable, VkStencilOpState front, VkStencilOpState back,
    float min_depth_bounds, float max_depth_bounds) {
  depth_stencil_state_ =
      std::make_unique<VkPipelineDepthStencilStateCreateInfo>();
  depth_stencil_state_->sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_stencil_state_->pNext = nullptr;
  depth_stencil_state_->flags = 0;
  depth_stencil_state_->depthTestEnable = depth_test_enable;
  depth_stencil_state_->depthWriteEnable = depth_write_enable;
  depth_stencil_state_->depthCompareOp = depth_compare_op;
  depth_stencil_state_->depthBoundsTestEnable = depth_bounds_test_enable;
  depth_stencil_state_->stencilTestEnable = stencil_test_enable;
  depth_stencil_state_->front = front;
  depth_stencil_state_->back = back;
  depth_stencil_state_->minDepthBounds = min_depth_bounds;
  depth_stencil_state_->maxDepthBounds = max_depth_bounds;
}

void GraphicsPipeline::addDynamicState(VkDynamicState dynamic_state) {
  if (!dynamic_state_) {
    dynamic_state_ = std::make_unique<VkPipelineDynamicStateCreateInfo>();
    dynamic_state_->sType =
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_->pNext = nullptr;
    dynamic_state_->flags = 0;
  }

  dynamic_states_.emplace_back(dynamic_state);
  dynamic_state_->dynamicStateCount = dynamic_states_.size();
  dynamic_state_->pDynamicStates = dynamic_states_.data();
}

} // namespace vk

} // namespace circe
