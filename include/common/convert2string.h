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

#include <string>
#include <vector>

#include <common/string_piece.h>
#include <common/types.h>

namespace common {

std::string EscapedText(const std::string& str);  // add \n at the end if needed

string16 ConvertToString16(const char* from);
string16 ConvertToString16(const std::string& from);
string16 ConvertToString16(const StringPiece& from);
template <typename ch>
string16 ConvertToString16(const ByteArray<ch>& from);
string16 ConvertToString16(const StringPiece16& from);

string16 ConvertToString16(bool value);
string16 ConvertToString16(char value);
string16 ConvertToString16(unsigned char value);
string16 ConvertToString16(short value);
string16 ConvertToString16(unsigned short value);
string16 ConvertToString16(int value);
string16 ConvertToString16(unsigned int value);
string16 ConvertToString16(long value);
string16 ConvertToString16(unsigned long value);
string16 ConvertToString16(long long value);
string16 ConvertToString16(unsigned long long value);
string16 ConvertToString16(float value);
string16 ConvertToString16(double value);

// std::string

std::string ConvertToString(const char* from);
template <typename ch>
std::string ConvertToString(const ByteArray<ch>& from);
std::string ConvertToString(const string16& from);
std::string ConvertToString(const StringPiece16& from);
std::string ConvertToString(const StringPiece& from);

std::string ConvertToString(bool value);
std::string ConvertToString(char value);
std::string ConvertToString(unsigned char value);
std::string ConvertToString(short value);
std::string ConvertToString(unsigned short value);
std::string ConvertToString(int value);
std::string ConvertToString(unsigned int value);
std::string ConvertToString(long value);
std::string ConvertToString(unsigned long value);
std::string ConvertToString(long long value);
std::string ConvertToString(unsigned long long value);
std::string ConvertToString(float value, int prec = 2);
std::string ConvertToString(double value, int prec = 2);

// buffer_t, char_buffer_t

buffer_t ConvertToBytes(const char* from);
buffer_t ConvertToBytes(const std::string& from);
buffer_t ConvertToBytes(const string16& from);
buffer_t ConvertToBytes(const char_buffer_t& from);
buffer_t ConvertToBytes(bool value);
buffer_t ConvertToBytes(char value);
buffer_t ConvertToBytes(unsigned char value);
buffer_t ConvertToBytes(short value);
buffer_t ConvertToBytes(unsigned short value);
buffer_t ConvertToBytes(int value);
buffer_t ConvertToBytes(unsigned int value);
buffer_t ConvertToBytes(long value);
buffer_t ConvertToBytes(unsigned long value);
buffer_t ConvertToBytes(long long value);
buffer_t ConvertToBytes(unsigned long long value);
buffer_t ConvertToBytes(float value, int prec = 2);
buffer_t ConvertToBytes(double value, int prec = 2);

char_buffer_t ConvertToCharBytes(const char* from);
char_buffer_t ConvertToCharBytes(const std::string& from);
char_buffer_t ConvertToCharBytes(const string16& from);
char_buffer_t ConvertToCharBytes(const buffer_t& from);
char_buffer_t ConvertToCharBytes(bool value);
char_buffer_t ConvertToCharBytes(char value);
char_buffer_t ConvertToCharBytes(unsigned char value);
char_buffer_t ConvertToCharBytes(short value);
char_buffer_t ConvertToCharBytes(unsigned short value);
char_buffer_t ConvertToCharBytes(int value);
char_buffer_t ConvertToCharBytes(unsigned int value);
char_buffer_t ConvertToCharBytes(long value);
char_buffer_t ConvertToCharBytes(unsigned long value);
char_buffer_t ConvertToCharBytes(long long value);
char_buffer_t ConvertToCharBytes(unsigned long long value);
char_buffer_t ConvertToCharBytes(float value, int prec = 2);
char_buffer_t ConvertToCharBytes(double value, int prec = 2);

bool ConvertFromString16(const string16& from, StringPiece* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, std::string* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromString16(const string16& from, ByteArray<ch>* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, bool* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, char* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, unsigned char* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, short* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, unsigned short* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, int* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, unsigned int* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, long* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, unsigned long* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, long long* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, unsigned long long* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, float* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, double* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, StringPiece16* out) WARN_UNUSED_RESULT;

bool ConvertFromString(const std::string& from, StringPiece16* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, string16* out) WARN_UNUSED_RESULT;
#if defined(WCHAR_T_IS_UTF16)
#else
bool ConvertFromString(const std::string& from, std::wstring* out);
#endif
template <typename ch>
bool ConvertFromString(const std::string& from, ByteArray<ch>* out) WARN_UNUSED_RESULT;

bool ConvertFromString(const std::string& from, bool* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, char* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, unsigned char* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, short* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, unsigned short* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, int* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, unsigned int* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, long* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, unsigned long* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, long long* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, unsigned long long* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, float* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, double* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, StringPiece* out) WARN_UNUSED_RESULT;

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, string16* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, std::string* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, bool* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, char* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, unsigned char* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, short* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, unsigned short* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, int* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, unsigned int* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, long* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, unsigned long* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, long long* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, unsigned long long* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, float* out) WARN_UNUSED_RESULT;
template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, double* out) WARN_UNUSED_RESULT;

namespace utils {
namespace hex {

bool encode(const StringPiece& input, bool is_lower, char_buffer_t* out);
bool decode(const StringPiece& input, char_buffer_t* out);

bool encode(const char_buffer_t& input, bool is_lower, char_buffer_t* out);
bool decode(const char_buffer_t& input, char_buffer_t* out);

//
bool encode(const StringPiece& input, bool is_lower, std::string* out);
bool encode(const char_buffer_t& input, bool is_lower, std::string* out);

}  // namespace hex

namespace xhex {

bool encode(const StringPiece& input, bool is_lower, char_buffer_t* out);
bool decode(const StringPiece& input, char_buffer_t* out);

bool encode(const char_buffer_t& input, bool is_lower, char_buffer_t* out);
bool decode(const char_buffer_t& input, char_buffer_t* out);

//
bool encode(const StringPiece& input, bool is_lower, std::string* out);
bool encode(const char_buffer_t& input, bool is_lower, std::string* out);

}  // namespace xhex

namespace unicode {

bool encode(const StringPiece16& input, bool is_lower, char_buffer_t* out);
bool decode(const StringPiece& input, string16* out);

}  // namespace unicode

namespace uunicode {

bool encode(const StringPiece16& input, bool is_lower, char_buffer_t* out);
bool decode(const StringPiece& input, string16* out);

}  // namespace uunicode
}  // namespace utils

std::string ConvertVersionNumberTo3DotString(uint32_t number);
std::string ConvertVersionNumberTo2DotString(uint32_t number);
uint32_t ConvertVersionNumberFromString(const std::string& version);

}  // namespace common
