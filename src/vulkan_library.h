#ifndef AERGIA_VULKAN_LIBRARY
#define AERGIA_VULKAN_LIBRARY

#include <windows.h>
#include <sstream>
#include <vector>
#include "vulkan_functions.h"

#if defined _WIN32
#define VulkanLibraryType HMODULE
#elif __linux
#define VulkanLibraryType void*
#endif

namespace aergia {

inline void concatenate(std::ostringstream& s) {}

template <typename H, typename... T>
void concatenate(std::ostringstream& s, H p, T... t) {
  s << p;
  concatenate(s, t...);
}

template <class... Args>
std::string concat(const Args&... args) {
  std::ostringstream s;
  concatenate(s, args...);
  return s.str();
}

class VulkanLibrary {
 public:
  struct QueueInfo {
    uint32_t familyIndex;
    std::vector<float> priorities;
  };
  struct WindowParameters {
#ifdef VK_USE_PLATFORM_WIN32_KHR

    HINSTANCE HInstance;
    HWND HWnd;

#elif defined VK_USE_PLATFORM_XLIB_KHR

    Display* Dpy;
    Window Window;

#elif defined VK_USE_PLATFORM_XCB_KHR

    xcb_connection_t* Connection;
    xcb_window_t Window;

#endif
  };
  struct PresentInfo {
    VkSwapchainKHR Swapchain;
    uint32_t ImageIndex;
  };
  struct WaitSemaphoreInfo {
    VkSemaphore Semaphore;
    VkPipelineStageFlags WaitingStage;
  };
  static bool loadFunctionFromVulkan(VulkanLibraryType& vulkanLibrary);
  static bool loadGlobalLevelFunctions();
  static bool checkAvaliableInstanceExtensions(
      std::vector<VkExtensionProperties>& extensions);
  static bool isExtensionSupported(
      const std::vector<VkExtensionProperties>& extensions,
      const char* extensionName);
  static bool createInstance(const std::vector<const char*>& extensions,
                             std::string applicationName, VkInstance& instance);
  static bool loadInstanceLevelFunctions(
      VkInstance instance, const std::vector<const char*>& extensions);
  static bool enumerateAvailablePhysicalDevices(
      VkInstance instance, std::vector<VkPhysicalDevice>& devices);
  static bool checkAvailableDeviceExtensions(
      VkPhysicalDevice physicalDevice,
      std::vector<VkExtensionProperties>& extensions);
  static void getFeaturesAndPropertiesOfPhysicalDevice(
      VkPhysicalDevice physical_device,
      VkPhysicalDeviceFeatures& device_features,
      VkPhysicalDeviceProperties& device_properties);
  static bool checkAvailableQueueFamiliesAndTheirProperties(
      VkPhysicalDevice physical_device,
      std::vector<VkQueueFamilyProperties>& queue_families);
  static bool selectIndexOfQueueFamilyWithDesiredCapabilities(
      VkPhysicalDevice physical_device, VkQueueFlags desired_capabilities,
      uint32_t& queue_family_index);
  static bool createLogicalDevice(
      VkPhysicalDevice physical_device, std::vector<QueueInfo> queue_infos,
      std::vector<char const*> const& desired_extensions,
      VkPhysicalDeviceFeatures* desired_features, VkDevice& logical_device);
  static bool loadDeviceLevelFunctions(
      VkDevice logical_device,
      std::vector<char const*> const& enabled_extensions);
  static void getDeviceQueue(VkDevice logical_device,
                             uint32_t queue_family_index, uint32_t queue_index,
                             VkQueue& queue);
  static bool createLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues(
      VkInstance instance, VkDevice& logical_device, VkQueue& graphics_queue,
      VkQueue& compute_queue);
  static void destroyLogicalDevice(VkDevice& logical_device);
  static void destroyVulkanInstance(VkInstance& instance);
  static void releaseVulkanLoaderLibrary(VulkanLibraryType& vulkan_library);
  static bool createVulkanInstanceWithWsiExtensionsEnabled(
      std::vector<char const*>& desired_extensions,
      char const* const application_name, VkInstance& instance);
  static bool createPresentationSurface(VkInstance instance,
                                        WindowParameters window_parameters,
                                        VkSurfaceKHR& presentation_surface);
  static bool selectQueueFamilyThatSupportsPresentationToGivenSurface(
      VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
      uint32_t& queue_family_index);
  static bool createLogicalDeviceWithWsiExtensionsEnabled(
      VkPhysicalDevice physical_device, std::vector<QueueInfo> queue_infos,
      std::vector<char const*>& desired_extensions,
      VkPhysicalDeviceFeatures* desired_features, VkDevice& logical_device);
  static bool selectDesiredPresentationMode(
      VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
      VkPresentModeKHR desired_present_mode, VkPresentModeKHR& present_mode);
  static bool getCapabilitiesOfPresentationSurface(
      VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
      VkSurfaceCapabilitiesKHR& surface_capabilities);
  static bool selectNumberOfSwapchainImages(
      VkSurfaceCapabilitiesKHR const& surface_capabilities,
      uint32_t& number_of_images);
  static bool chooseSizeOfSwapchainImages(
      VkSurfaceCapabilitiesKHR const& surface_capabilities,
      VkExtent2D& size_of_images);
  static bool selectDesiredUsageScenariosOfSwapchainImages(
      VkSurfaceCapabilitiesKHR const& surface_capabilities,
      VkImageUsageFlags desired_usages, VkImageUsageFlags& image_usage);
  static bool selectTransformationOfSwapchainImages(
      VkSurfaceCapabilitiesKHR const& surface_capabilities,
      VkSurfaceTransformFlagBitsKHR desired_transform,
      VkSurfaceTransformFlagBitsKHR& surface_transform);
  static bool selectFormatOfSwapchainImages(
      VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
      VkSurfaceFormatKHR desired_surface_format, VkFormat& image_format,
      VkColorSpaceKHR& image_color_space);
  static bool createSwapchain(
      VkDevice logical_device, VkSurfaceKHR presentation_surface,
      uint32_t image_count, VkSurfaceFormatKHR surface_format,
      VkExtent2D image_size, VkImageUsageFlags image_usage,
      VkSurfaceTransformFlagBitsKHR surface_transform,
      VkPresentModeKHR present_mode, VkSwapchainKHR& old_swapchain,
      VkSwapchainKHR& swapchain);
  static bool getHandlesOfSwapchainImages(
      VkDevice logical_device, VkSwapchainKHR swapchain,
      std::vector<VkImage>& swapchain_images);
  static bool createSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(
      VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
      VkDevice logical_device, VkImageUsageFlags swapchain_image_usage,
      VkExtent2D& image_size, VkFormat& image_format,
      VkSwapchainKHR& old_swapchain, VkSwapchainKHR& swapchain,
      std::vector<VkImage>& swapchain_images);
  static bool acquireSwapchainImage(VkDevice logical_device,
                                    VkSwapchainKHR swapchain,
                                    VkSemaphore semaphore, VkFence fence,
                                    uint32_t& image_index);
  static bool presentImage(VkQueue queue,
                           std::vector<VkSemaphore> rendering_semaphores,
                           std::vector<PresentInfo> images_to_present);
  static void destroySwapchain(VkDevice logical_device,
                               VkSwapchainKHR& swapchain);
  static void destroyPresentationSurface(VkInstance instance,
                                         VkSurfaceKHR& presentation_surface);
  static bool createCommandPool(VkDevice logical_device,
                                VkCommandPoolCreateFlags parameters,
                                uint32_t queue_family,
                                VkCommandPool& command_pool);
  static bool allocateCommandBuffers(
      VkDevice logical_device, VkCommandPool command_pool,
      VkCommandBufferLevel level, uint32_t count,
      std::vector<VkCommandBuffer>& command_buffers);
  static bool beginCommandBufferRecordingOperation(
      VkCommandBuffer command_buffer, VkCommandBufferUsageFlags usage,
      VkCommandBufferInheritanceInfo* secondary_command_buffer_info);
  static bool endCommandBufferRecordingOperation(
      VkCommandBuffer command_buffer);
  static bool resetCommandBuffer(VkCommandBuffer command_buffer,
                                 bool release_resources);
  static bool VulkanLibrary::resetCommandPool(VkDevice logical_device,
                                              VkCommandPool command_pool,
                                              bool release_resources);
  static bool createSemaphore(VkDevice logical_device, VkSemaphore& semaphore);
  static bool createFence(VkDevice logical_device, bool signaled,
                          VkFence& fence);
  static bool waitForFences(VkDevice logical_device,
                            std::vector<VkFence> const& fences,
                            VkBool32 wait_for_all, uint64_t timeout);
  static bool resetFences(VkDevice logical_device,
                          std::vector<VkFence> const& fences);
  static bool submitCommandBuffersToQueue(
      VkQueue queue, std::vector<WaitSemaphoreInfo> wait_semaphore_infos,
      std::vector<VkCommandBuffer> command_buffers,
      std::vector<VkSemaphore> signal_semaphores, VkFence fence);
  static bool synchronizeTwoCommandBuffers(
      VkQueue first_queue,
      std::vector<WaitSemaphoreInfo> first_wait_semaphore_infos,
      std::vector<VkCommandBuffer> first_command_buffers,
      std::vector<WaitSemaphoreInfo> synchronizing_semaphores,
      VkQueue second_queue, std::vector<VkCommandBuffer> second_command_buffers,
      std::vector<VkSemaphore> second_signal_semaphores, VkFence second_fence);
  static bool checkIfProcessingOfSubmittedCommandBufferHasFinished(
      VkDevice logical_device, VkQueue queue,
      std::vector<WaitSemaphoreInfo> wait_semaphore_infos,
      std::vector<VkCommandBuffer> command_buffers,
      std::vector<VkSemaphore> signal_semaphores, VkFence fence,
      uint64_t timeout, VkResult& wait_status);
  static bool waitUntilAllCommandsSubmittedToQueueAreFinished(VkQueue queue);
  static bool waitForAllSubmittedCommandsToBeFinished(VkDevice logical_device);
  static void destroyFence(VkDevice logical_device, VkFence& fence);
  static void destroySemaphore(VkDevice logical_device, VkSemaphore& semaphore);
  static void freeCommandBuffers(VkDevice logical_device,
                                 VkCommandPool command_pool,
                                 std::vector<VkCommandBuffer>& command_buffers);
  static void destroyCommandPool(VkDevice logical_device,
                                 VkCommandPool& command_pool);

 private:
  VulkanLibraryType vulkanLibrary_;
};

}  // namespace aergia

#endif  // AERGIA_VULKAN_LIBRARY