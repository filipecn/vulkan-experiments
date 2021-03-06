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

static void framebufferResizeCallback(GLFWwindow *window, int width,
                                      int height) {
  auto gd =
      reinterpret_cast<GraphicsDisplay *>(glfwGetWindowUserPointer(window));
  if (gd->resize_callback)
    gd->resize_callback(width, height);
}

GraphicsDisplay::GraphicsDisplay(uint32_t w, uint32_t h,
                                 const std::string &title)
    : width_(w), height_(h) {
  ASSERT(glfwInit());
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  ASSERT(glfwVulkanSupported());
  window_ = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window_, this);
  glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
}

GraphicsDisplay::~GraphicsDisplay() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

VkExtent2D GraphicsDisplay::framebufferSize() const {
  int w, h;
  glfwGetFramebufferSize(window_, &w, &h);
  VkExtent2D extent = {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
  return extent;
}

uint32_t GraphicsDisplay::width() const { return width_; }

uint32_t GraphicsDisplay::height() const { return height_; }

void GraphicsDisplay::open(const std::function<void()> &f) {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    f();
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

bool GraphicsDisplay::createWindowSurface(const Instance *instance,
                                          VkSurfaceKHR &surface) {
  R_CHECK_VULKAN(
      glfwCreateWindowSurface(instance->handle(), window_, nullptr, &surface));
  return true;
}

void GraphicsDisplay::waitForValidWindowSize() {
  int w = 0, h = 0;
  while (w == 0 || h == 0) {
    glfwGetFramebufferSize(window_, &w, &h);
    glfwWaitEvents();
  }
}

GLFWwindow *GraphicsDisplay::handle() { return window_; }

} // namespace vk

} // namespace circe
