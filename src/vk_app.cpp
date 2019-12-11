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
/// \file vk_app.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-08-04
///
/// \brief

#include "vk_app.h"
#include "logging.h"
#include "vulkan_library.h"
#include <map>

namespace circe::vk {

App::App(uint32_t w, uint32_t h, const std::string &name)
    : application_name_(name),
      graphics_display_(new GraphicsDisplay(w, h, name)) {
  graphics_display_->resize_callback = [&](int new_w, int new_h) {
    render_engine.resize(new_w, new_h);
  };
}

App::~App() {
  render_engine.destroy();
  if (vk_surface_ != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(instance_->handle(), vk_surface_, nullptr);
}

void App::run(const std::function<void()> &render_callback) {
  render_engine.init();
  auto draw_callback = [&]() {
    render_engine.draw(
        queue_families_.family("graphics").vk_queues[0],
        queue_families_.family("presentation").vk_queues[0]
    );
  };
  // graphics_display_->open(render_callback);
  graphics_display_->open(draw_callback);
}

void App::exit() { graphics_display_->close(); }

void App::setValidationLayers(
    const std::vector<const char *> &validation_layer_names,
    bool instance_level, bool device_level) {
  validation_layer_names_ = validation_layer_names;
}

bool App::setInstance(const std::vector<const char *> &extensions) {
  auto es = extensions;
  auto window_extensions = graphics_display_->requiredVkExtensions();
  for (auto e : window_extensions)
    es.emplace_back(e);
  instance_ = std::make_unique<Instance>(application_name_, es,
                                         validation_layer_names_);
  graphics_display_->createWindowSurface(instance_.get(), vk_surface_);
  render_engine.setSurface(vk_surface_);
  return instance_->good();
}

bool App::pickPhysicalDevice(
    const std::function<uint32_t(const PhysicalDevice &, QueueFamilies &)> &f) {
  if (!instance_)
    RETURN_FALSE_IF_NOT(setInstance());
  std::vector<PhysicalDevice> physical_devices;
  instance_->enumerateAvailablePhysicalDevices(physical_devices);
  // ordered map of <score, device index>
  std::multimap<uint32_t, uint32_t> candidates;
  std::vector<QueueFamilies> queue_families(physical_devices.size());
  for (uint32_t i = 0; i < physical_devices.size(); ++i) {
    uint32_t presentation_family = 0;
    uint32_t graphics_family = 0;
    // find a family that supports presentation and graphics
    if (physical_devices[i].selectIndexOfQueueFamily(vk_surface_,
                                                     presentation_family) &&
        physical_devices[i].selectIndexOfQueueFamily(VK_QUEUE_GRAPHICS_BIT,
                                                     graphics_family)) {
      queue_families[i].add(graphics_family, "graphics");
      queue_families[i].add(presentation_family, "presentation");
      candidates.insert(
          std::make_pair(f(physical_devices[i], queue_families[i]), i));
    }
  }
  CHECK_INFO(!candidates.empty() && candidates.rbegin()->first > 0,
             "failed to find a suitable GPU!");
  uint32_t selected_index = candidates.rbegin()->second;
  physical_device_ =
      std::make_unique<PhysicalDevice>(physical_devices[selected_index]);
  queue_families_ = queue_families[selected_index];
  return true;
}

bool App::createLogicalDevice(
    const std::vector<const char *> &desired_extensions,
    VkPhysicalDeviceFeatures *desired_features) {
  if (!physical_device_)
    pickPhysicalDevice([&](const circe::vk::PhysicalDevice &d,
                           circe::vk::QueueFamilies &q) -> uint32_t {
      if (d.properties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
          && d.features().samplerAnisotropy == VK_TRUE)
        return 1000;
      return 1;
    });
  VkPhysicalDeviceFeatures features = {};
  features.samplerAnisotropy = VK_TRUE;
  auto extensions = desired_extensions;
  extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  logical_device_ = std::make_unique<LogicalDevice>(
      *physical_device_, extensions, &features, queue_families_,
      validation_layer_names_);
  render_engine.setDeviceInfo(physical_device_.get(), logical_device_.get(),
                              queue_families_.family("graphics").family_index.value());
  return logical_device_->good();
}

const LogicalDevice *App::logicalDevice() {
  if (!logical_device_)
    createLogicalDevice();
  return logical_device_.get();
}

QueueFamilies &App::queueFamilies() { return queue_families_; }

} // namespace circe::vk