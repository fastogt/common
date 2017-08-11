/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

    This file is part of SiteOnYourDevice.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <string>  // for string

#include <common/event.h>

namespace common {
namespace application {

/**
 *  Function prototype for the timer callback function.
 *
 *  The callback function is passed the current timer interval and returns
 *  the next timer interval.  If the returned value is the same as the one
 *  passed in, the periodic alarm continues, otherwise a new alarm is
 *  scheduled.  If the callback returns 0, the periodic alarm is cancelled.
 */
typedef uint32_t (*timer_callback_t)(uint32_t interval, void* user_data);
typedef int timer_id_t;

class IApplicationImpl {
 public:
  typedef IEvent event_t;
  typedef IListener listener_t;

  IApplicationImpl(int argc, char** argv);

  virtual timer_id_t AddTimer(uint32_t interval, timer_callback_t cb, void* user_data) = 0;
  virtual bool RemoveTimer(timer_id_t id) = 0;

  virtual int PreExec() = 0;   // EXIT_FAILURE, EXIT_SUCCESS
  virtual int Exec() = 0;      // EXIT_FAILURE, EXIT_SUCCESS
  virtual int PostExec() = 0;  // EXIT_FAILURE, EXIT_SUCCESS

  virtual void PostEvent(event_t* event) = 0;
  virtual void SendEvent(event_t* event) = 0;

  virtual void Subscribe(listener_t* listener, events_size_t id) = 0;
  virtual void UnSubscribe(listener_t* listener, events_size_t id) = 0;
  virtual void UnSubscribe(listener_t* listener) = 0;

  virtual void ShowCursor() = 0;
  virtual void HideCursor() = 0;

  virtual void Exit(int result) = 0;
  virtual ~IApplicationImpl();
};

typedef IApplicationImpl* (*CreateApplicationImpl)(int argc, char** argv);

class Application {
 public:
  typedef IEvent event_t;
  typedef IListener listener_t;

  Application(int argc, char** argv, CreateApplicationImpl ptr);

  ~Application();

  std::string AppPath() const;
  std::string AppDir() const;
  int Argc() const;
  char** Argv() const;

  static Application* Instance();

  void PostEvent(event_t* event);
  void SendEvent(event_t* event);

  void Subscribe(listener_t* listener, events_size_t id);
  void UnSubscribe(listener_t* listener, events_size_t id);
  void UnSubscribe(listener_t* listener);

  /**
   * \brief Add a new timer to the pool of timers already running.
   *
   * \return A timer ID, or 0 when an error occurs.
   */
  timer_id_t AddTimer(uint32_t interval, timer_callback_t cb, void* user_data);
  /**
   * \brief Remove a timer knowing its ID.
   *
   * \return A boolean value indicating success or failure.
   */
  bool RemoveTimer(timer_id_t id);

  void ShowCursor();
  void HideCursor();

  int Exec();
  static void Exit(int result);

 private:
  static Application* self_;

  int argc_;
  char** argv_;

  const std::unique_ptr<IApplicationImpl> impl_;
};

}  // namespace application
}  // namespace common

#define fApp common::application::Application::Instance()
