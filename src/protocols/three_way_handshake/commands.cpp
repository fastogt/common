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

#include <common/protocols/three_way_handshake/commands.h>

#include <stdlib.h>

namespace common {
namespace protocols {
namespace three_way_handshake {

std::string CmdIdToString(cmd_id_t id) {
  static const std::string seq_names[] = {"REQUEST", "RESPONCE", "APPROVE"};
  if (id < SIZEOFMASS(seq_names)) {
    return seq_names[id];
  }

  DNOTREACHED();
  return std::string();
}

common::Error StableCommand(const std::string& command, std::string* stabled_command) {
  if (command.empty() || !stabled_command) {
    return common::make_error("Prepare commands, invalid input");
  }

  size_t pos = command.find_last_of(END_OF_COMMAND);
  if (pos == std::string::npos) {
    return common::make_error("UNKNOWN SEQUENCE: " + command);
  }

  *stabled_command = command.substr(0, pos - 1);
  return common::Error();
}

common::Error ParseCommand(const std::string& command, cmd_id_t* cmd_id, cmd_seq_t* seq_id, std::string* cmd_str) {
  if (command.empty() || !cmd_id || !seq_id || !cmd_str) {
    return common::make_error("Parse command, invalid input");
  }

  std::string stabled_command;
  common::Error err = StableCommand(command, &stabled_command);
  if (err) {
    return err;
  }

  char* star_seq = NULL;
  cmd_id_t lcmd_id = strtoul(stabled_command.c_str(), &star_seq, 10);
  if (*star_seq != ' ') {
    return common::make_error("PROBLEM EXTRACTING SEQUENCE: " + command);
  }

  const char* id_ptr = strchr(star_seq + 1, ' ');
  if (!id_ptr) {
    return common::make_error("PROBLEM EXTRACTING ID: " + command);
  }

  ptrdiff_t len_seq = id_ptr - (star_seq + 1);
  cmd_seq_t lseq_id = cmd_seq_t(star_seq + 1, len_seq);

  *cmd_id = lcmd_id;
  *seq_id = lseq_id;
  *cmd_str = id_ptr + 1;
  return common::Error();
}

}  // namespace three_way_handshake
}  // namespace protocols
}  // namespace common
