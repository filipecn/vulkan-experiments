/// Copyright (c) 2019, FilipeCN.
///
/// The MIT License (MIT)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
/// \file vulkan_library.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-08-03
///
/// The contets were based on the Vulkan Cookbook 2017.
///
/// \brief

#ifndef CIRCE_VULKAN_LIBRARY
#define CIRCE_VULKAN_LIBRARY

#include "vulkan_api.h"
#include <optional>
#include <sstream>
#include <vector>

#if defined _WIN32
#include <windows.h>
#define VulkanLibraryType HMODULE
#elif __linux
#define VulkanLibraryType void *
#elif __APPLE__
#define VulkanLibraryType void *
#endif

namespace circe {

namespace vk {

inline void concatenate(std::ostringstream &s) {}

template <typename H, typename... T>
void concatenate(std::ostringstream &s, H p, T... t) {
  s << p;
  concatenate(s, t...);
}

template <class... Args> std::string concat(const Args &... args) {
  std::ostringstream s;
  concatenate(s, args...);
  return s.str();
}

/// Holds a handle to the Vulkan API. Provides a set of helper functions
/// to perform all Vulkan operations.
class VulkanLibraryInterface {
public:
  /// Stores information about queues requested to a logical device and the list
  /// of priorities assifned to each queue
  struct QueueFamilyInfo {
    std::optional<uint32_t> family_index; //!< queue family index
    std::vector<float> priorities; //!< list of queue priorities, [0.0,1.0]
  };
  struct PhysicalDevice {
    VkPhysicalDevice handle;
    VkPhysicalDeviceFeatures &features;
    VkPhysicalDeviceProperties &properties;
    std::vector<VkQueueFamilyProperties> queue_families;
  };
  struct WindowParameters {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    HINSTANCE HInstance;
    HWND HWnd;
#elif defined VK_USE_PLATFORM_XLIB_KHR
    Display *Dpy;
    Window Window;
#elif defined VK_USE_PLATFORM_XCB_KHR
    xcb_connection_t *Connection;
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

  // VULKAN API FUNCTION LOADING
  // ---------------------------
  // Vulkan API provides a helper function that can be used to load its function
  // pointers across different platforms. Vulkan functions can be divided into 3
  // levels: global, instance and device.
  // Device-level: drawing, shader module creation, data copying.
  // Instance-level: create logical devices.
  // Global-level: load device and instance-level functions.

  /// Retrieves the pointer to the helper Vulkan API function that is used to
  /// load all other functions across platforms.
  /// \param vulkan_library **[out]** function loader's pointer.
  /// \return true if success
  static bool loadLoaderFunctionFromVulkan(VulkanLibraryType &vulkan_library);
  /// Since we load the Vulkan Library dinamically, we must explicitly release
  /// it.
  /// \param vulkan_library **[in/out]** function loader's pointer.
  static void releaseVulkanLoaderLibrary(VulkanLibraryType &vulkan_library);
  /// Loads all global level functions listed in the vulkan_api files.
  /// Global-level functions can be used to perform operations such as drawing,
  /// shader modules creation, data copying.
  /// \return bool true if success
  static bool loadGlobalLevelFunctions();
  /// Loads all instance level functions listed in the vulkan_api files.
  /// Instance-level functions can be used to create logical devices.
  /// \param instance **[in]**
  /// \param extensions **[in]**
  /// \return bool true if success
  static bool
  loadInstanceLevelFunctions(VkInstance instance,
                             const std::vector<const char *> &extensions);
  /// Loads all device level functions listed in the vulkan_api files.
  /// Device-level functions can be used for rendering, calculating collisions
  /// of objects, processing video frames, etc.
  /// \param logical_device **[in]** logical device handle
  /// \param enabled_extensions **[in]** enabled extensions for the logical
  /// device
  /// \return bool true if success
  static bool
  loadDeviceLevelFunctions(VkDevice logical_device,
                           std::vector<char const *> const &enabled_extensions);

  // VULKAN API EXTENSIONS
  // ---------------------
  // Since Vulkan is an platform agnostic api, it means we need extensions
  // to communicate with system specifics features. For example, an extension
  // to interface the application with the window system.
  // There are 2 levels of extensions: intance-level and device-level.
  // Instance Level Extensions:   enabled on instance creation
  // Device Level Extensions: enabled on logical device creation
  // In order to create a Vulkan Instance(or logical device) with the desired
  // extensions, we first need to check if the extensions are supported on the
  // current hardware platform. This is done by checking if the extensions we
  // want are listed by the respective supported extensions.

