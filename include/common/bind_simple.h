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

#include <functional>
#include <tuple>

namespace common {
namespace utils {

template <std::size_t...>
struct _Index_tuple;

template <std::size_t... _Indexes>
struct _Index_tuple {
  typedef _Index_tuple<_Indexes..., sizeof...(_Indexes)> __next;
};

template <>
struct _Index_tuple<0> {
  typedef void* __next;
};

template <typename _Tp>
struct _Maybe_wrap_member_pointer {
  typedef _Tp type;

  static const _Tp& __do_wrap(const _Tp& __x) { return __x; }
};

/// Builds an _Index_tuple<0, 1, 2, ..., _Num-1>.
template <std::size_t _Num>
struct _Build_index_tuple {
  typedef typename _Build_index_tuple<_Num - 1>::__type::__next __type;
};

template <>
struct _Build_index_tuple<0> {
  typedef _Index_tuple<> __type;
};

template <typename _Signature>
struct _Bind_simple;

template <typename _Callable, typename... _Args>
struct _Bind_simple<_Callable(_Args...)> {
  typedef typename std::result_of<_Callable(_Args...)>::type result_type;

  template <typename... _Args2, typename = typename std::enable_if<sizeof...(_Args) == sizeof...(_Args2)>::type>
  explicit _Bind_simple(const _Callable& __callable, _Args2&&... __args)
      : _M_bound(__callable, std::forward<_Args2>(__args)...) {}

  template <typename... _Args2, typename = typename std::enable_if<sizeof...(_Args) == sizeof...(_Args2)>::type>
  explicit _Bind_simple(_Callable&& __callable, _Args2&&... __args)
      : _M_bound(std::move(__callable), std::forward<_Args2>(__args)...) {}

  // _Bind_simple(const _Bind_simple&) = default;
  // _Bind_simple(_Bind_simple&&) = default;

  result_type operator()() {
    typedef typename _Build_index_tuple<sizeof...(_Args)>::__type _Indices;
    return _M_invoke(_Indices());
  }

 private:
  template <std::size_t... _Indices>
  typename std::result_of<_Callable(_Args...)>::type _M_invoke(_Index_tuple<_Indices...>) {
    // std::bind always forwards bound arguments as lvalues,
    // but this type can call functions which only accept rvalues.
    return std::forward<_Callable>(std::get<0>(_M_bound))(std::forward<_Args>(std::get<_Indices + 1>(_M_bound))...);
  }

  std::tuple<_Callable, _Args...> _M_bound;
};

template <typename _Func, typename... _BoundArgs>
struct _Bind_simple_helper {
  typedef _Maybe_wrap_member_pointer<typename std::decay<_Func>::type> __maybe_type;
  typedef typename __maybe_type::type __func_type;
  typedef _Bind_simple<__func_type(typename std::decay<_BoundArgs>::type...)> __type;
};

// Simplified version of std::bind for internal use, without support for
// unbound arguments, placeholders or nested bind expressions.
template <typename _Callable, typename... _Args>
typename _Bind_simple_helper<_Callable, _Args...>::__type bind_simple(_Callable&& __callable, _Args&&... __args) {
  typedef _Bind_simple_helper<_Callable, _Args...> __helper_type;
  typedef typename __helper_type::__maybe_type __maybe_type;
  typedef typename __helper_type::__type __result_type;
  return __result_type(__maybe_type::__do_wrap(std::forward<_Callable>(__callable)), std::forward<_Args>(__args)...);
}

}  // namespace utils
}  // namespace common
