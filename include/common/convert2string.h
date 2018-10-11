/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include <common/string_piece.h>
#include <common/types.h>

namespace common {

std::string EscapedText(const std::string& str);  // add \n at the end if needed

string16 ConvertToString16(const char* from);
string16 ConvertToString16(const std::string& from);
string16 ConvertToString16(const StringPiece& from);
string16 ConvertToString16(const buffer_t& from);
string16 ConvertToString16(const string16& from);
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
std::string ConvertToString(const buffer_t& from);
std::string ConvertToString(const string16& from);
std::string ConvertToString(const StringPiece16& from);
std::string ConvertToString(const std::string& from);
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

// buffer_t

buffer_t ConvertToBytes(const std::string& from);
buffer_t ConvertToBytes(const string16& from);
buffer_t ConvertToBytes(const buffer_t& from);

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

bool ConvertFromString16(const string16& from, StringPiece* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, std::string* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, buffer_t* out) WARN_UNUSED_RESULT;
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
bool ConvertFromString16(const string16& from, string16* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, StringPiece16* out) WARN_UNUSED_RESULT;

bool ConvertFromString(const std::string& from, StringPiece16* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, string16* out) WARN_UNUSED_RESULT;
#if defined(WCHAR_T_IS_UTF16)
#else
bool ConvertFromString(const std::string& from, std::wstring* out);
#endif
bool ConvertFromString(const std::string& from, buffer_t* out) WARN_UNUSED_RESULT;
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
bool ConvertFromString(const std::string& from, std::string* out) WARN_UNUSED_RESULT;
bool ConvertFromString(const std::string& from, StringPiece* out) WARN_UNUSED_RESULT;

bool ConvertFromBytes(const buffer_t& from, string16* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, std::string* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, bool* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, char* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, unsigned char* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, short* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, unsigned short* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, int* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, unsigned int* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, long* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, unsigned long* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, long long* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, unsigned long long* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, float* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, double* out) WARN_UNUSED_RESULT;
bool ConvertFromBytes(const buffer_t& from, buffer_t* out) WARN_UNUSED_RESULT;

namespace utils {
namespace hex {

buffer_t encode(const buffer_t& input, bool is_lower);
std::string encode(const StringPiece& input, bool is_lower);

buffer_t decode(const buffer_t& input);
std::string decode(const StringPiece& input);

}  // namespace hex

namespace xhex {

buffer_t encode(const buffer_t& input, bool is_lower);
std::string encode(const StringPiece& input, bool is_lower);

buffer_t decode(const buffer_t& input);
std::string decode(const StringPiece& input);

}  // namespace xhex

namespace unicode {

std::string encode(const StringPiece16& input, bool is_lower);
string16 decode(const StringPiece& input);

}  // namespace unicode

namespace uunicode {

std::string encode(const StringPiece16& input, bool is_lower);
string16 decode(const StringPiece& input);

}  // namespace uunicode
}  // namespace utils

bool HexStringToInt(const StringPiece& input, int32_t* output);
bool HexStringToUInt(const StringPiece& input, uint32_t* output);
bool HexStringToInt64(const StringPiece& input, int64_t* output);
bool HexStringToUInt64(const StringPiece& input, uint64_t* output);
bool HexStringToBytes(const std::string& input, std::vector<uint8_t>* output);

std::string ConvertVersionNumberTo3DotString(uint32_t number);
std::string ConvertVersionNumberTo2DotString(uint32_t number);
uint32_t ConvertVersionNumberFromString(const std::string& version);

}  // namespace common
