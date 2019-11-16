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
/// \file vk_graphics_display.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-08-04
///
/// \brief

#ifndef CIRCE_VK_GRAPHICS_DISPLAY_H
#define CIRCE_VK_GRAPHICS_DISPLAY_H

#include "vulkan_instance.h"
#include <functional>
// #define GLFW_INCLUDE_VULKAN
// #include <GLFW/glfw3.h>

namespace circe {

namespace vk {

/// Represents a window used by the application to display graphics and interact
/// with the user.
class GraphicsDisplay {
public:
  /// \brief Construct a new Graphics Display object
  /// \param w **[in]** window width (in pixels)
  /// \param h **[in]** window height (in pixels)
  /// \param title **[in]** window title text
  GraphicsDisplay(
      uint32_t w, uint32_t h,
      const std::string &title = std::string("Vulkan Display Window"));
  /// \brief Destroy the Graphics Display object
  ~GraphicsDisplay();
  uint32_t width() const;
  uint32_t height() const;
  /// Runs window loop
  void open(const std::function<void()> & = []() {});
  /// Stops window loop
  void close();
  /// \return bool true if window is running
  bool isOpen() const;
  /// \return std::vector<const char *> list of vulkan extensions required by
  /// the GraphicsDisplay. The hardware must support these extensions in order
  /// to allow the application to use the GraphicsDisplay.
  std::vector<const char *> requiredVkExtensions() const;
  /// \brief Create a Window Surface object
  /// \param instance **[in]** vulkan instance handle
  /// \param surface **[out]** surface handle
  /// \return bool true if success
  bool createWindowSurface(const Instance &instance,
                           VkSurfaceKHR &surface) const;

private:
  uint32_t width_;
  uint32_t height_;
  GLFWwindow *window_ = nullptr;
};

} // namespace vk

} // namespace circe

#endif
