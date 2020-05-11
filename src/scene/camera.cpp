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
///\file camera.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2020-05-08
///
///\brief

#include "camera.h"

namespace circe::vk {

Camera::Camera(const ponos::point3 &pos, const ponos::point3 &target,
               const ponos::vec3 &up) : up_(up), pos_(pos), target_(target) {
  update();
  projection_ = ponos::Transform::perspectiveRH(45.0f, 1.f, 0.1f, 10.0f);
}

void Camera::setPos(const ponos::point3 &pos) {
  pos_ = pos;
  update();
}

void Camera::setTarget(const ponos::point3 &target) {
  target_ = target;
  update();
}

void Camera::setUp(const ponos::vec3 &up) {
  up_ = up;
  update();
}

ponos::Transform Camera::view() const { return view_; }

ponos::Transform Camera::projection() const { return projection_; }

void Camera::update() {
  view_ = ponos::Transform::lookAtRH(pos_, target_, up_);
}

} // circe::vk namespace