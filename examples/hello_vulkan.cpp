#include "vk.h"
#include <iostream>

struct vec2 {
  float x, y;
};
struct vec3 {
  float x, y, z;
};

struct Vertex {
  vec2 pos;
  vec3 color;
};

const std::vector<Vertex> vertices = {{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
                                      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

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
                                             std::move(path + "/frag.spv"));
  // With a shader module in hands, we need to describe it to define our
  // pipeline later.
  circe::vk::PipelineShaderStage frag_shader_stage_info(
      VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader_module, "main", nullptr, 0);
  circe::vk::ShaderModule vert_shader_module(app.logicalDevice(),
                                             std::move(path + "/vert.spv"));
  circe::vk::PipelineShaderStage vert_shader_stage_info(
      VK_SHADER_STAGE_VERTEX_BIT, vert_shader_module, "main", nullptr, 0);
  // An important part is to describe the resources that will be used by the
  // shaders, which is done via a pipeline layout.
  auto *layout = app.pipelineLayout();
  auto *renderpass = app.renderpass();
  renderpass->addAttachment(
      app.swapchain()->surfaceFormat().format, VK_SAMPLE_COUNT_1_BIT,
      VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
      VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  auto &subpass_desc = renderpass->newSubpassDescription();
  subpass_desc.addColorAttachmentRef(0,
                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  renderpass->addSubpassDependency(
      VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
  auto &pipeline = *app.graphicsPipeline();
  // Vertex data
  pipeline.vertex_input_state.addBindingDescription(
      0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
  pipeline.vertex_input_state.addAttributeDescription(
      0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos));
  pipeline.vertex_input_state.addAttributeDescription(
      1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));
  pipeline.addShaderStage(vert_shader_stage_info);
  pipeline.addShaderStage(frag_shader_stage_info);
  pipeline.setInputState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  pipeline.viewport_state.addViewport(0, 0, 800, 800, 0.f, 1.f);
  pipeline.viewport_state.addScissor(0, 0, 800, 800);
  pipeline.setRasterizationState(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL,
                                 VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE,
                                 VK_FALSE, 0.f, 0.f, 0.f, 1.0f);
  pipeline.setMultisampleState(VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.f,
                               std::vector<VkSampleMask>(), VK_FALSE, VK_FALSE);
  pipeline.color_blend_state.addAttachmentState(
      VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
      VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
  // pipeline.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
  // pipeline.addDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
  // vertex buffer
  circe::vk::Buffer vertex_buffer(
      app.logicalDevice(), sizeof(vertices[0]) * vertices.size(),
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices.data());
  circe::vk::DeviceMemory vertex_buffer_memory(
      app.logicalDevice(), vertex_buffer,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  vertex_buffer_memory.bind(vertex_buffer);
  vertex_buffer_memory.copy(vertex_buffer);
  app.resize_callback = [&](uint32_t w, uint32_t h) {
    auto &vp = pipeline.viewport_state.viewport(0);
    vp.width = w;
    vp.height = h;
    auto s = pipeline.viewport_state.scissor(0);
    s.extent.width = w;
    s.extent.height = h;
  };
  app.record_command_buffer_callback = [&](circe::vk::CommandBuffer &cb,
                                           circe::vk::Framebuffer &f) {
    cb.begin();
    circe::vk::RenderPassBeginInfo renderpass_begin_info(app.renderpass(), &f);
    renderpass_begin_info.setRenderArea(0, 0, f.width(), f.height());
    renderpass_begin_info.addClearColorValuef(0.f, 0.f, 0.f, 1.f);
    cb.beginRenderPass(renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    cb.bind(pipeline);
    std::vector<VkBuffer> vertex_buffers = {vertex_buffer.handle()};
    std::vector<VkDeviceSize> offsets = {0};
    cb.bindVertexBuffers(0, vertex_buffers, offsets);
    cb.draw(vertices.size());
    cb.endRenderPass();
    cb.end();
  };

  app.run();
  return 0;
}