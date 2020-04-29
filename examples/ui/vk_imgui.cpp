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
///\file vk_imgui.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2020-04-09
///
///\brief

#include "vk_imgui.h"
#include <array>

using namespace circe::vk;

ImGUI::ImGUI(App *app) : app_(app) {
  ImGui::CreateContext();
  // setup shaders
  std::string path(SHADERS_PATH);
  // fragment shader
  frag_shader_module_ = std::make_shared<ShaderModule>(
      app_->logicalDevice(), path + "/shaders/imgui/ui.frag.spv");
  frag_shader_stage_info_ = std::make_shared<PipelineShaderStage>(
      VK_SHADER_STAGE_FRAGMENT_BIT, *(frag_shader_module_.get()), "main",
      nullptr, 0);
  // vertex shader
  vert_shader_module_ = std::make_shared<ShaderModule>(
      app_->logicalDevice(), path + "/shaders/imgui/ui.vert.spv");
  vert_shader_stage_info_ = std::make_shared<PipelineShaderStage>(
      VK_SHADER_STAGE_VERTEX_BIT, *(vert_shader_module_.get()), "main", nullptr,
      0);
}

void ImGUI::init(ponos::size2 size) {
  // Color scheme
  ImGuiStyle &style = ImGui::GetStyle();
  style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
  style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
  style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
  style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
  // Dimensions
  ImGuiIO &io = ImGui::GetIO();
  io.DisplaySize = ImVec2(size.width, size.height);
  io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
}

void ImGUI::initResources(RenderPass *render_pass, VkQueue copy_queue,
                          uint32_t queue_family_index) {
  ImGuiIO &io = ImGui::GetIO();

  // Create font texture
  unsigned char *font_data;
  int tex_width, tex_height;
  io.Fonts->GetTexDataAsRGBA32(&font_data, &tex_width, &tex_height);
  VkDeviceSize upload_size = tex_width * tex_height * 4 * sizeof(char);
  font_texture_.reset(new Texture(
      app_->logicalDevice(), VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM,
      {(u32)tex_width, (u32)tex_height, 1}, 1, 1, VK_SAMPLE_COUNT_1_BIT,
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, false));
  font_texture_->setData(font_data, queue_family_index, copy_queue);
  font_texture_view_ = std::make_shared<Image::View>(
      font_texture_->image(), VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM,
      VK_IMAGE_ASPECT_COLOR_BIT);

  // Font texture Sampler
  font_texture_sampler_.reset(new Sampler(
      app_->logicalDevice(), VK_FILTER_LINEAR, VK_FILTER_LINEAR,
      VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 0.f, VK_FALSE, 0.f, VK_FALSE,
      VK_COMPARE_OP_ALWAYS, 0.f, 0.f, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
      VK_FALSE));

  // Descriptor pool
  descriptor_pool_ = std::make_unique<DescriptorPool>(app_->logicalDevice(), 2);
  descriptor_pool_->setPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);

  // Pipeline layout
  pipeline_layout_ = std::make_unique<PipelineLayout>(app_->logicalDevice());
  pipeline_layout_->addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, 0,
                                         sizeof(PushConstBlock));
  // Descriptor set layout
  auto &descriptor_set_layout = pipeline_layout_->descriptorSetLayout(
      pipeline_layout_->createLayoutSet(0));
  descriptor_set_layout.addLayoutBinding(
      0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
      VK_SHADER_STAGE_FRAGMENT_BIT);

  // Descriptor Sets
  descriptor_pool_->allocate(pipeline_layout_->descriptorSetLayouts(),
                             descriptor_sets_);
  VkDescriptorImageInfo font_descriptor = {};
  font_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  font_descriptor.imageView = font_texture_view_->handle();
  font_descriptor.sampler = font_texture_sampler_->handle();
  std::array<VkWriteDescriptorSet, 1> descriptor_writes = {};
  descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor_writes[0].dstSet = descriptor_sets_[0];
  descriptor_writes[0].dstBinding = 0;
  descriptor_writes[0].dstArrayElement = 0;
  descriptor_writes[0].descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptor_writes[0].descriptorCount = 1;
  descriptor_writes[0].pImageInfo = &font_descriptor;
  vkUpdateDescriptorSets(app_->logicalDevice()->handle(),
                         static_cast<uint32_t>(descriptor_writes.size()),
                         descriptor_writes.data(), 0, nullptr);

  // Setup graphics pipeline for UI rendering
  pipeline_ = std::make_unique<GraphicsPipeline>(
      app_->logicalDevice(), pipeline_layout_.get(), render_pass, 0);
  pipeline_->setInputState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
  pipeline_->setRasterizationState(
      VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE,
      VK_FRONT_FACE_COUNTER_CLOCKWISE, 0, 0, 0, 0, 1);
  pipeline_->setDepthStencilState(VK_FALSE, VK_FALSE,
                                  VK_COMPARE_OP_LESS_OR_EQUAL, VK_FALSE,
                                  VK_FALSE, {}, {}, 0, 1);
  pipeline_->color_blend_state.addAttachmentState(
      VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, 0);
  pipeline_->viewport_state.addViewport(0, 0, 800, 800, 0, 1);
  pipeline_->viewport_state.addScissor(0, 0, 800, 800);
  pipeline_->setMultisampleState(app_->render_engine.msaaSamples(), VK_FALSE, 1,
                                 std::vector<VkSampleMask>(), VK_FALSE,
                                 VK_FALSE);
  pipeline_->addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
  pipeline_->addDynamicState(VK_DYNAMIC_STATE_SCISSOR);

  // Vertex bindings an attributes based on ImGui vertex definition
  pipeline_->vertex_input_state.addBindingDescription(
      0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX);
  pipeline_->vertex_input_state.addAttributeDescription( // Location 0: Position
      0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos));
  pipeline_->vertex_input_state.addAttributeDescription( // Location 1: UV
      1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv));
  pipeline_->vertex_input_state.addAttributeDescription( // Location 2: Color
      2, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col));
  pipeline_->addShaderStage(*(vert_shader_stage_info_.get()));
  pipeline_->addShaderStage(*(frag_shader_stage_info_.get()));
}

