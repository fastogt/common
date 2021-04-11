/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <common/qt/convert2string.h>

#include <QByteArray>  // for QByteArray
#include <QChar>       // for QChar, operator!=

namespace common {

string16 ConvertToString16(const QString& value) {
  return string16(reinterpret_cast<const char16*>(value.constData()), sizeof(QChar) * value.size());
}

bool ConvertFromString16(const string16& value, QString* out) {
  if (!out) {
    return false;
  }

#if defined(WCHAR_T_IS_UTF16)
  const QChar* unicode = reinterpret_cast<const QChar*>(value.c_str());
  *out = QString(unicode, value.size());
#elif defined(WCHAR_T_IS_UTF32)
  *out = QString::fromUtf16(reinterpret_cast<const char16*>(value.c_str()), value.size());
#endif
  return true;
}

std::string ConvertToString(const QString& from) {
  QByteArray utf8 = from.toUtf8();
  return std::string(utf8.constData(), utf8.size());
}

bool ConvertFromString(const std::string& value, QString* out) {
  if (!out) {
    return false;
  }

  *out = QString::fromUtf8(value.c_str(), value.size());
  return true;
}

buffer_t ConvertToBytes(const QString& from) {
  QByteArray utf8 = from.toUtf8();
  return MAKE_BUFFER_SIZE(utf8.constData(), utf8.size());
}

char_buffer_t ConvertToCharBytes(const QString& from) {
  QByteArray utf8 = from.toUtf8();
  return MAKE_CHAR_BUFFER_SIZE(utf8.constData(), utf8.size());
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& value, QString* out) {
  if (!out) {
    return false;
  }

  *out = QString::fromUtf8(reinterpret_cast<const char*>(value.data()), value.size());
  return true;
}

template bool ConvertFromBytes(const ByteArray<char>& value, QString* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& value, QString* out);

}  // namespace common
