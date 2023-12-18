// Copyright 2022 Stéphane Caron
// SPDX-License-Identifier: Apache-2.0
/*
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *     Copyright 2020 Josh Pieper, jjp@pobox.com.
 *     SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace vulp::actuation::moteus {

//! Output information forwarded to an interface's callback function.
struct Output {
  //! Size of the replies array.
  size_t query_result_size = 0;
};

}  // namespace vulp::actuation::moteus
