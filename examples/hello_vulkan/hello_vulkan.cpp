#include <array>
#include <chrono>
#include <core/vk.h>
#include <iostream>
#include <ponos/ponos.h>
#include <circe/ui/ui_camera.h>
#include <scene/model.h>

#include <example_base.h>

using namespace circe::vk;

class HelloVulkan : public ExampleBase {
public:
  struct UniformBufferObject {
    alignas(16) ponos::mat4 model;
    alignas(16) ponos::mat4 view;
    alignas(16) ponos::mat4 proj;
  };

  HelloVulkan() : ExampleBase(800, 800) {
    app_->graphicsDisplay()->mouse_callback = [&](double x, double y) {
      auto p = app_->graphicsDisplay()->getMouseNPos();
      camera.mouseMove(p);
    };
    app_->graphicsDisplay()->button_callback = [&](int button, int action, int mods) {
      auto p = app_->graphicsDisplay()->getMouseNPos();
      camera.mouseButton(action, button, p);
    };
    app_->graphicsDisplay()->scroll_callback = [&](double x, double y) {
      auto p = app_->graphicsDisplay()->getMouseNPos();
      camera.mouseScroll(p, ponos::vec2(x, y));
    };
    app_->render_engine.resize_callback = [&](uint32_t w, uint32_t h) {
      // TODO update graphics display width, height
      auto &vp = pipeline->viewport_state.viewport(0);
      vp.width = static_cast<float>(w);
      vp.height = static_cast<float>(h);
      auto s = pipeline->viewport_state.scissor(0);
      s.extent.width = w;
      s.extent.height = h;
      camera.resize(w, h);
    };
    app_->render_engine.record_command_buffer_callback =
        [&](CommandBuffer &cb, uint32_t i) {
          Framebuffer &f = this->framebuffers_[i];
          VkDescriptorSet ds = descriptor_sets[i];
          cb.begin();
          circe::vk::RenderPassBeginInfo renderpass_begin_info(this->renderpass_.get(), &f);
          renderpass_begin_info.setRenderArea(0, 0, f.width(), f.height());
          renderpass_begin_info.addClearColorValuef(0.f, 0.f, 0.f, 1.f);
          renderpass_begin_info.addClearDepthStencilValue(1, 0);
          cb.beginRenderPass(renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
          cb.bind(pipeline.get());
          std::vector<VkBuffer> vertex_buffers = {model.vertices().handle()};
          std::vector<VkDeviceSize> offsets = {0};
          cb.bindVertexBuffers(0, vertex_buffers, offsets);
          cb.bindIndexBuffer(model.indices(), 0, VK_INDEX_TYPE_UINT32);
          cb.bind(VK_PIPELINE_BIND_POINT_GRAPHICS,
                  pipeline_layout.get(), 0, {ds});
          cb.drawIndexed(model.indices().size() / sizeof(uint32_t));
          cb.endRenderPass();
          cb.end();
        };
    // setup camera
    camera.setHandedness(false);
    camera.setPosition(ponos::point3(2.0f, 0.0f, 0.0f));
    camera.setTarget(ponos::point3(0.0f, 0.0f, 0.0f));
    camera.setUp(ponos::vec3(0.0f, 1.0f, 0.0f));
    circe::TrackballInterface::createDefault3D(
        camera.trackball);
  }
  ~HelloVulkan() override = default;

  void render() override {}
  void prepare() override {
    ExampleBase::prepare();
    loadModel();
    preparePipeline();
    prepareUniformBuffers();
    prepareDescriptorSets();
  }

  void loadModel() {
    // set vertex layout
    model_vertex_layout.components = {circe::vk::VertexComponent::VERTEX_COMPONENT_POSITION,
                                      circe::vk::VertexComponent::VERTEX_COMPONENT_COLOR,
                                      circe::vk::VertexComponent::VERTEX_COMPONENT_UV};
    model_vertex_layout.fillWithDefaultFormats();
    // init model
    model.setDevice(app_->logicalDevice());
    model.setDeviceQueue(this->graphics_queue_, this->graphics_queue_family_index_);
    std::string model_path(MODELS_PATH);

    if (!model.loadFromOBJ(model_path + "/axis.obj", model_vertex_layout))
      return;
    // load texture
    std::string texture_path(TEXTURES_PATH);
    texture = std::make_unique<Texture>(app_->logicalDevice(), texture_path + "/chalet.jpg",
                                        this->graphics_queue_family_index_, this->graphics_queue_);
    texture_view = std::make_unique<Image::View>(texture->image(), VK_IMAGE_VIEW_TYPE_2D,
                                                 VK_FORMAT_R8G8B8A8_SRGB,
                                                 VK_IMAGE_ASPECT_COLOR_BIT);
    texture_sampler = std::make_unique<Sampler>(
        this->app_->logicalDevice(), VK_FILTER_LINEAR, VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0.f,
        VK_TRUE, 16, VK_FALSE, VK_COMPARE_OP_ALWAYS, 0.f, 0.f,
        VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE);
    // load shaders
    std::string path(SHADERS_PATH);
    frag_shader_module.setDevice(app_->logicalDevice());
    frag_shader_module.load(path + "/frag.spv");
    frag_shader_stage_info.set(VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader_module, "main", nullptr, 0);
    vert_shader_module.setDevice(app_->logicalDevice());
    vert_shader_module.load(path + "/vert.spv");
    vert_shader_stage_info.set(VK_SHADER_STAGE_VERTEX_BIT, vert_shader_module, "main", nullptr, 0);
  }
  void preparePipeline() {
    // create pipeline object
    pipeline = std::make_unique<GraphicsPipeline>(
        this->app_->logicalDevice(), pipeline_layout.get(), this->renderpass_.get(), 0);
    pipeline_layout = std::make_unique<PipelineLayout>(this->app_->logicalDevice());
    pipeline->setLayout(pipeline_layout.get());
    /////////////////////////////////////// ///////////////////////////////////
    pipeline->vertex_input_state.addBindingDescription(0, model_vertex_layout.stride(), VK_VERTEX_INPUT_RATE_VERTEX);
    for (uint32_t i = 0; i < 3; ++i) {
      auto component = model_vertex_layout[i];
      pipeline->vertex_input_state.addAttributeDescription(i, 0, model_vertex_layout.componentFormat(component),
                                                           model_vertex_layout.componentOffset(component));
    }
    pipeline->addShaderStage(vert_shader_stage_info);
    pipeline->addShaderStage(frag_shader_stage_info);

    pipeline->setInputState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    pipeline->viewport_state.addViewport(0, 0, 800, 800, 0.f, 1.f);
    pipeline->viewport_state.addScissor(0, 0, 800, 800);

    pipeline->setRasterizationState(
        VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT,
        VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.f, 0.f, 0.f, 1.0f);

    pipeline->setMultisampleState(this->msaa_samples_, VK_FALSE,
                                  1.f, std::vector<VkSampleMask>(), VK_FALSE,
                                  VK_FALSE);

    pipeline->color_blend_state.addAttachmentState(
        VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);

    pipeline->setDepthStencilState(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS,
                                   VK_FALSE, VK_FALSE, {}, {}, 0.0, 1.0);
    // pipeline.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    // pipeline.addDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
  }
  void prepareDescriptorSets() {
    int set_count = app_->render_engine.swapchainImageViews().size();
    descriptor_pool = std::make_unique<DescriptorPool>(app_->logicalDevice(), set_count);
    descriptor_pool->setPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, set_count);
    descriptor_pool->setPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1000);
    descriptor_pool->setPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000);
    descriptor_pool->setPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000);
    descriptor_pool->setPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000);
    descriptor_pool->setPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000);
    descriptor_pool->setPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000);
    descriptor_pool->setPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000);
    descriptor_pool->setPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000);
    descriptor_pool->setPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000);
    descriptor_pool->setPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000);
    for (int i = 0; i < set_count; ++i) {
      circe::vk::DescriptorSetLayout &dsl =
          pipeline_layout->descriptorSetLayout(pipeline_layout->createLayoutSet(i));
      dsl.addLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                           VK_SHADER_STAGE_VERTEX_BIT);
      dsl.addLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                           VK_SHADER_STAGE_FRAGMENT_BIT);
    }
    descriptor_pool->allocate(pipeline_layout->descriptorSetLayouts(), descriptor_sets);

    for (int i = 0; i < set_count; ++i) {
      VkDescriptorSet ds = descriptor_sets[i];
      VkBuffer ubo = uniform_buffers[i].handle();
      VkDescriptorBufferInfo buffer_info = {};
      buffer_info.buffer = ubo;
      buffer_info.offset = 0;
      buffer_info.range = sizeof(UniformBufferObject);
      VkDescriptorImageInfo image_info = {};
      image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      image_info.imageView = texture_view->handle();
      image_info.sampler = texture_sampler->handle();

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
    }
  }
  void prepareUniformBuffers() {
    for (size_t i = 0; i < app_->render_engine.swapchainImageViews().size(); ++i) {
      uniform_buffers.emplace_back(app_->logicalDevice(),
                                   sizeof(UniformBufferObject),
                                   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      uniform_buffer_memories.emplace_back(uniform_buffers[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      uniform_buffer_memories[i].bind(uniform_buffers[i]);
    }
  }
  void prepareFrameImage(uint32_t index) override {
    auto &ubm = uniform_buffer_memories[index];
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
        current_time - start_time).count();
    UniformBufferObject ubo; // = {};
    ubo.model = ponos::transpose(camera.getModelTransform().matrix());
//        ponos::transpose(
//            ponos::rotateZ(ponos::DEGREES(time * ponos::RADIANS(90.0f)))
//                .matrix());
    ubo.view = camera.getViewTransform().matrix();
    ubo.proj = camera.getProjectionTransform().matrix();
    ubm.copy(&ubo, sizeof(UniformBufferObject));
  }

  // scene
  circe::UserCamera3D camera;
  // model
  VertexLayout model_vertex_layout;
  ShaderModule frag_shader_module;
  PipelineShaderStage frag_shader_stage_info;
  ShaderModule vert_shader_module;
  PipelineShaderStage vert_shader_stage_info;
  Model model;
  std::unique_ptr<Texture> texture;
  std::unique_ptr<Image::View> texture_view;
  std::unique_ptr<Sampler> texture_sampler;
  // pipeline
  std::unique_ptr<PipelineLayout> pipeline_layout;
  std::unique_ptr<GraphicsPipeline> pipeline;
  // descriptor sets
  std::unique_ptr<DescriptorPool> descriptor_pool;
  std::vector<VkDescriptorSet> descriptor_sets;
  // shader resources
  std::vector<Buffer> uniform_buffers;
  std::vector<DeviceMemory> uniform_buffer_memories;
};

int main(int argc, char const *argv[]) {
  HelloVulkan e;
  e.prepare();
  e.run();
  return 0;
}
