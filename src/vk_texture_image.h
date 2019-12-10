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
/// \file vk_texture_image.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-12-09
///
/// \brief

#ifndef CIRCE_VK_TEXTURE_IMAGE_H
#define CIRCE_VK_TEXTURE_IMAGE_H

#include <string>
#include "vk_image.h"
#include "vk_device_memory.h"

namespace circe::vk {

class Texture {
public:
  explicit Texture(const LogicalDevice *logical_device,
                   const std::string& filename);
private:
  const LogicalDevice* logical_device_ = nullptr;
  std::unique_ptr<Image> image_;
  std::unique_ptr<DeviceMemory> image_memory_;
};

} // circe::vk namespace

#endif
