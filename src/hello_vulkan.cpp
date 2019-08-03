#include <iostream>
#include "vulkan_library.h"

int main(int argc, char const* argv[]) {
#if defined _WIN32
  HMODULE vulkanLibrary = LoadLibrary("vulkan-1.dll");
#elif __linux
  void* vulkanLibrary = dlopen("libvulkan.so.1", RTLD_NOW);
#endif

  if (vulkanLibrary == nullptr) {
    std::cerr << "Could not connect with a Vulkan Runtime library.\n";
    return -1;
  }

  aergia::VulkanLibrary::loadFunctionFromVulkan(vulkanLibrary);
  aergia::VulkanLibrary::loadGlobalLevelFunctions();
  std::vector<const char*> extensions;
  VkInstance instance;
  aergia::VulkanLibrary::createInstance(extensions, "hello vulkan", instance);

  return 0;
}
