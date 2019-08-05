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
/// \file vk_app.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-08-04
///
/// \brief

#ifndef CIRCE_VK_APP_H
#define CIRCE_VK_APP_H

#include "vk_graphics_display.h"

namespace circe {

namespace vk {

/// Holds all resources of a Vulkan graphical application: window, device,
/// instance, etc.
class App {
public:
  /// \brief Construct a new App object
  /// \param w **[in]** window width (in pixels)
  /// \param h **[in]** window height (in pixels)
  /// \param title **[in]** window title text
  App(size_t w, size_t h,
      const std::string &title = std::string("Vulkan Application"));
  /// \brief Destroy the App object
  ~App();
  /// Runs application loop
  void run();
  /// Stops application loop
  void exit();
  /// Internally creates the Vulkan Instance from information about the
  /// application and desired extensions.
  /// \param extensions **[in | optional]** list of extension names
  /// \return bool true if success
  bool createInstance(const std::vector<const char *> &extensions =
                          std::vector<const char *>());
  /// Iterates over physical devices. This can be used to check which device
  /// suits the application's needs.
  /// Note: It only iterates over devices that have support for presentation.
  /// There is no need for checking for the queue family with support for
  /// presentation of the application surface.
  /// \param f **[in]** callback for device
  /// \return bool
  void pickPhysicalDevice(
      const std::function<bool(const VulkanLibraryInterface::PhysicalDevice &)>
          &f);
  /// \brief Create a Logical Device object
  ///
  /// \param queue_infos **[in]**
  /// \param desired_extensions **[in]**
  /// \param desired_features **[in]**
  /// \return bool
  bool createLogicalDevice(
      const std::vector<VulkanLibraryInterface::QueueFamilyInfo> &queue_infos,
      const std::vector<const char *> &desired_extensions =
          std::vector<char const *>(),
      VkPhysicalDeviceFeatures *desired_features = {});

private:
  std::string application_name_;
  std::unique_ptr<GraphicsDisplay> graphics_display_;
  VkInstance instance_ = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debug_messenger_ = nullptr;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  VulkanLibraryInterface::QueueFamilyInfo present_family_;
  VkQueue present_queue_;
};

} // namespace vk

} // namespace circe

#endif