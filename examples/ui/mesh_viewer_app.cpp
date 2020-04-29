#include "vk_imgui.h"
#include <array>
#include <chrono>
#include <core/vk.h>
#include <iostream>
#include <ponos/ponos.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map>

ponos::Transform perspectiveRH(real_t fovy, real_t aspect, real_t z_near,
                               real_t z_far, bool zero_to_one = false) {
  const real_t tan_half_fovy = std::tan(ponos::RADIANS(fovy / 2.f));
  ponos::mat4 m;
  if (zero_to_one) {
    m.m[0][0] = 1 / (aspect * tan_half_fovy);
    m.m[1][1] = 1 / tan_half_fovy;
    m.m[2][2] = z_far / (z_near - z_far);
    m.m[2][3] = -1;
    m.m[3][2] = -(z_far * z_near) / (z_far - z_near);
  } else {
    m.m[0][0] = 1 / (aspect * tan_half_fovy);
    m.m[1][1] = 1 / (tan_half_fovy);
    m.m[2][2] = -(z_far + z_near) / (z_far - z_near);
    m.m[2][3] = -1;
    m.m[3][2] = -(2 * z_far * z_near) / (z_far - z_near);
  }
  return ponos::Transform(m, ponos::inverse(m));
}

ponos::Transform _lookAtRH(const ponos::point3 &pos,
                           const ponos::point3 &target, const ponos::vec3 &up) {
  ponos::vec3 f = ponos::normalize(target - pos);
  ponos::vec3 s = ponos::normalize(ponos::cross(f, ponos::normalize(up)));
  ponos::vec3 u = ponos::cross(s, f);
  real_t m[4][4];
  m[0][0] = s.x;
  m[1][0] = s.y;
  m[2][0] = s.z;
  m[3][0] = 0;

  m[0][1] = u.x;
  m[1][1] = u.y;
  m[2][1] = u.z;
  m[3][1] = 0;

  m[0][2] = -f.x;
  m[1][2] = -f.y;
  m[2][2] = -f.z;
  m[3][2] = 0;

  m[0][3] = -ponos::dot(s, ponos::vec3(pos - ponos::point3()));
  m[1][3] = -ponos::dot(u, ponos::vec3(pos - ponos::point3()));
  m[2][3] = ponos::dot(f, ponos::vec3(pos - ponos::point3()));
  m[3][3] = 1;

  ponos::mat4 cam_to_world(m);
  return ponos::Transform(cam_to_world, ponos::inverse(cam_to_world));
}

struct Vertex {
  ponos::vec3 pos;
  ponos::vec3 color;
  ponos::vec2 tex_coord;

  bool operator==(const Vertex &other) const {
    return pos == other.pos && color == other.color &&
           tex_coord == other.tex_coord;
  }
};

namespace std {
template <> struct hash<Vertex> {
  size_t operator()(Vertex const &vertex) const {
    return ((hash<ponos::vec3>()(vertex.pos) ^
             (hash<ponos::vec3>()(vertex.color) << 1)) >>
            1) ^
           (hash<ponos::vec2>()(vertex.tex_coord) << 1);
  }
};
} // namespace std

struct UniformBufferObject {
  alignas(16) ponos::mat4 model;
  alignas(16) ponos::mat4 view;
  alignas(16) ponos::mat4 proj;
};

class ShaderSet {
public:
  ShaderSet(const circe::vk::LogicalDevice *logical_device)
      : logical_device_(logical_device) {
    // setup shaders
    std::string path(SHADERS_PATH);
    // fragment shader
    frag_shader_module_ = std::make_shared<circe::vk::ShaderModule>(
        logical_device, path + "/frag.spv");
    frag_shader_stage_info_ = std::make_shared<circe::vk::PipelineShaderStage>(
        VK_SHADER_STAGE_FRAGMENT_BIT, *(frag_shader_module_.get()), "main",
        nullptr, 0);
    // vertex shader
    vert_shader_module_ = std::make_shared<circe::vk::ShaderModule>(
        logical_device, path + "/vert.spv");
    vert_shader_stage_info_ = std::make_shared<circe::vk::PipelineShaderStage>(
        VK_SHADER_STAGE_VERTEX_BIT, *(vert_shader_module_.get()), "main",
        nullptr, 0);
  }
  void addTo(circe::vk::GraphicsPipeline &pipeline) {
    // Vertex data
    pipeline.vertex_input_state.addBindingDescription(
        0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
    pipeline.vertex_input_state.addAttributeDescription(
        0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos));
    pipeline.vertex_input_state.addAttributeDescription(
        1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));
    pipeline.vertex_input_state.addAttributeDescription(
        2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, tex_coord));
    pipeline.addShaderStage(*(vert_shader_stage_info_.get()));
    pipeline.addShaderStage(*(frag_shader_stage_info_.get()));
  }