  /// Gets the list of the properties of supported instance extensions on the
  /// current hardware platform.
  /// \param extensions **[out]** list of extensions
  /// \return bool true if success
  static bool checkAvaliableInstanceExtensions(
      std::vector<VkExtensionProperties> &extensions);
  /// Gets the list of the properties of supported extensions for the device.
  /// \param physical_device **[in]** physical device handle
  /// \param extensions **[out]** list of extensions
  /// \return bool true if success
  static bool checkAvailableDeviceExtensions(
      VkPhysicalDevice physical_device,
      std::vector<VkExtensionProperties> &extensions);
  /// Checks if **extension_name** is listed by **extensions**
  /// \param extensions **[in]** list of supported extensions
  /// \param extension_name **[in]** extension name
  /// \return bool true if extension is a supported extension
  static bool
  isExtensionSupported(const std::vector<VkExtensionProperties> &extensions,
                       const char *extension_name);

  // VULKAN INSTANCE
  // ---------------
  // The Vulkan Instance holds all kind of information about the application,
  // such as application name, version, etc. The instance is the interface
  // between the application and the Vulkan Library, that can perform
  // operations like the enumeration of available physical devices and creation
  // of logical devices.

  /// \brief Create a Vulkan Instance object
  /// A Vulkan Instance represents the application state and connects the
  /// application with the Vulkan Library.
  /// \param extensions **[in]** list of desired instance extensions
  /// \param application_name **[in]** application's name
  /// \param instance **[out]** vulkan instance handle
  /// \return bool if success
  static bool createInstance(const std::vector<const char *> &extensions,
                             std::string application_name,
                             VkInstance &instance);
  /// Clean up the resources used by the Vulkan Instance and destroys its
  /// handle. Must be called after the destruction of all its dependencies
  /// (logical devices).
  /// \param instance **[in/out]** Vulkan Instance handle
  static void destroyVulkanInstance(VkInstance &instance);

  // VULKAN PHYSICAL DEVICE
  // ----------------------
  // Physical devices are the hardware we intend to use with Vulkan. Thus we
  // need to look for the devices that supports the features we need. We can
  // select any number of graphics cards and use them simultaneously.
  // The Vulkan Library allows us to get devices capabilities and properties so
  // we can select the one that best suits for our application.

  /// Enumarates the available physical devices in the system that support
  /// the requirements of the application.
  /// \param instance **[in]** instance object (describing our needs)
  /// \param devices **[out]** physical device handles
  /// \return bool true if success
  static bool
  enumerateAvailablePhysicalDevices(VkInstance instance,
                                    std::vector<VkPhysicalDevice> &devices);
  /// Retrieves the physical device capabilities and features. Device properties
  /// describe general information surch as name, version of a driver, type of
  /// the device (integrated or discrete), memory, etc. Device features include
  /// items such as geometry and tesselation shaders, depth clamp, etc.
  /// \param physical_device **[in]** physical device handle
  /// \param device_features **[out]** device features
  /// \param device_properties **[out]** device properties
  static void getFeaturesAndPropertiesOfPhysicalDevice(
      VkPhysicalDevice physical_device,
      VkPhysicalDeviceFeatures &device_features,
      VkPhysicalDeviceProperties &device_properties);

  // VULKAN QUEUE FAMILIES
  // ---------------------
  // Every Vulkan operation requires commands that are submitted to a queue.
  // Different queues can be processed independently and may support different
  // types of operations.
  // Queues with the same capabilities are grouped into families. A device may
  // expose any number of queue families. For example, there could be a queue
  // family that only allows processing of compute commands or one that only
  // allows memory transfer related commands.

