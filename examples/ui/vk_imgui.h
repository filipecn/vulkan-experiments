/// Copyright (c) 2020, FilipeCN.
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
///\file vk_imgui.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2020-04-09
///
///\brief

#include <core/vk.h>
#include <imgui.h>
#include <ponos/ponos.h>

// ImGUI wrapper for vulkan
// extracted from:
// https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanUIOverlay.h
class ImGUI {
public:
  // UI params are set via push constants
  struct PushConstBlock {
    ponos::vec2 scale;
    ponos::vec2 translate;
  };

  ///\param app **[in]**
  ImGUI(circe::vk::App *app);
  // Initialize styles, keys, etc.
  void init(ponos::size2 size);
  // Initialize all vk resources used by the ui
  void initResources(circe::vk::RenderPass *render_pass, VkQueue copy_queue,
                     uint32_t queue_family_index);
  // Update vertex and index buffer containing the imGui elements when required
  void updateBuffers();
  // Draw current imGui frame into a command buffer
  void drawFrame(circe::vk::CommandBuffer &command_buffer);

private:
  circe::vk::App *app_;
  std::unique_ptr<circe::vk::Texture> font_texture_;
  std::shared_ptr<circe::vk::Image::View> font_texture_view_;
  std::unique_ptr<circe::vk::Sampler> font_texture_sampler_;
  std::unique_ptr<circe::vk::DescriptorPool> descriptor_pool_;
  std::vector<VkDescriptorSet> descriptor_sets_;
  std::unique_ptr<circe::vk::PipelineLayout> pipeline_layout_;
  std::unique_ptr<circe::vk::GraphicsPipeline> pipeline_;
  PushConstBlock push_const_block_;
  // mesh
  std::unique_ptr<circe::vk::Buffer> vertex_buffer_, index_buffer_;
  std::unique_ptr<circe::vk::DeviceMemory> vertex_buffer_memory_,
      index_buffer_memory_;
  i32 vertex_count_{0}, index_count_{0};
  std::shared_ptr<circe::vk::ShaderModule> frag_shader_module_,
      vert_shader_module_;
  std::shared_ptr<circe::vk::PipelineShaderStage> frag_shader_stage_info_,
      vert_shader_stage_info_;
};