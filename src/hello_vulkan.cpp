#include "vk_app.h"
#include <iostream>

// #include <dlfcn.h>

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
  // In order to setup the window we first need to connect to the vulkan
  // library. Here we could pass a list of vulkan instance extensions needed by
  // the application. Since the App automatically handles the basic extensions
  // required by the glfw library, we don't need any extra extension.
  ASSERT(app.createInstance());
  // A important step is to choose the hardware we want our application to use.
  // The pickPhysicalDevice gives us the chance to analyse the available
  // hardware and to pick the one that suits best to our needs. This is done by
  // checking the available vulkan queue families that present the features we
  // need, in this example we need just want a queue with graphics and
  // presentation capabilities. The presentation capabilities is already checked
  // automatically, so we just need to check graphics.
  std::vector<circe::vk::VulkanLibraryInterface::QueueFamilyInfo> queue_infos;
  app.pickPhysicalDevice(
      [&](const circe::vk::VulkanLibraryInterface::PhysicalDevice &d) -> bool {
        uint32_t graphics_family_index = 0;
        if (circe::vk::VulkanLibraryInterface::
                selectIndexOfQueueFamilyWithDesiredCapabilities(
                    d.handle, VK_QUEUE_GRAPHICS_BIT, graphics_family_index)) {
          queue_infos.push_back({graphics_family_index, {1.f}});
          return true;
        }
        return false;
      });
  // After picking the hardware, we can create its digital representation. As
  // creating the vulkan instance, here we can also choose extra logical device
  // extensions our application need. The swap chain extension is added
  // automatically, so we need no extra extension.
  ASSERT(app.createLogicalDevice(queue_infos));
  // The swapchain is the mechanism responsible for representing images in our
  // display, here we also need to configure it by choosing image format and
  // color space.
  ASSERT(app.setupSwapChain(VK_FORMAT_R8G8B8A8_UNORM,
                            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR));
  app.run();
  return 0;
}
