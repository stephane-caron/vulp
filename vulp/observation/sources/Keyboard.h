/*
 * Copyright 2023 Inria
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <string>

#include "vulp/observation/Source.h"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

//! Maximum number of bytes to encode a key
constexpr ssize_t kMaxKeyBytes = 3;

//! Polling interval in milliseconds
constexpr int64_t kPollingIntervalMS = 50;

// Scan codes for arrow keys
constexpr unsigned char UP_BYTES[] = {0x1B, 0x5B, 0x41};
constexpr unsigned char DOWN_BYTES[] = {0x1B, 0x5B, 0x42};
constexpr unsigned char RIGHT_BYTES[] = {0x1B, 0x5B, 0x43};
constexpr unsigned char LEFT_BYTES[] = {0x1B, 0x5B, 0x44};

inline bool is_lowercase_alpha(unsigned char c) {
  return 0x61 <= c && c <= 0x7A;
}
inline bool is_uppercase_alpha(unsigned char c) {
  return 0x41 <= c && c <= 0x5A;
}
inline bool is_printable_ascii(unsigned char c) {
  return 0x20 <= c && c <= 0x7F;
}

namespace vulp::observation::sources {
enum class Key {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  W,
  A,
  S,
  D,
  X,
  NONE,    // No key pressed
  UNKNOWN  // Map everything else to this key
};

/*! Source for reading Keyboard inputs.
 *
 * \note This source reads from the standard input, and does
 * not listen to Keyboard events. It can only read one key at a time.
 */
class Keyboard : public Source {
 public:
  /*! Constructor sets up the terminal in non-canonical mode where
   * input is available immediately without waiting for a newline.
   */
  Keyboard();

  //! Destructor
  ~Keyboard() override;

  //! Prefix of output in the observation dictionary.
  inline std::string prefix() const noexcept final { return "keyboard"; }

  /*! Write output to a dictionary.
   *
   * \param[out] output Dictionary to write observations to.
   */
  void write(Dictionary& output) final;

 private:
  //! Read the next key event from STDIN.
  bool read_event();

  /*! Map a character to a key code.
   *
   * \param[in] buf Buffer containing the character.
   * \return Key value
   */
  Key map_char_to_key(unsigned char* buf);

  //! Buffer to store incoming bytes from STDIN
  unsigned char buf_[kMaxKeyBytes];

  //! Key code of the last key pressed
  Key key_code_;

  //! Whether the last key pressed is still pressed
  bool key_pressed_;

  //! Last time a key was pressed in milliseconds
  system_clock::time_point last_key_poll_time_;
};

}  // namespace vulp::observation::sources
