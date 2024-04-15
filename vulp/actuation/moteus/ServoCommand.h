// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 Stéphane Caron
/*
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *     SPDX-License-Identifier: Apache-2.0
 *     Copyright 2020 Josh Pieper, jjp@pobox.com.
 */

#pragma once

#include "vulp/actuation/moteus/Mode.h"
#include "vulp/actuation/moteus/PositionCommand.h"
#include "vulp/actuation/moteus/PositionResolution.h"
#include "vulp/actuation/moteus/QueryCommand.h"

namespace vulp::actuation::moteus {

//! Command sent to a servo
struct ServoCommand {
  //! Servo identifier
  int id = 0;

  //! Servo mode
  moteus::Mode mode = moteus::Mode::kStopped;

  //! Position commands, used when mode == kPosition (or kZeroVelocity)
  moteus::PositionCommand position;

  //! Resolution for each \ref position field
  moteus::PositionResolution resolution;

  //! Resolution required for each reply field in \ref ServoReply::result
  moteus::QueryCommand query;
};

}  // namespace vulp::actuation::moteus
