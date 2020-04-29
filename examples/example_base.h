/// Copyright (c) 2020, FilipeCN.
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
///\file example_base.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2020-04-28
///
///\brief

#ifndef EXAMPLE_BASE_H
#define EXAMPLE_BASE_H

#include <core/vk_app.h>
#include <ponos/common/defs.h>

class ExampleBase {
public:
  template<typename... Args> explicit ExampleBase(Args &&... args) {
    // setup app
    app_ = std::make_unique<circe::vk::App>(std::forward<Args>(args)...);
    app_->setValidationLayers({"VK_LAYER_KHRONOS_validation"});
    // retrieve queue for buffer upload operations
    graphics_queue_family_index_ =
        app_->queueFamilies().family("graphics").family_index.value();
    queue_ = app_->queueFamilies().family("graphics").vk_queues[0];
  }
  virtual ~ExampleBase() = default;

protected:
  virtual void prepareRenderpass();
  virtual void preparePipeline();

  std::unique_ptr<circe::vk::App> app_;
  VkQueue graphics_queue_{nullptr};
  u32 graphics_queue_family_index_{0};
private:
};

#endif //EXAMPLE_BASE_H