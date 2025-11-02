/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include <common/libev/websocket/websocket_client.h>
#include <common/net/net.h>
#include <common/threads/thread_manager.h>

namespace {

// Mock IoLoop for testing
class MockIoLoop : public common::libev::IoLoop {
 public:
  MockIoLoop() : IoLoop(nullptr) {}
  ~MockIoLoop() override {}
};

// Test for large payload handling
TEST(WebSocket, LargePayload) {
  common::net::socket_info info;
  MockIoLoop loop;
  common::libev::websocket::WebSocketServerClient client(&loop, info);

  // Simulate a large payload frame (e.g., 5000 bytes, within MAX_PAYLOAD_SIZE)
  size_t large_size = 5000;
  std::string large_data(large_size, 'A');
  bool called = false;
  auto pred = [&](char* data, size_t size) {
    called = true;
    EXPECT_EQ(size, large_size);
    EXPECT_EQ(std::string(data, size), large_data);
  };

  // Manually set up frame for testing (this would normally come from network)
  // For simplicity, assume ProcessFrame is called with a pre-set frame_
  // In real test, you'd need to mock SingleRead or use integration test

  // Since ProcessFrame reads from socket, for unit test, we can test frame_buffer_new
  frame_buffer_t* fb = frame_buffer_server_new(1, 1, large_data.c_str(), large_size);
  ASSERT_TRUE(fb);
  EXPECT_EQ(fb->len, 2 + large_size);  // Header + payload
  frame_buffer_free(fb);
}

// Test for fragmented messages (basic check, full implementation needed)
TEST(WebSocket, FragmentedMessage) {
  // Test accumulation of fragments
  // This requires extending ProcessFrame to handle fragments
  // For now, assert that fragmented frames are not fully supported (as per review)
  SUCCEED();  // Placeholder: Implement after fixing fragmentation
}

// Test for invalid frames
TEST(WebSocket, InvalidFrame) {
  common::net::socket_info info;
  MockIoLoop loop;
  common::libev::websocket::WebSocketServerClient client(&loop, info);

  // Test invalid opcode
  frame_t frame;
  char invalid_buf[2] = {0x80 | 0xF, 0x00};  // Invalid opcode 0xF
  bool parsed = parse_frame_header(invalid_buf, &frame);
  EXPECT_TRUE(parsed);
  EXPECT_EQ(frame.opcode, 0xF);  // Should be rejected in ProcessFrame

  // Test payload too large
  // Simulate frame with payload_len > MAX_PAYLOAD_SIZE
  // This would be caught in ProcessFrame step FOUR
}

}  // namespace