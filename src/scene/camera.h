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
///\file camera.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2020-05-08
///
///\brief

#ifndef CIRCE_VK_CAMERA_H
#define CIRCE_VK_CAMERA_H

#include <ponos/geometry/transform.h>

namespace circe::vk {

class Camera {
public:
  ///
  /// \param pos
  /// \param target
  /// \param up
  Camera(const ponos::point3 &pos, const ponos::point3 &target,
         const ponos::vec3 &up = ponos::vec3(0, 1, 0));
  void setPos(const ponos::point3 &pos);
  void setTarget(const ponos::point3 &target);
  void setUp(const ponos::vec3 &up);
  ponos::Transform view() const;
  ponos::Transform projection() const;

private:
  void update();

  ponos::vec3 up_;
  ponos::point3 pos_;
  ponos::point3 target_;
  ponos::Transform view_;
  ponos::Transform model_;
  ponos::Transform projection_;
};

} // circe::vk namespace

#endif //CIRCE_VK_CAMERA_H