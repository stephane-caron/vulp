// Copyright 2022 Stéphane Caron
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <palimpsest/Dictionary.h>

#include "vulp/observation/Observer.h"

namespace vulp::observation::tests {

//! Exception-throwing observer
class ThrowingObserver : public observation::Observer {
 public:
  void read(const palimpsest::Dictionary& observation) override {
    throw std::runtime_error("could not get schwifty");
  }
};

}  // namespace vulp::observation::tests
