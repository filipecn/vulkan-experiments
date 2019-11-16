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
///\file vk_shader_module.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-11-01
///
///\brief

#ifndef CIRCE_VULKAN_SHADER_MODULE_H
#define CIRCE_VULKAN_SHADER_MODULE_H

#include "vulkan_logical_device.h"

namespace circe {

namespace vk {

/// This class contains the information required to specialize a shader, which
/// is the process of building a shader with some of its constants compiled in.
class ShaderSpecializationInfo {
public:
  ShaderSpecializationInfo();
  ~ShaderSpecializationInfo();

private:
  std::vector<VkSpecializationMapEntry> entries_;
};

// The computations executed inside the pipeline are performed by shaders.
// Shaders are represented by Shader Modules and must be provided to Vulkan as
// SPIR-V assembly code. A single module may contain code for multiple shader
// stages.
class ShaderModule {
public:
  ShaderModule(const LogicalDevice *logical_device,
               const std::string &filename);
  ShaderModule(const LogicalDevice *logical_device,
               std::vector<char> const &source_code);
  ~ShaderModule();
  VkShaderModule handle() const;

private:
  const LogicalDevice *logical_device_ = nullptr;
  VkShaderModule vk_shader_module_ = VK_NULL_HANDLE;
};

} // namespace vk

} // namespace circe

#endif // CIRCE_VULKAN_SHADER_MODULE_H