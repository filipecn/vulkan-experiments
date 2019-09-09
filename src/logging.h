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
/// \file logging.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date 2019-09-08
///
/// \brief

///
#define INFO(M)                                                                \
  std::cerr << "[INFO] in [" << __FILE__ << "][" << __LINE__ << "]: " << (M)   \
            << std::endl;
///
#define CHECK_INFO(A, M)                                                       \
  if (!(A)) {                                                                  \
    std::cerr << "[INFO] in [" << __FILE__ << "][" << __LINE__ << "]: " << (M) \
              << std::endl;                                                    \
    return false;                                                              \
  }
///
#define ASSERT(A)                                                              \
  if (!(A)) {                                                                  \
    std::cerr << "[ASSERTION_ERROR] in [" << __FILE__ << "][" << __LINE__      \
              << "]: " << #A << std::endl;                                     \
    exit(-1);                                                                  \
  }
///
#define D_RETURN_FALSE_IF_NOT(A, M)                                            \
  if (!(A)) {                                                                  \
    std::cerr << "[CHECK_ERROR] in [" << __FILE__ << "][" << __LINE__          \
              << "]: " << (M) << std::endl;                                    \
    return false;                                                              \
  }
#define RETURN_FALSE_IF_NOT(A)                                                 \
  if (!(A)) {                                                                  \
    std::cerr << "[CHECK_ERROR] in [" << __FILE__ << "][" << __LINE__          \
              << "]: " << #A << std::endl;                                     \
    return false;                                                              \
  }
