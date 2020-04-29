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

#include "vk_command_buffer.h"
#include "vk_device_memory.h"
#include "vk_graphics_display.h"
#include "vk_image.h"
#include "vk_pipeline.h"
#include "vk_render_engine.h"
#include "vk_renderpass.h"
#include "vk_swap_chain.h"
#include "vk_sync.h"
#include "vulkan_logical_device.h"
#include <functional>
#include <memory>

namespace circe::vk {

/// Holds all resources of a Vulkan graphical application: window, device,
/// instance, etc.
class App {
public:
  /// \brief Construct a new App object
  /// \param w **[in]** window width (in pixels)
  /// \param h **[in]** window height (in pixels)
  /// \param title **[in | optional]** window title text
  App(uint32_t w, uint32_t h,
      const std::string &title = std::string("Vulkan Application"));
  /// \brief Destroy the App object
  ~App();
  /// Runs application loop
  void run(const std::function<void()> &render_callback = []() {});
  /// Stops application loop
  void exit();
  void
  setValidationLayers(const std::vector<const char *> &validation_layer_names,
                      bool instance_level = true, bool device_level = true);
  /// Internally creates the Vulkan Instance from information about the
  /// application and desired extensions.
  /// \param extensions **[in | optional]** list of instance extension names
  /// \return true on success
  bool setInstance(const std::vector<const char *> &extensions =
                       std::vector<const char *>());
  /// Iterates over physical devices. This can be used to check which device
  /// suits the application's needs.
  /// Note: It only iterates over devices that have support for presentation.
  /// There is no need for checking for the queue family with support for
  /// presentation of the application surface.
  /// \param f **[in]** callback for device. Return the score of the device, the
  /// device with highest score will be picked. Devices with score 0 are
  /// discarted.
  /// \return bool
  bool pickPhysicalDevice(const std::function<uint32_t(const PhysicalDevice &,
                                                       QueueFamilies &)> &f);
  /// \brief Create a Logical Device object
  /// There is no need to append the swapchain extension, this method already
  /// does it.
  /// \param queue_infos **[in]**
  /// \param desired_extensions **[in]** desired device extensions list
  /// \param desired_features **[in]** desired features list
  /// \return bool true if success
  bool createLogicalDevice(const std::vector<const char *> &desired_extensions =
                               std::vector<char const *>(),
                           VkPhysicalDeviceFeatures *desired_features = {});
  const Instance *instance();
  const LogicalDevice *logicalDevice();
  QueueFamilies &queueFamilies();
  GraphicsDisplay *graphicsDisplay();

  RenderEngine render_engine;

private:
  std::unique_ptr<GraphicsDisplay> graphics_display_;
  std::unique_ptr<Instance> instance_;
  std::unique_ptr<PhysicalDevice> physical_device_;
  std::unique_ptr<LogicalDevice> logical_device_;

  QueueFamilies queue_families_;

  std::vector<const char *> validation_layer_names_;
  std::string application_name_;
  VkSurfaceKHR vk_surface_ = VK_NULL_HANDLE;
};

} // namespace circe::vk

#endif