  /// Retrieve available queue families exposed by a physical device
  /// \param physical_device **[in]** physical device handle
  /// \param queue_families **[in]** list of exposed queue families
  /// \return bool true if success
  static bool checkAvailableQueueFamiliesAndTheirProperties(
      VkPhysicalDevice physical_device,
      std::vector<VkQueueFamilyProperties> &queue_families);
  /// Given a physical device, finds the queue family index that supports the
  /// desired set of capabilities.
  /// \param physical_device **[in]** physical device handle
  /// \param desired_capabilities **[in]** desired set of capabalities
  /// \param queue_family_index **[in]** capable queue family index
  /// \return bool true if success
  static bool selectIndexOfQueueFamilyWithDesiredCapabilities(
      VkPhysicalDevice physical_device, VkQueueFlags desired_capabilities,
      uint32_t &queue_family_index);
  /// \brief
  ///
  /// \param physical_device **[in]**
  /// \param presentation_surface **[in]**
  /// \param queue_family_index **[in]**
  /// \return bool
  static bool selectQueueFamilyThatSupportsPresentationToGivenSurface(
      VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
      uint32_t &queue_family_index);

  // VULKAN LOGICAL DEVICE
  // ---------------------
  // The logical device makes the interface of the application and the physical
  // device. Represents the hardware, along with the extensions and features
  // enabled for it and all the queues requested from it. The logical device
  // allows us to record commands, submit them to queues and acquire the
  // results.
  // Device queues need to be requested on device creation, we cannot create or
  // destroy queues explicitly. They are created/destroyed on logical device
  // creation/destruction.

  /// \brief Creates a Logical Device handle
  /// \param physical_device **[in]** physical device handle
  /// \param queue_infos **[in]** queues description
  /// \param desired_extensions **[in]** desired extensions
  /// \param desired_features **[in]** desired features
  /// \param logical_device **[out]** logical device handle
  /// \return bool true if success
  static bool
  createLogicalDevice(VkPhysicalDevice physical_device,
                      std::vector<QueueFamilyInfo> queue_infos,
                      std::vector<char const *> const &desired_extensions,
                      VkPhysicalDeviceFeatures *desired_features,
                      VkDevice &logical_device);
  /// Clean up the resources used by the logical device and destroys its handle.
  /// Must be destroyed before destroy the Vulkan Instance.
  /// \param logical_device **[in/out]** logical device handle
  static void destroyLogicalDevice(VkDevice &logical_device);
  /// \param logical_device **[in]** logical device handle
  /// \param queue_family_index **[in]** queue family index
  /// \param queue_index **[in]** queue index
  /// \param queue **[out]** queue
  static void getDeviceQueue(VkDevice logical_device,
                             uint32_t queue_family_index, uint32_t queue_index,
                             VkQueue &queue);

  // VULKAN SURFACE
  // --------------
  // Vulkan does not provide a way to display images in the application's
  // window by default. We need extensions to do so. These extensions are
  // commonly referred as Windowing System Integration (WSI) and each operating
  // system has its own set of extensions.
  // The presentation surface is the Vulkan representation of an application's
  // window. Instance-level extensions are responsable for managing, creating,
  // and destroying a presentation surface.

  /// \brief Create a Presentation Surface handle
  /// \param instance **[in]** vulkan instance handle
  /// \param window_parameters **[in]** window parameters
  /// \param presentation_surface **[out]** presentation surface handle
  /// \return bool true if success
  static bool createPresentationSurface(VkInstance instance,
                                        WindowParameters window_parameters,
                                        VkSurfaceKHR &presentation_surface);
  // UTILS