private:
  const circe::vk::LogicalDevice *logical_device_;
  // shaders
  std::shared_ptr<circe::vk::ShaderModule> frag_shader_module_,
      vert_shader_module_;
  std::shared_ptr<circe::vk::PipelineShaderStage> frag_shader_stage_info_,
      vert_shader_stage_info_;
};

class Mesh {
public:
  Mesh(const circe::vk::LogicalDevice *logical_device)
      : logical_device_(logical_device) {
    // setup texture sampler for shader access
    texture_sampler_ = std::make_shared<circe::vk::Sampler>(
        logical_device, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0.f,
        VK_TRUE, 16, VK_FALSE, VK_COMPARE_OP_ALWAYS, 0.f, 0.f,
        VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE);
  }
  void loadModel(const std::string &obj_filename) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                          obj_filename.c_str()))
      throw std::runtime_error(warn + err);

    std::unordered_map<Vertex, uint32_t> unique_vertices = {};
    for (const auto &shape : shapes) {
      for (const auto &index : shape.mesh.indices) {
        Vertex vertex = {};

        vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
                      attrib.vertices[3 * index.vertex_index + 1],
                      attrib.vertices[3 * index.vertex_index + 2]};

        vertex.tex_coord = {attrib.texcoords[2 * index.texcoord_index + 0],
                            1.0f -
                                attrib.texcoords[2 * index.texcoord_index + 1]};

        vertex.color = {1.0f, 1.0f, 1.0f};

        if (unique_vertices.count(vertex) == 0) {
          unique_vertices[vertex] = static_cast<uint32_t>(h_vertices_.size());
          h_vertices_.push_back(vertex);
        }

        h_indices_.push_back(unique_vertices[vertex]);
      }
    }
  }
  void setTexture(const std::string &path, uint32_t family_index,
                  VkQueue queue) {
    texture_ = std::make_shared<circe::vk::Texture>(logical_device_, path,
                                                    family_index, queue);
    texture_view_ = std::make_shared<circe::vk::Image::View>(
        texture_->image(), VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_ASPECT_COLOR_BIT);
  }
  circe::vk::Image::View *textureView() { return texture_view_.get(); }
  circe::vk::Sampler *textureSampler() { return texture_sampler_.get(); }
  std::vector<Vertex> &vertices() { return h_vertices_; }
  std::vector<uint32_t> indices() { return h_indices_; }

private:
  const circe::vk::LogicalDevice *logical_device_;
  // texture
  std::shared_ptr<circe::vk::Texture> texture_;
  std::shared_ptr<circe::vk::Image::View> texture_view_;
  std::shared_ptr<circe::vk::Sampler> texture_sampler_;
  // model data
  std::vector<Vertex> h_vertices_;
  std::vector<uint32_t> h_indices_;
};

