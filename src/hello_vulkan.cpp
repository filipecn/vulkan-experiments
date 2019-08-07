#include "vk_app.h"
#include <iostream>

#include <dlfcn.h>

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
  circe::vk::App app(800, 800);
  app.createInstance();
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
  app.createLogicalDevice(queue_infos);
  app.setupSwapChain(VK_FORMAT_R8G8B8A8_UNORM,
                     VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
  app.run();
  return 0;
}
