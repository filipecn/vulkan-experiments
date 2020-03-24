#include "vk.h"
#include <array>
#include <chrono>
#include <iostream>
#include <ponos/ponos.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map>

struct UniformBufferObject {
  alignas(16) ponos::mat4 model;
  alignas(16) ponos::mat4 view;
  alignas(16) ponos::mat4 proj;
};

// struct vec2 {
//   float x, y;
// };
// struct vec3 {
//   float x, y, z;
// };

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

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;
void loadObj(const std::string &filename) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                        filename.c_str()))
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
        unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }

      indices.push_back(unique_vertices[vertex]);

      // vertices.push_back(vertex);
      // indices.push_back(indices.size());
    }
  }
}

int main(int argc, char const *argv[]) {
#ifndef GLFW_INCLUDE_VULKAN
#if defined __WIN32__
  HMODULE vulkanLibrary = LoadLibrary("vulkan-1.dll");
#elif __linux
  void *vulkanLibrary = dlopen("libvulkan.so.1", RTLD_NOW);
#elif __APPLE__
  void *vulkanLibrary = dlopen("libvulkan.1.1.114.dylib", RTLD_NOW);
#endif
  if (vulkanLibrary == nullptr) {
    std::cerr << "Could not connect with a Vulkan Runtime library.\n";
    return -1;
  }
  circe::vk::VulkanLibraryInterface::loadLoaderFunctionFromVulkan(
      vulkanLibrary);
  circe::vk::VulkanLibraryInterface::loadGlobalLevelFunctions();
#endif
  // The app represents the window in which we display our graphics
  circe::vk::App app(800, 800);
  app.setValidationLayers({"VK_LAYER_KHRONOS_validation"});
  // In order to setup the window we first need to connect to the vulkan
  // library. Here we could pass a list of vulkan instance extensions needed by
  // the application. The App automatically handles the basic extensions
  // required by the glfw library, so we don't need any extra extension.
  // app.setInstance(...);
  // A important step is to choose the hardware we want our application to use.
  // The pickPhysicalDevice gives us the chance to analyse the available
  // hardware and to pick the one that suits best to our needs. This is done by
  // checking the available vulkan queue families that present the features we
  // need, in this example we need just a queue with graphics and
  // presentation capabilities. The presentation capabilities is already checked
  // automatically, so we just need to check graphics.
  // app.pickPhysicalDevice([&](const circe::vk::PhysicalDevice &d,
  //                           circe::vk::QueueFamilies &q) -> uint32_t {
  //  std::cerr << d;
  //  if (d.properties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
  //    return 1000;
  //  return 1;
  //});
  // After picking the hardware, we can create its digital representation. As
  // creating the vulkan instance, here we can also choose extra logical device
  // extensions our application need. The swap chain extension is added
  // automatically, so we need no extra extension.
  // ASSERT(app.createLogicalDevice());
  // The swapchain is the mechanism responsible for representing images in our
  // display, here we also need to configure it by choosing image format and
  // color space.
  // ASSERT(app.setupSwapChain(VK_FORMAT_R8G8B8A8_UNORM,
  //                          VK_COLOR_SPACE_SRGB_NONLINEAR_KHR));
  // GRAPHICS PIPELINE
  // To setup a graphics application we need to define the pipeline that will
  // process our data and render our images. This is a complex procedure
  // consisted of several parts.
  // Shaders represent the code that will process the data and render. A set of
  // shaders is compiled into a shader module.
  std::string path(SHADERS_PATH);
  circe::vk::ShaderModule frag_shader_module(app.logicalDevice(),
                                             path + "/frag.spv");
  // With a shader module in hands, we need to describe it to define our
  // pipeline later.
  circe::vk::PipelineShaderStage frag_shader_stage_info(
      VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader_module, "main", nullptr, 0);
  circe::vk::ShaderModule vert_shader_module(app.logicalDevice(),
                                             path + "/vert.spv");
  circe::vk::PipelineShaderStage vert_shader_stage_info(
      VK_SHADER_STAGE_VERTEX_BIT, vert_shader_module, "main", nullptr, 0);
  uint32_t graphics_family_index =
      app.queueFamilies().family("graphics").family_index.value();
  VkQueue graphics_queue = app.queueFamilies().family("graphics").vk_queues[0];
  // load model
  std::string model_path(MODELS_PATH);
  loadObj(model_path + "/chalet.obj");
  // load texture
  std::string texture_path(TEXTURES_PATH);
  circe::vk::Texture texture(app.logicalDevice(), texture_path + "/chalet.jpg",
                             graphics_family_index, graphics_queue);
  circe::vk::Image::View texture_view(texture.image(), VK_IMAGE_VIEW_TYPE_2D,
                                      VK_FORMAT_R8G8B8A8_SRGB,
                                      VK_IMAGE_ASPECT_COLOR_BIT);
  circe::vk::Sampler texture_sampler(
      app.logicalDevice(), VK_FILTER_LINEAR, VK_FILTER_LINEAR,
      VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
      VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0.f,
      VK_TRUE, 16, VK_FALSE, VK_COMPARE_OP_ALWAYS, 0.f, 0.f,
      VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE);

  app.render_engine.uniform_buffer_size_callback = []() -> uint32_t {
    return sizeof(UniformBufferObject);
  };
  app.render_engine
      .update_uniform_buffer_callback = [&](circe::vk::Buffer &ubm) {
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     current_time - start_time)
                     .count();
    UniformBufferObject ubo; // = {};
    ubo.model = ponos::transpose(
        ponos::rotateZ(ponos::DEGREES(time * ponos::RADIANS(90.0f))).matrix());
    ubo.view = ponos::transpose(ponos::lookAtRH(ponos::point3(2.0f, 2.0f, 2.0f),
                                                ponos::point3(0.0f, 0.0f, 0.0f),
                                                ponos::vec3(0.0f, 1.0f, 0.0f))
                                    .matrix());
    ubo.proj =
        ponos::transpose(/*ponos::mat4({1.0f, 0.0f, 0.0f, 0.0f,  //
                                      0.0f, -1.0f, 0.0f, 0.0f, //
                                      0.0f, 0.0f, 0.5f, 0.0f,  //
                                      0.0f, 0.0f, 0.5f, 1.0f}) **/
                         ponos::perspective(45.0f, 1.f, 0.1f, 10.0f).matrix());
    ubm.setData(&ubo, sizeof(UniformBufferObject));
  };
  // An important part is to describe the resources that will be used by the
  // shaders, which is done via a pipeline layout.
  app.render_engine.descriptor_set_layout_callback =
      [](circe::vk::DescriptorSetLayout &dsl) {
        dsl.addLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                             VK_SHADER_STAGE_VERTEX_BIT);
        dsl.addLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                             VK_SHADER_STAGE_FRAGMENT_BIT);
      };
  app.render_engine.update_descriptor_set_callback = [&](VkDescriptorSet ds,
                                                         VkBuffer ubo) {
    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = ubo;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(UniformBufferObject);
    VkDescriptorImageInfo image_info = {};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = texture_view.handle();
    image_info.sampler = texture_sampler.handle();

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

    vkUpdateDescriptorSets(app.logicalDevice()->handle(),
                           static_cast<uint32_t>(descriptor_writes.size()),
                           descriptor_writes.data(), 0, nullptr);
  };
  auto *layout = app.render_engine.pipelineLayout();
  auto *renderpass = app.render_engine.renderpass();
  auto &subpass_desc = renderpass->newSubpassDescription();
  { // COLOR ATTACHMENT
    renderpass->addAttachment(
        app.render_engine.swapchain()->surfaceFormat().format,
        app.render_engine.msaaSamples(), VK_ATTACHMENT_LOAD_OP_CLEAR,
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
        app.render_engine.depthFormat(), app.render_engine.msaaSamples(),
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
        app.render_engine.swapchain()->surfaceFormat().format,
        VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    subpass_desc.addResolveAttachmentRef(
        2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  }
  auto &pipeline = *app.render_engine.graphicsPipeline();
  // Vertex data
  pipeline.vertex_input_state.addBindingDescription(
      0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
  pipeline.vertex_input_state.addAttributeDescription(
      0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos));
  pipeline.vertex_input_state.addAttributeDescription(
      1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));
  pipeline.vertex_input_state.addAttributeDescription(
      2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, tex_coord));
  pipeline.addShaderStage(vert_shader_stage_info);
  pipeline.addShaderStage(frag_shader_stage_info);
  pipeline.setInputState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  pipeline.viewport_state.addViewport(0, 0, 800, 800, 0.f, 1.f);
  pipeline.viewport_state.addScissor(0, 0, 800, 800);
  pipeline.setRasterizationState(
      VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT,
      VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.f, 0.f, 0.f, 1.0f);
  pipeline.setMultisampleState(app.render_engine.msaaSamples(), VK_FALSE, 1.f,
                               std::vector<VkSampleMask>(), VK_FALSE, VK_FALSE);
  pipeline.color_blend_state.addAttachmentState(
      VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
      VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
  pipeline.setDepthStencilState(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS, VK_FALSE,
                                VK_FALSE, {}, {}, 0.0, 1.0);
  // pipeline.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
  // pipeline.addDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
  circe::vk::MeshBufferData mesh(
      app.logicalDevice(), sizeof(vertices[0]) * vertices.size(),
      vertices.data(), sizeof(indices[0]) * indices.size(), indices.data(),
      graphics_family_index, graphics_queue);
  app.render_engine.resize_callback = [&](uint32_t w, uint32_t h) {
    auto &vp = app.render_engine.graphicsPipeline()->viewport_state.viewport(0);
    vp.width = static_cast<float>(w);
    vp.height = static_cast<float>(h);
    auto s = app.render_engine.graphicsPipeline()->viewport_state.scissor(0);
    s.extent.width = w;
    s.extent.height = h;
  };
  app.render_engine.record_command_buffer_callback =
      [&](circe::vk::CommandBuffer &cb, circe::vk::Framebuffer &f,
          VkDescriptorSet ds) {
        cb.begin();
        circe::vk::RenderPassBeginInfo renderpass_begin_info(
            app.render_engine.renderpass(), &f);
        renderpass_begin_info.setRenderArea(0, 0, f.width(), f.height());
        renderpass_begin_info.addClearColorValuef(0.f, 0.f, 0.f, 1.f);
        renderpass_begin_info.addClearDepthStencilValue(1, 0);
        cb.beginRenderPass(renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        cb.bind(app.render_engine.graphicsPipeline());
        std::vector<VkBuffer> vertex_buffers = {mesh.vertexBuffer()->handle()};
        std::vector<VkDeviceSize> offsets = {0};
        cb.bindVertexBuffers(0, vertex_buffers, offsets);
        cb.bindIndexBuffer(*mesh.indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
        cb.bind(VK_PIPELINE_BIND_POINT_GRAPHICS,
                app.render_engine.pipelineLayout(), 0, {ds});
        cb.drawIndexed(indices.size());
        cb.endRenderPass();
        cb.end();
      };
  app.run();
  return 0;
}