void ImGUI::updateBuffers() {
  ImDrawData *imDrawData = ImGui::GetDrawData();

  // Note: Alignment is done inside buffer creation
  VkDeviceSize vertex_buffer_size =
      imDrawData->TotalVtxCount * sizeof(ImDrawVert);
  VkDeviceSize index_buffer_size =
      imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
  if ((vertex_buffer_size == 0) || (index_buffer_size == 0))
    return;
  // Update buffers only if vertex or index count has been changed compared to
  // current buffer size

  // Vertex buffer
  if ((!vertex_buffer_) || (!vertex_buffer_memory_) ||
      (vertex_count_ != imDrawData->TotalVtxCount)) {
    vertex_buffer_ =
        std::make_unique<Buffer>(app_->logicalDevice(), vertex_buffer_size,
                                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vertex_buffer_memory_ = std::make_unique<DeviceMemory>(
        *(vertex_buffer_.get()), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vertex_count_ = imDrawData->TotalVtxCount;
    vertex_buffer_memory_->map();
    vertex_buffer_memory_->bind(*(vertex_buffer_.get()));
  }

  // Index buffer
  if ((!index_buffer_) || (!index_buffer_memory_) ||
      (index_count_ != imDrawData->TotalIdxCount)) {
    index_buffer_ =
        std::make_unique<Buffer>(app_->logicalDevice(), index_buffer_size,
                                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    index_buffer_memory_ = std::make_unique<DeviceMemory>(
        *(index_buffer_.get()), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    index_count_ = imDrawData->TotalIdxCount;
    index_buffer_memory_->map();
    index_buffer_memory_->bind(*(index_buffer_.get()));
  }

  // Upload data
  ImDrawVert *vtx_dst = (ImDrawVert *)vertex_buffer_memory_->mapped();
  ImDrawIdx *idx_dst = (ImDrawIdx *)index_buffer_memory_->mapped();

  for (int n = 0; n < imDrawData->CmdListsCount; n++) {
    const ImDrawList *cmd_list = imDrawData->CmdLists[n];
    memcpy(vtx_dst, cmd_list->VtxBuffer.Data,
           cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
    memcpy(idx_dst, cmd_list->IdxBuffer.Data,
           cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
    vtx_dst += cmd_list->VtxBuffer.Size;
    idx_dst += cmd_list->IdxBuffer.Size;
  }

  // Flush to make writes visible to GPU
  vertex_buffer_memory_->flush();
  index_buffer_memory_->flush();
}

void ImGUI::drawFrame(circe::vk::CommandBuffer &cb) {
  ImGuiIO &io = ImGui::GetIO();
  cb.bind(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_.get(), 0,
          descriptor_sets_);
  cb.bind(pipeline_.get());
  cb.setViewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y,
                 0.0f, 1.0f);

  // UI scale and translate via push constants
  push_const_block_.scale =
      ponos ::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
  push_const_block_.translate = ponos::vec2(-1.f);
  cb.pushConstants(*(pipeline_layout_.get()), VK_SHADER_STAGE_VERTEX_BIT, 0,
                   sizeof(PushConstBlock), &push_const_block_);

  // Render commands
  ImDrawData *imDrawData = ImGui::GetDrawData();
  int32_t vertex_offset = 0;
  int32_t index_offset = 0;

  if (imDrawData->CmdListsCount > 0) {

    std::vector<VkDeviceSize> offsets = {0};
    cb.bindVertexBuffers(0, {vertex_buffer_->handle()}, offsets);
    cb.bindIndexBuffer(*(index_buffer_.get()), 0, VK_INDEX_TYPE_UINT16);

    for (int32_t i = 0; i < imDrawData->CmdListsCount; i++) {
      const ImDrawList *cmd_list = imDrawData->CmdLists[i];
      for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
        const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[j];
        cb.setScissor(std::max((int32_t)(pcmd->ClipRect.x), 0),
                      std::max((int32_t)(pcmd->ClipRect.y), 0),
                      (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x),
                      (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y));
        cb.drawIndexed(pcmd->ElemCount, 1, index_offset, vertex_offset, 0);
        index_offset += pcmd->ElemCount;
      }
      vertex_offset += cmd_list->VtxBuffer.Size;
    }
  }
}