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

#include <atomic>

#include <common/macros.h>

namespace common {

template <typename T>
class intrusive_ptr {
 private:
  typedef intrusive_ptr this_type;

 public:
  typedef T element_type;

  intrusive_ptr() : px(0) {}

  intrusive_ptr(T* p, bool add_ref = true) : px(p) {
    if (px != 0 && add_ref) {
      intrusive_ptr_add_ref(px);
    }
  }

  template <typename U>
  intrusive_ptr(const intrusive_ptr<U>& rhs) : px(rhs.get()) {
    if (px != 0) {
      intrusive_ptr_add_ref(px);
    }
  }

  intrusive_ptr(const intrusive_ptr& rhs) : px(rhs.px) {
    if (px != 0) {
      intrusive_ptr_add_ref(px);
    }
  }

  ~intrusive_ptr() {
    if (px != 0) {
      intrusive_ptr_release(px);
    }
  }

  template <typename U>
  intrusive_ptr& operator=(const intrusive_ptr<U>& rhs) {
    this_type(rhs).swap(*this);
    return *this;
  }

  // Move support

  intrusive_ptr(intrusive_ptr&& rhs) : px(rhs.px) { rhs.px = 0; }

  intrusive_ptr& operator=(intrusive_ptr&& rhs) {
    this_type(static_cast<intrusive_ptr&&>(rhs)).swap(*this);
    return *this;
  }

  intrusive_ptr& operator=(const intrusive_ptr& rhs) {
    this_type(rhs).swap(*this);
    return *this;
  }

  intrusive_ptr& operator=(T* rhs) {
    this_type(rhs).swap(*this);
    return *this;
  }

  explicit operator bool() const { return get() != 0; }

  void reset() { this_type().swap(*this); }

  void reset(T* rhs) { this_type(rhs).swap(*this); }

  void reset(T* rhs, bool add_ref) { this_type(rhs, add_ref).swap(*this); }

  T* get() const { return px; }

  T* detach() {
    T* ret = px;
    px = 0;
    return ret;
  }

  T& operator*() const { return *px; }

  T* operator->() const { return px; }

  // implicit conversion to "bool"

  void swap(intrusive_ptr& rhs) {
    T* tmp = px;
    px = rhs.px;
    rhs.px = tmp;
  }

 private:
  T* px;
};

template <typename T, typename U>
inline bool operator==(const intrusive_ptr<T>& a, const intrusive_ptr<U>& b) {
  return a.get() == b.get();
}

template <typename T, typename U>
inline bool operator!=(const intrusive_ptr<T>& a, const intrusive_ptr<U>& b) {
  return a.get() != b.get();
}

template <typename T, typename U>
inline bool operator==(const intrusive_ptr<T>& a, U* b) {
  return a.get() == b;
}

template <typename T, typename U>
inline bool operator!=(const intrusive_ptr<T>& a, U* b) {
  return a.get() != b;
}

template <typename T, typename U>
inline bool operator==(T* a, const intrusive_ptr<U>& b) {
  return a == b.get();
}

template <typename T, typename U>
inline bool operator!=(T* a, const intrusive_ptr<U>& b) {
  return a != b.get();
}

template <typename T>
inline bool operator==(const intrusive_ptr<T>& a, std::nullptr_t) {
  return !a;
}

template <typename T>
inline bool operator==(std::nullptr_t, const intrusive_ptr<T>& a) {
  return !a;
}

template <typename T>
inline bool operator!=(const intrusive_ptr<T>& a, std::nullptr_t) {
  return static_cast<bool>(a);
}

template <typename T>
inline bool operator!=(std::nullptr_t, const intrusive_ptr<T>& a) noexcept {
  return static_cast<bool>(a);
}

template <typename T>
inline bool operator<(const intrusive_ptr<T>& a, const intrusive_ptr<T>& b) {
  return std::less<T*>()(a.get(), b.get());
}

template <typename T>
void swap(intrusive_ptr<T>& lhs, intrusive_ptr<T>& rhs) {
  lhs.swap(rhs);
}

// mem_fn support

template <typename T>
T* get_pointer(const intrusive_ptr<T>& p) {
  return p.get();
}

template <typename T, typename U>
intrusive_ptr<T> static_pointer_cast(const intrusive_ptr<U>& p) {
  return static_cast<T*>(p.get());
}

template <typename T, typename U>
intrusive_ptr<T> const_pointer_cast(const intrusive_ptr<U>& p) {
  return const_cast<T*>(p.get());
}

template <typename T, typename U>
intrusive_ptr<T> dynamic_pointer_cast(const intrusive_ptr<U>& p) {
  return dynamic_cast<T*>(p.get());  // +
}

// operator<<

template <typename Y>
std::ostream& operator<<(std::ostream& os, const intrusive_ptr<Y>& p) {
  os << p.get();
  return os;
}

template <typename T, typename refcount_t = std::atomic_size_t>
struct intrusive_ptr_base {
  typedef refcount_t refcount_type;
  typedef intrusive_ptr_base<T, refcount_t> class_type;

 protected:
  intrusive_ptr_base() : ref_count_(0) {}

  ~intrusive_ptr_base() {}

  friend void intrusive_ptr_add_ref(class_type const* s) { ++s->ref_count_; }

  friend void intrusive_ptr_release(class_type const* s) {
    if (--s->ref_count_ == 0) {
      T const* t = static_cast<T const*>(s);
      destroy(&t);
    }
  }

  refcount_type ref_count() const { return ref_count_; }

 private:
  mutable refcount_type ref_count_;
  DISALLOW_COPY_AND_ASSIGN(intrusive_ptr_base);
};

template <typename T, typename... Args>
inline intrusive_ptr<T> make_intrusive(Args&&... args) {
  return intrusive_ptr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace common
