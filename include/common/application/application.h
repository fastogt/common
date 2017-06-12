/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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
#include <common/smart_ptr.h>

namespace common {
namespace application {

class IApplicationImpl {
 public:
  typedef IEvent event_t;
  typedef IListener listener_t;

  IApplicationImpl(int argc, char** argv);

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

  void ShowCursor();
  void HideCursor();

  int Exec();
  static void Exit(int result);

 private:
  static Application* self_;

  int argc_;
  char** argv_;

  const common::scoped_ptr<IApplicationImpl> impl_;
};

}  // namespace application
}  // namespace common

#define fApp common::application::Application::Instance()
