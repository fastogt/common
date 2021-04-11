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

#include <common/application/application.h>

#include <stdlib.h>  // for EXIT_FAILURE

#include <common/file_system/string_path_utils.h>

namespace common {
namespace application {

IApplication* IApplication::self_ = nullptr;

IApplication::IApplication(int argc, char** argv) : argc_(argc), argv_(argv) {
  CHECK(!self_);
  self_ = this;
}

IApplication::~IApplication() {
  self_ = nullptr;
}

std::string IApplication::GetAppPath() const {
  return argv_[0];
}

std::string IApplication::GetAppDir() const {
#if defined(OS_MACOSX)
  const std::string app_path = file_system::pwd();
#else
  const std::string app_path = GetAppPath();
#endif
  return file_system::get_dir_path(app_path);
}

int IApplication::GetArgc() const {
  return argc_;
}

char** IApplication::GetArgv() const {
  return argv_;
}

IApplication* IApplication::GetInstance() {
  return self_;
}

int IApplication::Exec() {  // EXIT_FAILURE, EXIT_SUCCESS
  int res = PreExecImpl();
  if (res == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  res = ExecImpl();
  if (res == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }
  return PostExecImpl();
}

void IApplication::Exit(int result) {
  if (!self_) {
    return;
  }

  self_->ExitImpl(result);
}

}  // namespace application
}  // namespace common
