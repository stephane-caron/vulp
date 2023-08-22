/*
 * Copyright 2022 Stéphane Caron
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

#include <algorithm>
#include <random>
#include <string>

namespace vulp::utils {

/*! Generate a random string.
 *
 * \param[in] length String length.
 *
 * \return Random string.
 *
 * The generated string contains only alphanumeric characters with no
 * repetition.
 */
inline std::string random_string(unsigned length = 16) {
  std::string alphanum(
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
  length = (length <= 62) ? length : 62;  // 62 == alphanum.size()
  std::random_device device;
  std::mt19937 generator(device());
  std::shuffle(alphanum.begin(), alphanum.end(), generator);
  return alphanum.substr(0, length);
}

}  // namespace vulp::utils
