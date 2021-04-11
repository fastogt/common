/*  Copyright (C) 2014-2021 FastoGT. All right reserved.

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

#include <stdint.h>
#include <string>

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

class IApplication {
 public:
  typedef IEvent event_t;
  typedef IListener listener_t;

  IApplication(int argc, char** argv);

  std::string GetAppPath() const;
  std::string GetAppDir() const;
  int GetArgc() const;
  char** GetArgv() const;

  virtual timer_id_t AddTimer(uint32_t interval, timer_callback_t cb, void* user_data) = 0;
  virtual bool RemoveTimer(timer_id_t id) = 0;

  virtual void PostEvent(event_t* event) = 0;
  virtual void SendEvent(event_t* event) = 0;

  virtual void Subscribe(listener_t* listener, events_size_t id) = 0;
  virtual void UnSubscribe(listener_t* listener, events_size_t id) = 0;
  virtual void UnSubscribe(listener_t* listener) = 0;

  virtual void ShowCursor() = 0;
  virtual void HideCursor() = 0;
  virtual bool IsCursorVisible() const = 0;

  int Exec();

  static IApplication* GetInstance();
  static void Exit(int result);

  virtual ~IApplication();

 private:
  virtual void ExitImpl(int result) = 0;

  virtual int PreExecImpl() = 0;   // EXIT_FAILURE, EXIT_SUCCESS
  virtual int ExecImpl() = 0;      // EXIT_FAILURE, EXIT_SUCCESS
  virtual int PostExecImpl() = 0;  // EXIT_FAILURE, EXIT_SUCCESS

  static IApplication* self_;

  int argc_;
  char** argv_;
};

}  // namespace application
}  // namespace common

#define fApp common::application::IApplication::GetInstance()
