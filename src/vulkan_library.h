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

#ifndef GLFW_INCLUDE_VULKAN
// #include "vulkan_api.h"
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
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
  /// Stores swap chain support information:
  /// - Basic surface capabilities (min/max number of images in swap chain,
  /// min/max width and height of images)
  /// - Surface formats (pixel format, color space)
  /// - Available presentation modes
  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
  };
  ///
  struct PhysicalDevice {
    VkPhysicalDevice handle;
    VkPhysicalDeviceFeatures &features;
    VkPhysicalDeviceProperties &properties;
    std::vector<VkQueueFamilyProperties> queue_families;
  };
  ///
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
  /// Used in the presentation of an image in a given swapchain, for each
  /// swapchain only one image can be presented at a time.
  struct PresentInfo {
    VkSwapchainKHR Swapchain;
    uint32_t ImageIndex;
  };
  /// Stores a semaphore on which hardware should wait and on what pipeline
  /// stages the wait should occur.
  struct WaitSemaphoreInfo {
    VkSemaphore Semaphore;
    VkPipelineStageFlags WaitingStage;
  };
  /// Defines parameters to use for buffer memory barrier
  struct BufferTransition {
    VkBuffer Buffer;
    VkAccessFlags CurrentAccess; //!< how the buffer has been used so far
    VkAccessFlags NewAccess;     //!< how the buffer will be used from now on
    uint32_t CurrentQueueFamily; //!< queue family owning the buffer
    uint32_t NewQueueFamily; //!< queue family that will receive the ownership
  };
  /// Defines parameters to use for image memory barrier
  struct ImageTransition {
    VkImage Image;
    VkAccessFlags CurrentAccess; //!< how the image has been used so far
    VkAccessFlags NewAccess;     //!< how the image will be used from now on
    VkImageLayout CurrentLayout; //!< how the image has been organized in memory
    VkImageLayout NewLayout;     //!< how the image will be organized in memory
    uint32_t CurrentQueueFamily; //!< queue family owning the image
    uint32_t NewQueueFamily; //!< queue family that will receive the ownership
    VkImageAspectFlags
        Aspect; //!< image's usage contex (color, depth or stencil)
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
  /// Acquires physical device memory properties, such as number of heaps, their
  /// sizes, types and etc.
  /// \param physical_device **[in]** physical device handler
  /// \param properties **[out]** physical device memory properties
  static void getPhysicalDeviceMemoryProperties(
      VkPhysicalDevice physical_device,
      VkPhysicalDeviceMemoryProperties &properties);

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
  /// Finds a queue family of a physical device that can accept commands for a
  /// given surface
  /// \param physical_device **[in]** physical device handle
  /// \param presentation_surface **[in]** surface handle
  /// \param queue_family_index **[out]** queue family index
  /// \return bool true if success
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

  // VULKAN RESOURCES AND MEMORY
  // ---------------------------
  // Vulkan uses memory objects to provide resources such as buffers and images.
  // Buffers represent linear arrays and images can represent up to
  // three-dimensional data arranged in ways specific to hardware. These
  // resources serve to various purposes (such as shader data, render targets,
  // etc). These usages must be specified on resource's creation.
  // The driver must be informed about the usage not only during the
  // buffer/image creation, but also before the actual usage. It is because we
  // might want to change the usage of the buffer/image during execution. This
  // is done by memory barriers, which are set as part of the pipeline barriers
  // during command buffer recording.
  // Memory barriers are very important to assure that commands read content
  // from buffers/images properly, i.e. that the commands that write into the
  // memory finish their job before the read.
  // Vulkan also provides image layouts. Depending on the usage of the image,
  // it may be organized in memory differently to optimize access to it. We
  // can also change the image layout during an image memory barrier.

  /// Iterates over available physical device's memory types and check against
  /// the desired memory properties (number of heaps, their sizes and types),
  /// then allocate and binds it to the given buffer.
  /// \param physical_device **[in]** physical device handle which the logical
  /// device was created
  /// \param logical_device **[in]** logical device handle created from the
  /// physical device
  /// \param buffer **[in]** buffer handle
  /// \param memory_properties **[in]** desired memory properties
  /// \param memory_object **[out]** allocated memory object
  /// \return bool true if success
  static bool allocateAndBindMemoryObjectToBuffer(
      VkPhysicalDevice physical_device, VkDevice logical_device,
      VkBuffer buffer, VkMemoryPropertyFlagBits memory_properties,
      VkDeviceMemory &memory_object);
  /// Setups buffer memory barriers
  /// \param command_buffer **[in]** command buffer handle (in recording stage)
  /// \param generating_stages **[in]** pipeline stages that have been using the
  /// buffer so far
  /// \param consuming_stages **[in]** pipeline stages in which the buffer will
  /// be used after the barrier
  /// \param buffer_transitions **[in]** parameters for each buffer that a
  /// barrier will be set up for
  void setBufferMemoryBarrier(VkCommandBuffer command_buffer,
                              VkPipelineStageFlags generating_stages,
                              VkPipelineStageFlags consuming_stages,
                              std::vector<BufferTransition> buffer_transitions);
  /// Iterates over available physical device's memory types and check against
  /// the desired memory properties (number of heaps, their sizes and types),
  /// then allocate and binds it to the given image.
  /// \param physical_device **[in]** physical device handle which the logical
  /// device was created
  /// \param logical_device **[in]** logical device handle created from the
  /// physical device
  /// \param image **[in]** image handle
  /// \param memory_properties **[in]** desired memory properties
  /// \param memory_object **[out]** allocated memory object
  /// \return bool true if success
  static bool
  allocateAndBindMemoryObjectToImage(VkPhysicalDevice physical_device,
                                     VkDevice logical_device, VkImage image,
                                     VkMemoryPropertyFlagBits memory_properties,
                                     VkDeviceMemory &memory_object);
  /// Setups image memory barriers
  /// \param command_buffer **[in]** command buffer handle (in recording stage)
  /// \param generating_stages **[in]** pipeline stages that have been using the
  /// image so far
  /// \param consuming_stages **[in]** pipeline stages in which the image will
  /// be used after the barrier
  /// \param image_transitions **[in]** parameters for each image that a
  /// barrier will be set up for
  static void VulkanLibraryInterface::SetImageMemoryBarrier(
      VkCommandBuffer command_buffer, VkPipelineStageFlags generating_stages,
      VkPipelineStageFlags consuming_stages,
      std::vector<ImageTransition> image_transitions);
  /// \param logical_device **[in]** logical device handle
  /// \param buffer **[in/out]** buffer handle
  static void destroyBuffer(VkDevice logical_device, VkBuffer &buffer);
  /// \param logical_device **[in]** logical device handle
  /// \param image **[in/out]** image handle
  static void destroyImage(VkDevice logical_device, VkImage &image);
  /// \param logical_device **[in]** logical device handle
  /// \param memory_object **[in/out]** memory object
  static void freeMemoryObject(VkDevice logical_device,
                               VkDeviceMemory &memory_object);

  // VULKAN BUFFER VIEW
  // -----------------
  // Buffer views allow us to define how buffer's memory is accessed and
  // interpreted. For example, we can choose to look at the buffer as a uniform
  // texel buffer or as a storage texel buffer.

  /// Creates a buffer view of a portion of the given buffer
  /// \param logical_device **[in]** logical device handle
  /// \param buffer **[in]** buffer handle
  /// \param format **[in]** how buffer contents should be interpreted
  /// \param memory_offset **[in]** view's starting point
  /// \param memory_range **[in]** size of the view
  /// \param buffer_view **[out]** buffer view object
  /// \return bool true if success
  static bool createBufferView(VkDevice logical_device, VkBuffer buffer,
                               VkFormat format, VkDeviceSize memory_offset,
                               VkDeviceSize memory_range,
                               VkBufferView &buffer_view);
  /// \param logical_device **[in]** logical device handle
  /// \param buffer_view **[in/out]** buffer view handle
  static void destroyBufferView(VkDevice logical_device,
                                VkBufferView &buffer_view);

  // VULKAN IMAGE VIEW
  // -----------------
  // Image views define a selected part of an image's memory and specify
  // additional information needed to properly read an image's data.

  /// Creates an image view object for the entire given image
  /// \param logical_device **[in]** logical device handle
  /// \param image **[in]** image handle
  /// \param view_type **[in]**
  /// \param format **[in]** data format
  /// \param aspect **[in]** context: color, depth or stencil
  /// \param image_view **[out]** image view object
  /// \return bool true if success
  static bool createImageView(VkDevice logical_device, VkImage image,
                              VkImageViewType view_type, VkFormat format,
                              VkImageAspectFlags aspect,
                              VkImageView &image_view);
  /// \param logical_device **[in]** logical device handle
  /// \param image_view **[in/out]** image view handle
  static void DestroyImageView(VkDevice logical_device,
                               VkImageView &image_view);

  // VULKAN SURFACE
  // --------------
  // Vulkan does not provide a way to display images in the application's
  // window by default. We need extensions to do so. These extensions are
  // commonly referred as Windowing System Integration (WSI) and each
  // operating system has its own set of extensions. The presentation surface
  // is the Vulkan representation of an application's window. Instance-level
  // extensions are responsable for managing, creating, and destroying a
  // presentation surface.

  /// \brief Create a Presentation Surface handle
  /// \param instance **[in]** vulkan instance handle
  /// \param window_parameters **[in]** window parameters
  /// \param presentation_surface **[out]** presentation surface handle
  /// \return bool true if success
  static bool createPresentationSurface(VkInstance instance,
                                        WindowParameters window_parameters,
                                        VkSurfaceKHR &presentation_surface);
  /// \param instance **[in]** instance handle
  /// \param presentation_surface **[in/out]** surface handle
  static void destroyPresentationSurface(VkInstance instance,
                                         VkSurfaceKHR &presentation_surface);
  /// \brief Retrieve the capabilities of the surface
  /// \param physical_device **[in]** physical device handle
  /// \param presentation_surface **[in]** surface handle
  /// \param surface_capabilities **[out]** surface capabilities
  /// \return bool true if success
  static bool getCapabilitiesOfPresentationSurface(
      VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
      VkSurfaceCapabilitiesKHR &surface_capabilities);

  // VULKAN SWAP CHAIN
  // -----------------
  // Different from other high level libraries, such as OpenGL, Vulkan does
  // not have a system of framebuffers. In order to control the buffers that
  // are rendered and presented on the display, Vulkan provides a mechanism
  // called swap chain. The Vulkan swapchain is a queue of images that are
  // presented to the screen in a synchronized manner, following the rules and
  // properties defined on its setup. The swapchain is owned by the
  // presentation engine, and not by the application. We can't create the
  // images or destroy them, all the application does is to request images, do
  // work and give it back to the presentation engine. In order to use the
  // swapchain, the device has to support the VK_KHR_swapchain extension. The
  // swapchain works following a presentation mode. The presentation mode
  // defines the format of an image, the number of images (double/triple
  // buffering), v-sync and etc. In other words, it defines how images are
  // displayed on screen. Vulkan provides 4 presentation modes:
  // 1. IMMEDIATE mode
  //    The image to be presented immediately replaces the image that is being
  //    displayed. Screen tearing may happen when using this mode.
  // 2. FIFO mode
  //    When a image is presented, it is added to the queue. Images are
  //    displayed on screen in sync with blanking periods (v-sync). This mode
  //    is similar to OpenGL's buffer swap.
  // 3. (FIFO) RELAXED mode
  //    Images are displayed with blanking periods only when are faster than
  //    the refresh rate.
  // 4. MAILBOX mode (triple buffering)
  //    There is a queue with just one element. An image waiting in this queue
  //    is displayed in sync with blanking periods. When the application
  //    presents an image, the new image replaces the one waiting in the
  //    queue. So the displayed image is always the most recent available.

  /// Checks if the desired presentation mode is supported by the device, if
  /// so, it is returned in **present_mode**. If not, VK_PRESENT_MODE_FIFO_KHR
  /// is chosen. \param physical_device **[in]** physical device handle \param
  /// presentation_surface **[in]** surface handle \param desired_present_mode
  /// **[in]** described presentation mode \param present_mode **[out]**
  /// available presentation mode \return bool true if success
  static bool selectDesiredPresentationMode(
      VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
      VkPresentModeKHR desired_present_mode, VkPresentModeKHR &present_mode);
  /// \brief Gets swap chain support information for a surface in a given
  /// physical device.
  /// \param physical_device **[in]** physical device handle
  /// \param presentation_surface **[in]** surface handle
  /// \param details **[out]** support information
  /// \return bool true if success
  static bool querySwapChainSupport(VkPhysicalDevice physical_device,
                                    VkSurfaceKHR surface,
                                    SwapChainSupportDetails &details);
  /// Computes the minimal value + 1, of images required for the presentation
  /// engine to work properly.
  /// \param surface_capabilities **[in]** surface capabilities
  /// \param number_of_images **[out]** available number of images
  /// \return bool true if success
  static bool selectNumberOfSwapchainImages(
      const VkSurfaceCapabilitiesKHR &surface_capabilities,
      uint32_t &number_of_images);
  /// Clamps the **size_of_images** to the maximum supported by the surface
  /// capabilities.
  /// \param surface_capabilities **[in]** surface capabilities
  /// \param size_of_images **[in/out]** desired/available size of images
  /// \return bool true if success
  static bool chooseSizeOfSwapchainImages(
      VkSurfaceCapabilitiesKHR const &surface_capabilities,
      VkExtent2D &size_of_images);
  /// \brief Clamps the usage flag to the surface capabilities
  /// Images can also be used for purposes other than as color attachments.
  /// For example, we can sample from them and use them in copy operations.
  /// \param surface_capabilities **[in]** surface capabilities
  /// \param desired_usages **[in]** desired usages
  /// \param image_usage **[in/out]** available usages
  /// \return bool true if success
  static bool selectDesiredUsageScenariosOfSwapchainImages(
      VkSurfaceCapabilitiesKHR const &surface_capabilities,
      VkImageUsageFlags desired_usages, VkImageUsageFlags &image_usage);
  /// \brief Clamps the desired image orientation to the supported image
  /// orientation.
  /// On some devices, images can be displayed in different orientations.
  /// \param surface_capabilities **[in]** surface capabilities
  /// \param desired_transform **[in]** desired image orientation
  /// \param surface_transform **[in]** available image orientation
  /// \return bool true if success
  static bool selectTransformationOfSwapchainImages(
      VkSurfaceCapabilitiesKHR const &surface_capabilities,
      VkSurfaceTransformFlagBitsKHR desired_transform,
      VkSurfaceTransformFlagBitsKHR &surface_transform);
  /// Clamps the desired image format to the supported format by the
  /// device. The format defines the number of color components, the number of
  /// bits for each component and data type. Also, we must specify the color
  /// space to be used for encoding color.
  /// \param physical_device **[in]** physical device handle
  /// \param presentation_surface **[in]** surface handle
  /// \param desired_surface_format **[in]** desired image format
  /// \param image_format **[out]** available image format
  /// \param image_color_space **[out]** available color space
  /// \return bool true if success
  static bool selectFormatOfSwapchainImages(
      VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
      VkSurfaceFormatKHR desired_surface_format, VkFormat &image_format,
      VkColorSpaceKHR &image_color_space);
  /// \brief Create a Swapchain object
  /// \param logical_device **[in]** logical device handle
  /// \param presentation_surface **[in]** surface handle
  /// \param image_count **[in]** swapchain image count
  /// \param surface_format **[in]** surface format
  /// \param image_size **[in]** image size
  /// \param image_usage **[in]** image usages
  /// \param surface_transform **[in]** surface transform
  /// \param present_mode **[in]** presentation mode
  /// \param old_swapchain **[in | optional]** old swap chain handle
  /// \param swapchain **[out]** new swap chain handle
  /// \return bool true if success
  static bool
  createSwapchain(VkDevice logical_device, VkSurfaceKHR presentation_surface,
                  uint32_t image_count, VkSurfaceFormatKHR surface_format,
                  VkExtent2D image_size, VkImageUsageFlags image_usage,
                  VkSurfaceTransformFlagBitsKHR surface_transform,
                  VkPresentModeKHR present_mode, VkSwapchainKHR &old_swapchain,
                  VkSwapchainKHR &swapchain);
  /// Needs to be destroyed before the presentation surface
  /// \param logical_device **[in]** logical device handle
  /// \param swapchain **[in/out]** swapchain handle
  static void destroySwapchain(VkDevice logical_device,
                               VkSwapchainKHR &swapchain);

  // SWAPCHAIN IMAGE OPERATIONS
  // --------------------------
  // In order to use the images contained in the swapchain, we need to acquire
  // them first. When acquiring images, we can use semaphores and fences.
  // Semaphores can be used in internal queue synchronization. Fences are used
  // to synchronize the queues and the application.
  // After we use the image, we need to give it back to the presentation
  // engine so it can be displayed on screen. The type of access on the images
  // is described by the image view object, which defines the portion of the
  // image to be accessed and how it will be accessed (for example, if it
  // should be treated as a 2D depth texture with mipmap levels).

  /// \brief Get the handle list of swapchain images
  /// \param logical_device **[in]** logical device handle
  /// \param swapchain **[in]** swapchain handle
  /// \param swapchain_images **[out]** list of swapchain image handles
  /// \return bool true if success
  static bool
  getHandlesOfSwapchainImages(VkDevice logical_device, VkSwapchainKHR swapchain,
                              std::vector<VkImage> &swapchain_images);
  /// Acquires an image index (in the array returned by the
  /// **getHandlesOfSwapchainImages** method). The fence is used to make sure
  /// the application does not modify the image while there are still
  /// previously submitted operations happening on the image. The semaphore is
  /// used to tell the driver to not start processing new commands with the
  /// given image. \param logical_device **[in]** logical device handle \param
  /// swapchain **[in]** swapchain handle \param semaphore **[in]** semaphore
  /// handle \param fence **[in]** fence handle \param image_index **[out]**
  /// acquired image index \return bool true if success
  static bool acquireSwapchainImage(VkDevice logical_device,
                                    VkSwapchainKHR swapchain,
                                    VkSemaphore semaphore, VkFence fence,
                                    uint32_t &image_index);
  /// \brief Sends to the hardware the images to be presented. Semaphores are
  /// used to assure the correct display time for each image. Multiple images
  /// can be presented at the same time, but only one per swapchain.
  /// \param queue **[in]** the handle of a queue that supports presentation
  /// \param rendering_semaphores **[in]** associated semaphore for each image
  /// list
  /// \param images_to_present **[in]** list of images
  static bool presentImage(VkQueue queue,
                           std::vector<VkSemaphore> rendering_semaphores,
                           std::vector<PresentInfo> images_to_present);

  // SYNCHRONIZATION
  // ---------------
  // A very important task in vulkan applications is the submission of
  // operations to the hardware. The operations are submitted in form of
  // commands that are stored in buffers and sent to family queues provided
  // by the device. Each of these queues are specialized in certain types of
  // commands and different queues can be processed simultaniously. Depending
  // on the application and the commands being executed and the operations
  // waiting to be executed, some dependencies might appear. One queue might
  // need the operations of another queue to finish first and then complete
  // its work for example. The same may happen on the application side,
  // waiting for the queue to finish its work. For that, Vulkan provides
  // semaphores and fences.
  // - Semaphores allow us to coordinate operations submitted within one queue
  //   and between different queues in one logical device. They are submitted
  //   to command buffer submissions and have their state changed as soon as
  //   all commands are finished. We can also specify that certain commands
  //   should wait until all semaphores from a certain list get activated.
  // - Fences inform the application that a submitted work is finished. A
  // fence
  //   changes its state as soon all work submitted along with it is finished.

  /// \brief Creates a semaphore for a given logical device
  /// \param logical_device **[in]** logical device handle
  /// \param semaphore **[out]** semaphore handle
  /// \return bool true if success
  static bool createSemaphore(VkDevice logical_device, VkSemaphore &semaphore);
  /// \param logical_device **[in]** logical device handle
  /// \param semaphore **[in/out]** semaphore handle
  static void destroySemaphore(VkDevice logical_device, VkSemaphore &semaphore);
  /// \brief Creates a fence for a given logical device.
  /// \param logical_device **[in]** logical device handle
  /// \param signaled **[in]** fence's initial state
  /// \param fence **[out]** fence handle
  /// \return bool true if success
  static bool createFence(VkDevice logical_device, bool signaled,
                          VkFence &fence);
  /// \param logical_device **[in]** logical device handle
  /// \param fence **[in/out]** fence handle
  static void destroyFence(VkDevice logical_device, VkFence &fence);
  /// \brief Blocks application until the fence(s) get changed or time out.
  /// \param logical_device **[in]** logical device handle
  /// \param fences **[in]** list of fences
  /// \param wait_for_all **[in]** **false** if any fence that gets changed is
  /// enough, or **true** if all fences must be changed.
  /// \param timeout **[in]** time out
  /// \return bool true if success
  static bool waitForFences(VkDevice logical_device,
                            std::vector<VkFence> const &fences,
                            VkBool32 wait_for_all, uint64_t timeout);
  /// \brief Deactivate the given fences. Its the application's responsability
  /// to put the fences back to their initial state after they get activated.
  /// \param logical_device **[in]** logical device handle
  /// \param fences **[in/out]** list of fences
  /// \return bool true if success
  static bool resetFences(VkDevice logical_device,
                          std::vector<VkFence> const &fences);

  // VULKAN COMMAND BUFFERS
  // ----------------------
  // Command buffers record operations and are submitted to the hardware. They
  // can be recorded in multiple threads and also can be saved and reused.
  // Synchronization is very important on this part, because the operations
  // submitted need to be processed properly.
  // Before allocating command buffers, we need to allocate command pools,
  // from which the command buffers acquire memory. Command pools are also
  // responsible for informing the driver on how to deal with the command
  // buffers memory allocated from them (for example, whether the command
  // buffer will have a short life or if they need to be reset or freed).
  // Command pools also control the queues that receive the command buffers.
  // Command buffers are separate in two groups:
  // 1. Primary - can be directly submitted to queues and call secondary
  // command
  //    buffers.
  // 2. Secondary - can only be executed from primary command buffers.
  // When recording commands to command buffers, we need to set the state for
  // the operations as well (for example, vertex attributes use other buffers
  // to work). When calling a secondary command buffer, the state of the
  // caller primary command buffer is not preserved (unless it is a render
  // pass).

  /// \brief Create a Command Pool object
  /// Command pools cannot be used concurrently, we must create a separate
  /// command pool for each thread.
  /// \param logical_device **[in]** logical device handle
  /// \param parameters **[in]** flags that define how the command pool will
  /// handle command buffer creation, memory, life span, etc.
  /// \param queue_family **[in]** queue family to which command buffers will
  /// be submitted. \param command_pool **[out]** command pool handle \return
  /// bool
  static bool createCommandPool(VkDevice logical_device,
                                VkCommandPoolCreateFlags parameters,
                                uint32_t queue_family,
                                VkCommandPool &command_pool);
  /// \param logical_device **[in]** logical device handle
  /// \param command_pool **[in/out]** command pool handle
  static void destroyCommandPool(VkDevice logical_device,
                                 VkCommandPool &command_pool);
  /// Creates a list of command buffers from a given command pool.
  /// \param logical_device **[in]** logical device handle
  /// \param command_pool **[in]** command pool handle
  /// \param level **[in]** command buffers level (Primary or Secondary)
  /// \param count **[in]** number of command buffers
  /// \param command_buffers **[out]** list of command buffer handles
  /// \return bool true if success
  static bool
  allocateCommandBuffers(VkDevice logical_device, VkCommandPool command_pool,
                         VkCommandBufferLevel level, uint32_t count,
                         std::vector<VkCommandBuffer> &command_buffers);
  /// \brief Puts the command buffer in record state and allow vkCmd*
  /// functions to be recorderd. \param command_buffer **[in]** command buffer
  /// handle \param usage **[in]** usage description \param
  /// secondary_command_buffer_info **[in]** \return bool true if success
  static bool beginCommandBufferRecordingOperation(
      VkCommandBuffer command_buffer, VkCommandBufferUsageFlags usage,
      VkCommandBufferInheritanceInfo *secondary_command_buffer_info);
  /// \brief Stops recording commands by taking the command buffer out of the
  /// recording state.
  /// \param command_buffer **[in]** command buffer handle
  /// \return bool true if success
  static bool
  endCommandBufferRecordingOperation(VkCommandBuffer command_buffer);
  /// \brief Explicitly reset the command buffer to allow a new recording.
  /// \param command_buffer **[in]** command buffer handle
  /// \param release_resources **[in]** set to give back the memory to the
  /// memory pool
  /// \return bool true if success
  static bool resetCommandBuffer(VkCommandBuffer command_buffer,
                                 bool release_resources);
  /// \brief Resets all command buffers of a given command pool at once.
  /// \param logical_device **[in]** logical device
  /// \param command_pool **[in]** command pool handle
  /// \param release_resources **[in]** give memory from the command buffers
  /// back to the pool
  /// \return bool true if success
  static bool resetCommandPool(VkDevice logical_device,
                               VkCommandPool command_pool,
                               bool release_resources);
  /// \param logical_device **[in]** logical device handle
  /// \param command_pool **[in]** command pool handle
  /// \param command_buffers **[in]** list of command buffer handles
  static void freeCommandBuffers(VkDevice logical_device,
                                 VkCommandPool command_pool,
                                 std::vector<VkCommandBuffer> &command_buffers);

  // COMMAND BUFFER SUBMISSION
  // -------------------------
  // When submitting a command buffer, we can choose semaphores on which the
  // device should wait before processing the command buffer and also in which
  // pipeline stages the wait should occur.

  /// \brief Submittes command buffers to a queue following waiting rules.
  /// \param queue **[in]** queue handle
  /// \param wait_semaphore_infos **[in]** waiting information
  /// \param command_buffers **[in]** list of command buffer handles
  /// \param signal_semaphores **[in]** list of semaphores on which the queue
  /// should wait to execute the command buffers
  /// \param fence **[in]** fance handle to be signaled after all command
  /// buffers get executed
  /// \return bool true if success
  static bool submitCommandBuffersToQueue(
      VkQueue queue, std::vector<WaitSemaphoreInfo> wait_semaphore_infos,
      std::vector<VkCommandBuffer> command_buffers,
      std::vector<VkSemaphore> signal_semaphores, VkFence fence);
  /// \param queue **[in]** queue handle
  /// \return bool true if success
  static bool waitUntilAllCommandsSubmittedToQueueAreFinished(VkQueue queue);
  /// \param logical_device **[in]** logical device handle
  /// \return bool true if success
  static bool waitForAllSubmittedCommandsToBeFinished(VkDevice logical_device);

  // VULKAN PIPELINES
  // ----------------
  //

  // EXAMPLES
  // --------
  // Here follows some auxiliary methods for instance and device creation

  /// Same as **createVulkanInstance**, but automatically appends the
  /// VK_KHR_***_SURFACE_EXTANTION_NAME to the desired extensions list
  /// \param desired_extensions **[in]** list of desired extensions
  /// \param application_name **[in]** application name
  /// \param instance **[out]** instance handle
  /// \return bool true if success
  static bool createVulkanInstanceWithWsiExtensionsEnabled(
      std::vector<char const *> &desired_extensions,
      char const *const application_name, VkInstance &instance);
  /// Same as **createLogicalDevice**, but automatically appends the
  /// VK_KHR_SWAPCHAIN_EXTENTSION_NAME to the desired extensions list
  /// \param physical_device **[in]** physical device handle
  /// \param queue_infos **[in]** queues description
  /// \param desired_extensions **[in]** desired extensions
  /// \param desired_features **[in]** desired features
  /// \param logical_device **[out]** logical device handle
  /// \return bool true if success
  static bool createLogicalDeviceWithWsiExtensionsEnabled(
      VkPhysicalDevice physical_device,
      std::vector<QueueFamilyInfo> queue_infos,
      std::vector<char const *> &desired_extensions,
      VkPhysicalDeviceFeatures *desired_features, VkDevice &logical_device);
  static bool createLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues(
      VkInstance instance, VkDevice &logical_device, VkQueue &graphics_queue,
      VkQueue &compute_queue);
  static bool createSwapchainWithR8G8B8A8FormatAndMailboxPresentMode(
      VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
      VkDevice logical_device, VkImageUsageFlags swapchain_image_usage,
      VkExtent2D &image_size, VkFormat &image_format,
      VkSwapchainKHR &old_swapchain, VkSwapchainKHR &swapchain,
      std::vector<VkImage> &swapchain_images);
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

private:
  // VulkanLibraryType vulkanLibrary_;
};

} // namespace vk

} // namespace circe

#endif // CIRCE_VULKAN_LIBRARY