/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include <gtest/gtest.h>

#include <common/uri/gurl.h>

#define HTTP_PATH "/home/index.html"
#define FILE_PATH "/home/sasha/1.mp4"
#define DEV_VIDEO_PATH "/dev/video3"

TEST(Url, IsValid) {
  common::uri::GURL path;
  ASSERT_FALSE(path.is_valid());

  common::uri::GURL path2("http://www.permadi.com/index.html");
  ASSERT_TRUE(path2.is_valid());

  common::uri::GURL path3("http://www.permadi.com/tutorial/urlEncoding/index.html");
  ASSERT_TRUE(path3.is_valid());

  common::uri::GURL path4(
      "http://www.permadi.com/tutorial/urlEncoding/example.html?var=This+is+a+simple+%26+short+test");
  ASSERT_TRUE(path4.is_valid());
  ASSERT_TRUE(path4.SchemeIsHTTPOrHTTPS());

  const std::string originFile = "file://" + std::string(FILE_PATH);
  common::uri::GURL path5(originFile);
  ASSERT_TRUE(path5.is_valid());
  ASSERT_TRUE(path5.SchemeIsFile());
  ASSERT_EQ(FILE_PATH, path5.path());
  ASSERT_EQ(originFile, path5.spec());

  const std::string originDev = "dev://" + std::string(DEV_VIDEO_PATH);
  common::uri::GURL path6(originDev);
  ASSERT_TRUE(path6.is_valid());
  ASSERT_TRUE(path6.SchemeIsDev());
  ASSERT_EQ(DEV_VIDEO_PATH, path6.path());
  ASSERT_EQ(originDev, path6.spec());

  common::uri::GURL http_path(HTTP_PATH);
  ASSERT_FALSE(http_path.is_valid());
}
