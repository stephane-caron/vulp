// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 Stéphane Caron
/*
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *     Copyright 2020 Josh Pieper, jjp@pobox.com.
 *     SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "vulp/actuation/moteus/QueryResult.h"

namespace vulp::actuation::moteus {

//! Reply from servo
struct ServoReply {
  //! Servo identifier
  int id = 0;

  //! Content of servo reply, with resolution from \ref ServoCommand::query
  QueryResult result;
};

}  // namespace vulp::actuation::moteus
