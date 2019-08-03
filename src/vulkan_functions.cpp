#include "vulkan_functions.h"

namespace aergia {

#define EXPORTED_VULKAN_FUNCTION(name) PFN_##name name;
#define GLOBAL_LEVEL_VULKAN_FUNCTION(name) PFN_##name name;
#define INSTANCE_LEVEL_VULKAN_FUNCTION(name) PFN_##name name;
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension ) PFN_##name name;
#define DEVICE_LEVEL_VULKAN_FUNCTION(name) PFN_##name name;
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension ) PFN_##name name;

#include "vulkan_functions.inl"

      }  // namespace aergia