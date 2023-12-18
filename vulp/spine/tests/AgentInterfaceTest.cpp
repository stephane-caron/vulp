// Copyright 2022 Stéphane Caron
// SPDX-License-Identifier: Apache-2.0

#include <palimpsest/Dictionary.h>

#include <cmath>
#include <filesystem>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "vulp/spine/AgentInterface.h"
#include "vulp/utils/random_string.h"

namespace vulp::spine {

class AgentInterfaceTest : public ::testing::Test {
 protected:
  void SetUp() override {
    agent_interface_ =
        std::make_unique<AgentInterface>("/" + utils::random_string(), 1024);
  }

  void TearDown() override {}

  std::unique_ptr<AgentInterface> agent_interface_;
};

TEST_F(AgentInterfaceTest, GetSetRequest) {
  agent_interface_->set_request(Request::kObservation);
  ASSERT_EQ(agent_interface_->request(), Request::kObservation);
  agent_interface_->set_request(Request::kAction);
  ASSERT_EQ(agent_interface_->request(), Request::kAction);
}

TEST_F(AgentInterfaceTest, ReadWriteData) {
  char data[] = "foobar\0";
  const size_t size = 7;
  agent_interface_->write(data, size);
  ASSERT_STREQ(agent_interface_->data(), data);
  ASSERT_EQ(agent_interface_->size(), size);
}

}  // namespace vulp::spine
