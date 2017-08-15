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

#include <common/application/application.h>

#include <stdlib.h>  // for EXIT_FAILURE

#include <common/file_system.h>

namespace common {
namespace application {

IApplicationImpl::IApplicationImpl(int argc, char** argv) {
  UNUSED(argc);
  UNUSED(argv);
}

IApplicationImpl::~IApplicationImpl() {}

Application* Application::self_ = nullptr;

Application::Application(int argc, char** argv, CreateApplicationImpl ptr)
    : argc_(argc), argv_(argv), impl_(ptr(argc, argv)) {
  CHECK(!self_);
  if (!self_) {
    self_ = this;
  }
}

Application::~Application() {
  self_ = nullptr;
}

std::string Application::GetAppPath() const {
  return argv_[0];
}
std::string Application::GetAppDir() const {
#ifdef OS_MACOSX
  std::string appP = file_system::pwd();
#else
  std::string appP = GetAppPath();
#endif
  return file_system::get_dir_path(appP);
}
int Application::GetArgc() const {
  return argc_;
}
char** Application::GetArgv() const {
  return argv_;
}

Application* Application::GetInstance() {
  return self_;
}

void Application::PostEvent(event_t* event) {
  impl_->PostEvent(event);
}

void Application::SendEvent(event_t* event) {
  impl_->SendEvent(event);
}

void Application::Subscribe(listener_t* listener, events_size_t id) {
  impl_->Subscribe(listener, id);
}

void Application::UnSubscribe(listener_t* listener, events_size_t id) {
  impl_->UnSubscribe(listener, id);
}

void Application::UnSubscribe(listener_t* listener) {
  impl_->UnSubscribe(listener);
}

timer_id_t Application::AddTimer(uint32_t interval, timer_callback_t cb, void* user_data) {
  return impl_->AddTimer(interval, cb, user_data);
}

bool Application::RemoveTimer(timer_id_t id) {
  return impl_->RemoveTimer(id);
}

void Application::ShowCursor() {
  impl_->ShowCursor();
}

void Application::HideCursor() {
  impl_->HideCursor();
}

int Application::Exec() {  // EXIT_FAILURE, EXIT_SUCCESS
  if (!impl_) {
    return EXIT_FAILURE;
  }

  int res = impl_->PreExec();
  if (res == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  res = impl_->Exec();
  if (res == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }
  return impl_->PostExec();
}

void Application::Exit(int result) {
  if (!self_) {
    return;
  }

  if (!self_->impl_) {
    return;
  }

  self_->impl_->Exit(result);
}

}  // namespace application
}  // namespace common
