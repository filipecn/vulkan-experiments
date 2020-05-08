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
///\file vk_shader_module.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2019-11-01
///
///\brief

#include "vk_shader_module.h"
#include "vulkan_debug.h"
#include <fstream>

namespace circe {

namespace vk {

bool readFile(const std::string &filename, std::vector<char> &buffer) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
    return false;

  size_t fileSize = (size_t) file.tellg();
  buffer.resize(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();
  return true;
}

ShaderModule::ShaderModule() = default;

ShaderModule::ShaderModule(const LogicalDevice *logical_device,
                           const std::string &filename)
    : logical_device_(logical_device) {
  std::vector<char> source_code;
  if (!readFile(filename, source_code)) {
    std::cerr << "Could not read shader file:" << filename << std::endl;
    return;
  }
  VkShaderModuleCreateInfo shader_module_create_info = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // VkStructureType sType
      nullptr,            // const void                 * pNext
      0,                  // VkShaderModuleCreateFlags    flags
      source_code.size(), // size_t                       codeSize
      reinterpret_cast<uint32_t const *>(
          source_code.data()) // const uint32_t             * pCode
  };
  CHECK_VULKAN(vkCreateShaderModule(logical_device->handle(),
                                    &shader_module_create_info, nullptr,
                                    &vk_shader_module_));
}

ShaderModule::ShaderModule(const LogicalDevice *logical_device,
                           std::vector<char> const &source_code)
    : logical_device_(logical_device) {
  VkShaderModuleCreateInfo shader_module_create_info = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // VkStructureType sType
      nullptr,            // const void                 * pNext
      0,                  // VkShaderModuleCreateFlags    flags
      source_code.size(), // size_t                       codeSize
      reinterpret_cast<uint32_t const *>(
          source_code.data()) // const uint32_t             * pCode
  };
  CHECK_VULKAN(vkCreateShaderModule(logical_device->handle(),
                                    &shader_module_create_info, nullptr,
                                    &vk_shader_module_));
}

ShaderModule::~ShaderModule() {
  if (VK_NULL_HANDLE != vk_shader_module_)
    vkDestroyShaderModule(logical_device_->handle(), vk_shader_module_,
                          nullptr);
}

void ShaderModule::setDevice(const LogicalDevice *logical_device) {
  logical_device_ = logical_device;
}

bool ShaderModule::load(const std::string &filename) {
  if (!logical_device_)
    return false;
  std::vector<char> source_code;
  if (!readFile(filename, source_code)) {
    std::cerr << "Could not read shader file:" << filename << std::endl;
    return false;
  }
  VkShaderModuleCreateInfo shader_module_create_info = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // VkStructureType sType
      nullptr,            // const void                 * pNext
      0,                  // VkShaderModuleCreateFlags    flags
      source_code.size(), // size_t                       codeSize
      reinterpret_cast<uint32_t const *>(
          source_code.data()) // const uint32_t             * pCode
  };
  CHECK_VULKAN(vkCreateShaderModule(logical_device_->handle(),
                                    &shader_module_create_info, nullptr,
                                    &vk_shader_module_));
}

VkShaderModule ShaderModule::handle() const { return vk_shader_module_; }

} // namespace vk

} // namespace circe
