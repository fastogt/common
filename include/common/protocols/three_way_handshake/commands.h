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

#include <inttypes.h>
#include <string>

#include <common/error.h>
#include <common/sprintf.h>

#define END_OF_COMMAND "\r\n"

#define FAIL_COMMAND "fail"
#define SUCCESS_COMMAND "ok"

#define IS_EQUAL_COMMAND(BUF, CMD) BUF&& strncmp(BUF, CMD, sizeof(CMD) - 1) == 0

#define CID_FMT PRIu8

#define GENERATE_REQUEST_FMT(CMD) "%" CID_FMT " %s " CMD END_OF_COMMAND
#define GENERATE_REQUEST_FMT_ARGS(CMD, CMD_FMT) "%" CID_FMT " %s " CMD " " CMD_FMT END_OF_COMMAND

#define GENEATATE_SUCCESS(CMD) "%" CID_FMT " %s " SUCCESS_COMMAND " " CMD END_OF_COMMAND
#define GENEATATE_SUCCESS_FMT(CMD, CMD_FMT) "%" CID_FMT " %s " SUCCESS_COMMAND " " CMD " " CMD_FMT END_OF_COMMAND
#define GENEATATE_FAIL_FMT(CMD, CMD_FMT) "%" CID_FMT " %s " FAIL_COMMAND " " CMD " " CMD_FMT END_OF_COMMAND

#define REQUEST_COMMAND 0
#define RESPONCE_COMMAND 1
#define APPROVE_COMMAND 2

// request
// [uint8_t](0) [hex_string]seq [std::string]command

// responce
// [uint8_t](1) [hex_string]seq [OK|FAIL] [std::string]command args ...

// approve
// [uint8_t](2) [hex_string]seq [OK|FAIL] [std::string]command args ...

namespace common {
namespace protocols {
namespace three_way_handshake {

typedef std::string cmd_seq_t;
typedef uint8_t cmd_id_t;

std::string CmdIdToString(cmd_id_t id);

Error StableCommand(const std::string& command, std::string* stabled_command);
Error ParseCommand(const std::string& command, cmd_id_t* cmd_id, cmd_seq_t* seq_id, std::string* cmd_str);

template <cmd_id_t cmd_id>
class InnerCmd {
 public:
  InnerCmd(const cmd_seq_t& id, const std::string& cmd) : id_(id), cmd_(cmd) {}

  static cmd_id_t GetType() { return cmd_id; }

  cmd_seq_t GetId() const { return id_; }

  const std::string& GetCmd() const { return cmd_; }

 private:
  const cmd_seq_t id_;
  const std::string cmd_;
};

typedef InnerCmd<REQUEST_COMMAND> cmd_request_t;    // SYN
typedef InnerCmd<RESPONCE_COMMAND> cmd_responce_t;  // SYN-ACK
typedef InnerCmd<APPROVE_COMMAND> cmd_approve_t;    // ACK

template <typename... Args>
cmd_request_t MakeRequest(cmd_seq_t id, const char* cmd_fmt, Args... args) {
  std::string buff = MemSPrintf(cmd_fmt, REQUEST_COMMAND, id, args...);
  return cmd_request_t(id, buff);
}

template <typename... Args>
cmd_approve_t MakeApproveResponce(cmd_seq_t id, const char* cmd_fmt, Args... args) {
  std::string buff = MemSPrintf(cmd_fmt, APPROVE_COMMAND, id, args...);
  return cmd_approve_t(id, buff);
}

template <typename... Args>
cmd_responce_t MakeResponce(cmd_seq_t id, const char* cmd_fmt, Args... args) {
  std::string buff = MemSPrintf(cmd_fmt, RESPONCE_COMMAND, id, args...);
  return cmd_responce_t(id, buff);
}

}  // namespace three_way_handshake
}  // namespace protocols
}  // namespace common
