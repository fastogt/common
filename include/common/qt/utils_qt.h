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

#include <QEvent>
#include <QFileDevice>
#include <QModelIndex>

#include <common/error.h>

namespace common {
namespace qt {

template <typename Base, typename Derived>
inline Derived item(const QModelIndex& index) {
  COMPILE_ASSERT(std::is_polymorphic<typename std::remove_pointer<Derived>::type>::value, "Must be polymorphic type");
  COMPILE_ASSERT(
      (std::is_base_of<typename std::remove_pointer<Base>::type, typename std::remove_pointer<Derived>::type>::value),
      "Derived class must be derived from Base");
  void* raw = index.internalPointer();
  Base stype = static_cast<Base>(raw);
  if (!stype) {
    return nullptr;
  }

  Derived dtype = dynamic_cast<Derived>(stype);  // +
  return dtype;
}

template <typename value_t, unsigned event_t>
class Event : public QEvent {
 public:
  typedef value_t value_type;
  typedef QObject* senders_type;
  enum { EventType = event_t };

  Event(senders_type sender, const value_t& init_value)
      : QEvent(static_cast<QEvent::Type>(EventType)), value_(init_value), sender_(sender) {}

  const value_t& value() const { return value_; }

  senders_type sender() const { return sender_; }

 private:
  const value_t value_;
  senders_type sender_;
};

template <typename error_t>
class EventInfo {
 public:
  typedef error_t error_type;
  typedef void* initiator_type;

  EventInfo(initiator_type initiator, error_type err) : error_info_(err), initiator_(initiator) {}

  error_type errorInfo() const { return error_info_; }

  void setErrorInfo(error_type err) { error_info_ = err; }

  initiator_type initiator() const { return initiator_; }

 private:
  error_type error_info_;
  initiator_type initiator_;
};

QString ApplicationDirPath();

// qt file error
struct QtFileErrorTraits {
  static std::string GetTextFromErrorCode(QFileDevice::FileError error);
};
typedef ErrorBase<QFileDevice::FileError, QtFileErrorTraits> QtFileErrorValue;
typedef Optional<QtFileErrorValue> QtFileError;

QtFileError LoadFromFileText(const QString& filePath, QString* outText);
QtFileError SaveToFileText(QString filePath, const QString& text);

struct WaitCursorHolder {
  WaitCursorHolder();
  ~WaitCursorHolder();
};

}  // namespace qt
}  // namespace common
