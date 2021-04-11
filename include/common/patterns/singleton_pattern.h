/*  Copyright (C) 2014-2021 FastoGT. All right reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.
        * Neither the name of FastoGT. nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <mutex>

#include <common/macros.h>

namespace common {
namespace patterns {

template <class T>
class Singleton {
  static T* _self;

 public:
  void FreeInstance();
  static T* GetInstance();

 protected:
  ~Singleton() {}
  Singleton() {}
};

template <class T>
T* Singleton<T>::_self = nullptr;

template <class T>
T* Singleton<T>::GetInstance() {
  if (!_self) {
    _self = new T;
  }

  return _self;
}

template <class T>
void Singleton<T>::FreeInstance() {
  delete _self;
}

template <class T>
class LazySingleton {
 public:
  typedef LazySingleton<T> class_type;
  static T& GetInstance();

 protected:
  LazySingleton() {}
  ~LazySingleton() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(LazySingleton);
};

template <class T>
T& LazySingleton<T>::GetInstance() {
  static T _self;
  return _self;
}

template <class T>
class TSSingleton {
 public:
  static T* GetInstance() {
    if (!self_) {
      std::lock_guard<std::mutex> lock(mutex_);
      if (!self_) {
        self_ = new T;
      }
    }
    return self_;
  }

  void FreeInstance() {
    std::lock_guard<std::mutex> lock(mutex_);
    delete self_;
    self_ = nullptr;
  }

 protected:
  ~TSSingleton() {}
  TSSingleton() {}

 private:
  static T* self_;
  static std::mutex mutex_;
};

template <class T>
T* TSSingleton<T>::self_ = nullptr;

template <class T>
std::mutex TSSingleton<T>::mutex_;

}  // namespace patterns
}  // namespace common
