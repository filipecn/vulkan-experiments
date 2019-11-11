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
/// \file vk_graphics_display.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-08-04
///
/// \brief

#include "vk_graphics_display.h"
#include "logging.h"
#include "vulkan_debug.h"

namespace circe {

namespace vk {

GraphicsDisplay::GraphicsDisplay(uint32_t w, uint32_t h,
                                 const std::string &title) {
  ASSERT(glfwInit());
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  ASSERT(glfwVulkanSupported());
  window_ = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
}

GraphicsDisplay::~GraphicsDisplay() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

uint32_t GraphicsDisplay::width() const { return width_; }

uint32_t GraphicsDisplay::height() const { return height_; }

void GraphicsDisplay::open() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
  }
}

void GraphicsDisplay::close() { glfwSetWindowShouldClose(window_, GL_TRUE); }

bool GraphicsDisplay::isOpen() const { return !glfwWindowShouldClose(window_); }

std::vector<const char *> GraphicsDisplay::requiredVkExtensions() const {
  uint32_t glfw_extension_count = 0;
  const char **glfw_extensions;
  glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
  return std::vector<const char *>(glfw_extensions,
                                   glfw_extensions + glfw_extension_count);
}

bool GraphicsDisplay::createWindowSurface(const Instance &instance,
                                          VkSurfaceKHR &surface) const {
  R_CHECK_VULKAN(
      glfwCreateWindowSurface(instance.handle(), window_, nullptr, &surface));
  return true;
}

} // namespace vk

} // namespace circe
