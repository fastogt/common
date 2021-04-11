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

#include <common/error.h>  // for Error

namespace common {

typedef size_t identifier_t;
typedef size_t events_size_t;

class IEvent {
 public:
  typedef events_size_t event_id_t;
  virtual ~IEvent();

  event_id_t GetEventID() const;

 protected:
  explicit IEvent(event_id_t id);

 private:
  const event_id_t event_id_;
};

class IListener {
 public:
  typedef IEvent event_t;

  virtual void HandleEvent(event_t* event) = 0;
  virtual ~IListener();
};

template <typename type>
class IEventEx : public IEvent {
 public:
  typedef type type_t;

  type_t GetEventType() const {
    const event_id_t id = GetEventID();
    return static_cast<type_t>(id);
  }

 protected:
  explicit IEventEx(type_t event_id) : IEvent(event_id) {}
};

template <typename type, type event_t>
class Event : public IEventEx<type> {
 public:
  typedef type type_t;
  typedef void senders_t;
  static const type_t EventType = event_t;

  senders_t* GetSender() const { return sender_; }

 protected:
  explicit Event(senders_t* sender) : IEventEx<type_t>(event_t), sender_(sender) {}

 private:
  senders_t* sender_;
};

template <class type_t>
class IListenerEx : public IListener {
 public:
  typedef IEventEx<type_t> event_t;

  virtual void HandleEvent(event_t* event) = 0;
  virtual void HandleExceptionEvent(event_t* event, Error err) = 0;
  virtual ~IListenerEx() {}

 private:
  virtual void HandleEvent(IEvent* event) final {
    event_t* ev = static_cast<event_t*>(event);
    HandleEvent(ev);
  }
};

template <typename type>
class IExceptionEvent;

template <typename type_t>
struct event_traits {
  typedef IEventEx<type_t> event_t;
  typedef IExceptionEvent<type_t> ex_event_t;
  typedef IListenerEx<type_t> listener_t;

  static const events_size_t max_count;
  static const identifier_t id;
};

template <typename type>
class IExceptionEvent : public IEventEx<type> {
 public:
  typedef type type_t;
  typedef IEventEx<type_t> event_t;

  explicit IExceptionEvent(event_t* event, Error err)  // take ownerships event
      : IEventEx<type>(static_cast<type_t>(event_traits<type_t>::max_count)), event_(event), err_(err) {}

  virtual ~IExceptionEvent() { delete event_; }

  Error GetError() const { return err_; }

  event_t* GetEvent() const { return event_; }

 protected:
  event_t* event_;
  Error err_;
};

template <typename type>
IExceptionEvent<type>* make_exception_event(IEventEx<type>* event, Error err) {
  return new IExceptionEvent<type>(event, err);
}

}  // namespace common
