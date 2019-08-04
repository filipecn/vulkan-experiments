#include "vulkan_library.h"
#include <iostream>

#include <dlfcn.h>

int main(int argc, char const *argv[]) {
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

  circe::VulkanLibraryInterface::loadLoaderFunctionFromVulkan(vulkanLibrary);
  circe::VulkanLibraryInterface::loadGlobalLevelFunctions();
  std::vector<const char *> extensions;
  VkInstance instance;
  circe::VulkanLibraryInterface::createInstance(extensions, "hello vulkan",
                                                instance);

  return 0;
}
