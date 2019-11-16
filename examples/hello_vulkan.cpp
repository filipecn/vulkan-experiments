#include "vk.h"
#include <iostream>

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
  circe::vk::PipelineLayout layout(app.logicalDevice());
  circe::vk::RenderPass renderpass(app.logicalDevice());
  renderpass.addAttachment(
      app.swapchain()->surfaceFormat().format, VK_SAMPLE_COUNT_1_BIT,
      VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
      VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  auto &subpass_desc = renderpass.newSubpassDescription();
  subpass_desc.addColorAttachmentRef(0,
                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  renderpass.addSubpassDependency(
      VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
  circe::vk::GraphicsPipeline pipeline(app.logicalDevice(), layout, renderpass,
                                       0);
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
  pipeline.handle();
  // The attachments specified during render pass creation are bound by wrapping
  // them into a Framebuffer object. A framebuffer object references all of the
  // VkImageView objects that represent the attachments. We need one framebuffer
  // per swapchain image.
  std::vector<circe::vk::Framebuffer> framebuffers;
  auto &swapchain_image_views = app.swapchainImageViews();
  for (auto &image_view : swapchain_image_views) {
    circe::vk::Framebuffer framebuffer(app.logicalDevice(), &renderpass,
                                       app.swapchain()->imageSize().width,
                                       app.swapchain()->imageSize().height, 1);
    framebuffer.addAttachment(image_view);
    framebuffers.emplace_back(framebuffer);
  }
  circe::vk::CommandPool command_pool(
      app.logicalDevice(), 0,
      app.queueFamilies().family("graphics").family_index.value());
  std::vector<circe::vk::CommandBuffer> command_buffers;
  command_pool.allocateCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                      framebuffers.size(), command_buffers);
  for (size_t i = 0; i < command_buffers.size(); ++i) {
    command_buffers[i].begin();
    circe::vk::RenderPassBeginInfo renderpass_begin_info(renderpass,
                                                         framebuffers[i]);
    renderpass_begin_info.setRenderArea(0, 0, framebuffers[i].width(),
                                        framebuffers[i].height());
    renderpass_begin_info.addClearColorValuef(0.f, 0.f, 0.f, 1.f);
    command_buffers[i].beginRenderPass(renderpass_begin_info,
                                       VK_SUBPASS_CONTENTS_INLINE);
    command_buffers[i].bind(pipeline);
    command_buffers[i].draw(3);
    command_buffers[i].endRenderPass();
    command_buffers[i].end();
  }
  const int MAX_FRAMES_IN_FLIGHT = 2;
  std::vector<circe::vk::Semaphore> render_finished_semaphores,
      image_available_semaphores;
  std::vector<circe::vk::Fence> in_flight_fences;
  std::vector<VkFence> images_in_flight(framebuffers.size(), VK_NULL_HANDLE);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    in_flight_fences.emplace_back(app.logicalDevice(),
                                  VK_FENCE_CREATE_SIGNALED_BIT);
    image_available_semaphores.emplace_back(app.logicalDevice());
    render_finished_semaphores.emplace_back(app.logicalDevice());
  }
  size_t current_frame = 0;
  app.run([&]() {
    in_flight_fences[current_frame].wait();
    uint32_t image_index = 0;
    app.swapchain()->nextImage(
        image_available_semaphores[current_frame].handle(), VK_NULL_HANDLE,
        image_index);
    if (images_in_flight[image_index] != VK_NULL_HANDLE)
      vkWaitForFences(app.logicalDevice()->handle(), 1,
                      &images_in_flight[image_index], VK_TRUE, UINT64_MAX);
    images_in_flight[image_index] = in_flight_fences[current_frame].handle();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkCommandBuffer command_buffer = command_buffers[image_index].handle();
    VkSemaphore waitSemaphores[] = {
        image_available_semaphores[current_frame].handle()};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffer;

    VkSemaphore signalSemaphores[] = {
        render_finished_semaphores[current_frame].handle()};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    in_flight_fences[current_frame].reset();

    VkResult result =
        vkQueueSubmit(app.queueFamilies().family("graphics").vk_queues[0], 1,
                      &submitInfo, in_flight_fences[current_frame].handle());
    if (VK_SUCCESS != result)
      std::cerr << "osp\n";

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {app.swapchain()->handle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &image_index;

    vkQueuePresentKHR(app.queueFamilies().family("presentation").vk_queues[0],
                      &presentInfo);

    vkQueueWaitIdle(app.queueFamilies().family("presentation").vk_queues[0]);

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
  });
  return 0;
}