  static bool createLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues(
      VkInstance instance, VkDevice &logical_device, VkQueue &graphics_queue,
      VkQueue &compute_queue);
  static bool createVulkanInstanceWithWsiExtensionsEnabled(
      std::vector<char const *> &desired_extensions,
      char const *const application_name, VkInstance &instance);
  static bool createLogicalDeviceWithWsiExtensionsEnabled(
      VkPhysicalDevice physical_device,
      std::vector<QueueFamilyInfo> queue_infos,
      std::vector<char const *> &desired_extensions,
      VkPhysicalDeviceFeatures *desired_features, VkDevice &logical_device);
  static bool selectDesiredPresentationMode(
      VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
      VkPresentModeKHR desired_present_mode, VkPresentModeKHR &present_mode);
  static bool getCapabilitiesOfPresentationSurface(
      VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
      VkSurfaceCapabilitiesKHR &surface_capabilities);
  static bool selectNumberOfSwapchainImages(
      VkSurfaceCapabilitiesKHR const &surface_capabilities,
      uint32_t &number_of_images);
  static bool chooseSizeOfSwapchainImages(
      VkSurfaceCapabilitiesKHR const &surface_capabilities,
      VkExtent2D &size_of_images);
  static bool selectDesiredUsageScenariosOfSwapchainImages(
      VkSurfaceCapabilitiesKHR const &surface_capabilities,
      VkImageUsageFlags desired_usages, VkImageUsageFlags &image_usage);
  static bool selectTransformationOfSwapchainImages(
      VkSurfaceCapabilitiesKHR const &surface_capabilities,
      VkSurfaceTransformFlagBitsKHR desired_transform,
      VkSurfaceTransformFlagBitsKHR &surface_transform);
  static bool selectFormatOfSwapchainImages(
      VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
      VkSurfaceFormatKHR desired_surface_format, VkFormat &image_format,
      VkColorSpaceKHR &image_color_space);
  static bool
  createSwapchain(VkDevice logical_device, VkSurfaceKHR presentation_surface,
                  uint32_t image_count, VkSurfaceFormatKHR surface_format,
                  VkExtent2D image_size, VkImageUsageFlags image_usage,
                  VkSurfaceTransformFlagBitsKHR surface_transform,
                  VkPresentModeKHR present_mode, VkSwapchainKHR &old_swapchain,
                  VkSwapchainKHR &swapchain);
  static bool
  getHandlesOfSwapchainImages(VkDevice logical_device, VkSwapchainKHR swapchain,
                              std::vector<VkImage> &swapchain_images);
  static bool createSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(
      VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
      VkDevice logical_device, VkImageUsageFlags swapchain_image_usage,
      VkExtent2D &image_size, VkFormat &image_format,
      VkSwapchainKHR &old_swapchain, VkSwapchainKHR &swapchain,
      std::vector<VkImage> &swapchain_images);
  static bool acquireSwapchainImage(VkDevice logical_device,
                                    VkSwapchainKHR swapchain,
                                    VkSemaphore semaphore, VkFence fence,
                                    uint32_t &image_index);
  static bool presentImage(VkQueue queue,
                           std::vector<VkSemaphore> rendering_semaphores,
                           std::vector<PresentInfo> images_to_present);
  static void destroySwapchain(VkDevice logical_device,
                               VkSwapchainKHR &swapchain);
  static void destroyPresentationSurface(VkInstance instance,
                                         VkSurfaceKHR &presentation_surface);
  static bool createCommandPool(VkDevice logical_device,
                                VkCommandPoolCreateFlags parameters,
                                uint32_t queue_family,
                                VkCommandPool &command_pool);
  static bool
  allocateCommandBuffers(VkDevice logical_device, VkCommandPool command_pool,
                         VkCommandBufferLevel level, uint32_t count,
                         std::vector<VkCommandBuffer> &command_buffers);
  static bool beginCommandBufferRecordingOperation(
      VkCommandBuffer command_buffer, VkCommandBufferUsageFlags usage,
      VkCommandBufferInheritanceInfo *secondary_command_buffer_info);
  static bool
  endCommandBufferRecordingOperation(VkCommandBuffer command_buffer);
  static bool resetCommandBuffer(VkCommandBuffer command_buffer,
                                 bool release_resources);
  static bool resetCommandPool(VkDevice logical_device,
                               VkCommandPool command_pool,
                               bool release_resources);
  static bool createSemaphore(VkDevice logical_device, VkSemaphore &semaphore);
  static bool createFence(VkDevice logical_device, bool signaled,
                          VkFence &fence);
  static bool waitForFences(VkDevice logical_device,
                            std::vector<VkFence> const &fences,
                            VkBool32 wait_for_all, uint64_t timeout);
  static bool resetFences(VkDevice logical_device,
                          std::vector<VkFence> const &fences);
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
      uint64_t timeout, VkResult &wait_status);
  static bool waitUntilAllCommandsSubmittedToQueueAreFinished(VkQueue queue);
  static bool waitForAllSubmittedCommandsToBeFinished(VkDevice logical_device);
  static void destroyFence(VkDevice logical_device, VkFence &fence);
  static void destroySemaphore(VkDevice logical_device, VkSemaphore &semaphore);
  static void freeCommandBuffers(VkDevice logical_device,
                                 VkCommandPool command_pool,
                                 std::vector<VkCommandBuffer> &command_buffers);
  static void destroyCommandPool(VkDevice logical_device,
                                 VkCommandPool &command_pool);

private:
  VulkanLibraryType vulkanLibrary_;
};

} // namespace vk

} // namespace circe

#endif // CIRCE_VULKAN_LIBRARY