class MeshViewerApp {
public:
  template <typename... Args> MeshViewerApp(Args &&... args) {
    // setup app
    app_ = std::make_unique<circe::vk::App>(std::forward<Args>(args)...);
    app_->setValidationLayers({"VK_LAYER_KHRONOS_validation"});
    // init shaders
    shader_ = std::make_unique<ShaderSet>(app_->logicalDevice());
    // init mesh
    h_mesh_ = std::make_unique<Mesh>(app_->logicalDevice());
    // retrieve queue for buffer upload operations
    family_index_ =
        app_->queueFamilies().family("graphics").family_index.value();
    queue_ = app_->queueFamilies().family("graphics").vk_queues[0];
    imgui_ = std::make_unique<ImGUI>(app_.get());
  }
  ~MeshViewerApp() {}
  void run() {
    // Prepare render engine
    prepareDescriptorSets();
    preparePipeline();
    // setupImgui();
    // draw callback
    app_->render_engine.update_uniform_buffer_callback =
        [&](circe::vk::DeviceMemory &ubm) {
          static auto start_time = std::chrono::high_resolution_clock::now();
          auto current_time = std::chrono::high_resolution_clock::now();
          float time =
              std::chrono::duration<float, std::chrono::seconds::period>(
                  current_time - start_time)
                  .count();
          UniformBufferObject ubo; // = {};
          ubo.model =
              // ponos::Transform().matrix();
              ponos::transpose(
                  ponos::rotateZ(ponos::DEGREES(time * ponos::RADIANS(90.0f)))
                      .matrix());
          ubo.view = ponos::transpose(_lookAtRH(ponos::point3(2.0f, 0.0f, 0.0f),
                                                ponos::point3(0.0f, 0.0f, 0.0f),
                                                ponos::vec3(0.0f, 1.0f, 0.0f))
                                          .matrix());
          ubo.proj =
              // ponos::transpose(
              // ponos::mat4({1.0f, 0.0f, 0.0f, 0.0f,  //
              //              0.0f, -1.0f, 0.0f, 0.0f, //
              //              0.0f, 0.0f, 0.5f, 0.0f,  //
              //              0.0f, 0.0f, 0.5f, 1.0f}) *
              perspectiveRH(45.0f, 1.f, 0.1f, 10.0f).matrix();
          // ubo.proj.m[1][1] *= -1;
          // );
          ubm.copy(&ubo, sizeof(UniformBufferObject));
        };
    // set resize callback
    app_->render_engine.resize_callback = [&](uint32_t w, uint32_t h) {
      auto &vp =
          app_->render_engine.graphicsPipeline()->viewport_state.viewport(0);
      vp.width = static_cast<float>(w);
      vp.height = static_cast<float>(h);
      auto s =
          app_->render_engine.graphicsPipeline()->viewport_state.scissor(0);
      s.extent.width = w;
      s.extent.height = h;
    };
    // render callback
    app_->render_engine.record_command_buffer_callback =
        [&](circe::vk::CommandBuffer &cb, circe::vk::Framebuffer &f,
            VkDescriptorSet ds) {
          cb.begin();
          circe::vk::RenderPassBeginInfo renderpass_begin_info(
              app_->render_engine.renderpass(), &f);
          renderpass_begin_info.setRenderArea(0, 0, f.width(), f.height());
          renderpass_begin_info.addClearColorValuef(0.2f, 0.2f, 0.2f, 1.f);
          renderpass_begin_info.addClearDepthStencilValue(1, 0);
          cb.beginRenderPass(renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
          cb.bind(app_->render_engine.graphicsPipeline());
          std::vector<VkBuffer> vertex_buffers = {
              mesh_->vertexBuffer()->handle()};
          std::vector<VkDeviceSize> offsets = {0};
          cb.bindVertexBuffers(0, vertex_buffers, offsets);
          cb.bindIndexBuffer(*mesh_->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
          cb.bind(VK_PIPELINE_BIND_POINT_GRAPHICS,
                  app_->render_engine.pipelineLayout(), 0, {ds});
          cb.drawIndexed(h_mesh_->indices().size());
          imgui_->drawFrame(cb);
          cb.endRenderPass();
          cb.end();
        };
    imgui_->init(ponos::size2(app_->graphicsDisplay()->width(),
                              app_->graphicsDisplay()->height()));
    imgui_->initResources(app_->render_engine.renderpass(), queue_,
                          family_index_);
    drawImgui();
    app_->run([&]() {});
  }
  void loadModel(const std::string &obj_path) {
    h_mesh_->loadModel(obj_path);
    mesh_ = std::make_unique<circe::vk::MeshBufferData>(
        app_->logicalDevice(),
        sizeof(h_mesh_->vertices()[0]) * h_mesh_->vertices().size(),
        h_mesh_->vertices().data(),
        sizeof(h_mesh_->indices()[0]) * h_mesh_->indices().size(),
        h_mesh_->indices().data(), family_index_, queue_);
  }
  void loadTexture(const std::string &tex_path) {
    h_mesh_->setTexture(tex_path, family_index_, queue_);
  }

private:
  void prepareDescriptorSets() {
    // inform size of uniform buffer in bytes
    app_->render_engine.uniform_buffer_size_callback = []() -> uint32_t {
      return sizeof(UniformBufferObject);
    };
    // describe resources accessed by the shaders
    app_->render_engine.descriptor_set_layout_callback =
        [](circe::vk::DescriptorSetLayout &dsl) {
          dsl.addLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                               VK_SHADER_STAGE_VERTEX_BIT);
          dsl.addLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                               VK_SHADER_STAGE_FRAGMENT_BIT);
        };
    // set the descriptor set update callback
    app_->render_engine.update_descriptor_set_callback = [&](VkDescriptorSet ds,
                                                             VkBuffer ubo) {
      VkDescriptorBufferInfo buffer_info = {};
      buffer_info.buffer = ubo;
      buffer_info.offset = 0;
      buffer_info.range = sizeof(UniformBufferObject);
      VkDescriptorImageInfo image_info = {};
      image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      image_info.imageView = h_mesh_->textureView()->handle();
      image_info.sampler = h_mesh_->textureSampler()->handle();

      std::array<VkWriteDescriptorSet, 2> descriptor_writes = {};

      descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptor_writes[0].dstSet = ds;
      descriptor_writes[0].dstBinding = 0;
      descriptor_writes[0].dstArrayElement = 0;
      descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptor_writes[0].descriptorCount = 1;
      descriptor_writes[0].pBufferInfo = &buffer_info;

      descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptor_writes[1].dstSet = ds;
      descriptor_writes[1].dstBinding = 1;
      descriptor_writes[1].dstArrayElement = 0;
      descriptor_writes[1].descriptorType =
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptor_writes[1].descriptorCount = 1;
      descriptor_writes[1].pImageInfo = &image_info;

      vkUpdateDescriptorSets(app_->logicalDevice()->handle(),
                             static_cast<uint32_t>(descriptor_writes.size()),
                             descriptor_writes.data(), 0, nullptr);
    };
  }
  void preparePipeline() {
    auto *layout = app_->render_engine.pipelineLayout();
    auto *renderpass = app_->render_engine.renderpass();
    auto &subpass_desc = renderpass->newSubpassDescription();
    { // COLOR ATTACHMENT
      renderpass->addAttachment(
          app_->render_engine.swapchain()->surfaceFormat().format,
          app_->render_engine.msaaSamples(), VK_ATTACHMENT_LOAD_OP_CLEAR,
          VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
          // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      subpass_desc.addColorAttachmentRef(
          0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      renderpass->addSubpassDependency(
          VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
          // VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    }
    { // DEPTH ATTACHMENT
      renderpass->addAttachment(
          app_->render_engine.depthFormat(), app_->render_engine.msaaSamples(),
          VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
          VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
          VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
      subpass_desc.setDepthStencilAttachmentRef(
          1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }
    // since we are using multisampling, the first color attachment is cannot be
    // presented directly, first we need to resolve it into a proper image
    { // COLOR RESOLVE ATTACHMENT RESOLVE
      renderpass->addAttachment(
          app_->render_engine.swapchain()->surfaceFormat().format,
          VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
      subpass_desc.addResolveAttachmentRef(
          2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
    auto &pipeline = *app_->render_engine.graphicsPipeline();
    shader_->addTo(pipeline);
    pipeline.setInputState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipeline.viewport_state.addViewport(0, 0, 800, 800, 0.f, 1.f);
    pipeline.viewport_state.addScissor(0, 0, 800, 800);
    pipeline.setRasterizationState(
        VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT,
        VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.f, 0.f, 0.f, 1.0f);
    pipeline.setMultisampleState(app_->render_engine.msaaSamples(), VK_FALSE,
                                 1.f, std::vector<VkSampleMask>(), VK_FALSE,
                                 VK_FALSE);
    pipeline.color_blend_state.addAttachmentState(
        VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
    pipeline.setDepthStencilState(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS,
                                  VK_FALSE, VK_FALSE, {}, {}, 0.0, 1.0);
    // pipeline.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    // pipeline.addDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
  }
  void drawImgui() {
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
    ImGui::ShowDemoWindow();
    // Render to generate draw buffers
    ImGui::Render();
    imgui_->updateBuffers();
  }

  std::unique_ptr<circe::vk::App> app_;
  std::unique_ptr<circe::vk::RenderPass> imgui_renderpass_;
  std::unique_ptr<circe::vk::DescriptorPool> imgui_descriptor_pool_;
  std::unique_ptr<ShaderSet> shader_;
  std::unique_ptr<Mesh> h_mesh_;
  std::unique_ptr<circe::vk::MeshBufferData> mesh_;
  uint32_t family_index_;
  VkQueue queue_;
  std::unique_ptr<ImGUI> imgui_;
};

int main(int argc, char const *argv[]) {
  MeshViewerApp viewer(800, 800);
  // load model
  std::string model_path(MODELS_PATH);
  viewer.loadModel(model_path + "/chalet.obj");
  // load texture
  std::string texture_path(TEXTURES_PATH);
  viewer.loadTexture(texture_path + "/chalet.jpg");

  viewer.run();
  return 0;
}
