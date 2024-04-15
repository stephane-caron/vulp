// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 Stéphane Caron
// Copyright 2023 Inria

#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "tools/cpp/runfiles/runfiles.h"
#include "vulp/actuation/BulletInterface.h"

namespace vulp::actuation {

using bazel::tools::cpp::runfiles::Runfiles;

constexpr double kNoFriction = 1e-5;
constexpr double kLeftWheelFriction = 0.1;

class BulletInterfaceTest : public ::testing::Test {
 protected:
  //! Set up a new test fixture
  void SetUp() override {
    Dictionary config;
    config("bullet")("gui") = false;

    ServoLayout layout;
    layout.add_servo(1, 1, "right_hip");
    layout.add_servo(2, 1, "right_knee");
    layout.add_servo(3, 1, "right_wheel");
    layout.add_servo(4, 2, "left_hip");
    layout.add_servo(5, 2, "left_knee");
    layout.add_servo(6, 2, "left_wheel");

    std::string error;
    std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest(&error));
    ASSERT_NE(runfiles, nullptr);

    BulletInterface::Parameters params(config);
    params.dt = 1.0 / 1000.0;
    params.floor = false;  // wheels roll freely during testing
    params.joint_friction.try_emplace("left_wheel", kLeftWheelFriction);
    params.robot_urdf_path =
        runfiles->Rlocation("upkie_description/urdf/upkie.urdf");
    interface_ = std::make_unique<BulletInterface>(layout, params);

    for (const auto& pair : layout.servo_joint_map()) {
      commands_.push_back({});
      commands_.back().id = pair.first;
    }
    replies_.resize(commands_.size());
    data_.commands = {commands_.data(), commands_.size()};
    data_.replies = {replies_.data(), replies_.size()};
  }

 protected:
  //! Bullet actuation interface
  std::unique_ptr<BulletInterface> interface_;

  //! Servo commands placeholder
  std::vector<actuation::moteus::ServoCommand> commands_;

  //! Servo replies placeholder
  std::vector<actuation::moteus::ServoReply> replies_;

  //! Data buffer to call interface_->cycle()
  moteus::Data data_;
};

TEST_F(BulletInterfaceTest, CycleCallsCallback) {
  moteus::Data data;
  bool callback_called = false;
  interface_->cycle(data, [&callback_called](const moteus::Output& output) {
    callback_called = true;
  });
  ASSERT_TRUE(callback_called);
}

TEST_F(BulletInterfaceTest, JointProperties) {
  const auto& joint_props = interface_->joint_properties();

  ASSERT_NO_THROW(joint_props.at("left_hip"));
  ASSERT_NO_THROW(joint_props.at("left_knee"));
  ASSERT_NO_THROW(joint_props.at("left_wheel"));
  ASSERT_NO_THROW(joint_props.at("right_hip"));
  ASSERT_NO_THROW(joint_props.at("right_knee"));
  ASSERT_NO_THROW(joint_props.at("right_wheel"));

  ASSERT_LT(joint_props.at("left_hip").friction, kNoFriction);
  ASSERT_LT(joint_props.at("left_knee").friction, kNoFriction);
  ASSERT_GE(joint_props.at("left_wheel").friction, kLeftWheelFriction);
  ASSERT_LT(joint_props.at("right_hip").friction, kNoFriction);
  ASSERT_LT(joint_props.at("right_knee").friction, kNoFriction);
  ASSERT_LT(joint_props.at("right_wheel").friction, kNoFriction);

  ASSERT_GT(joint_props.at("left_hip").maximum_torque, 5.0);
  ASSERT_GT(joint_props.at("left_knee").maximum_torque, 5.0);
  ASSERT_GT(joint_props.at("left_wheel").maximum_torque, 0.5);
  ASSERT_GT(joint_props.at("right_hip").maximum_torque, 5.0);
  ASSERT_GT(joint_props.at("right_knee").maximum_torque, 5.0);
  ASSERT_GT(joint_props.at("right_wheel").maximum_torque, 0.5);
}

TEST_F(BulletInterfaceTest, CycleDoesntThrow) {
  ASSERT_NO_THROW(
      interface_->cycle(data_, [](const moteus::Output& output) {}));
}

TEST_F(BulletInterfaceTest, ResetBaseState) {
  Dictionary config;
  config("bullet")("gui") = false;
  config("bullet")("reset")("orientation_base_in_world") =
      Eigen::Quaterniond(0.707, 0.0, -0.707, 0.0);
  config("bullet")("reset")("position_base_in_world") =
      Eigen::Vector3d(0.0, 0.0, 1.0);
  config("bullet")("reset")("linear_velocity_base_to_world_in_world") =
      Eigen::Vector3d(4.0, 5.0, 6.0);
  config("bullet")("reset")("angular_velocity_base_in_base") =
      Eigen::Vector3d(7.0, 8.0, 9.0);
  interface_->reset(config);

  const Eigen::Matrix4d T = interface_->transform_base_to_world();
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  EXPECT_NEAR(R(0, 0), 0.0, 1e-7);
  EXPECT_NEAR(R(0, 2), -1.0, 1e-7);
  EXPECT_NEAR(R(1, 1), 1.0, 1e-7);
  EXPECT_NEAR(R(2, 0), 1.0, 1e-7);
  EXPECT_NEAR(R(2, 2), 0.0, 1e-7);

  const Eigen::Vector3d p = T.block<3, 1>(0, 3);
  ASSERT_DOUBLE_EQ(p.x(), 0.0);
  ASSERT_DOUBLE_EQ(p.y(), 0.0);
  ASSERT_DOUBLE_EQ(p.z(), 1.0);

  const Eigen::Vector3d v =
      interface_->linear_velocity_base_to_world_in_world();
  ASSERT_DOUBLE_EQ(v.x(), 4.0);
  ASSERT_DOUBLE_EQ(v.y(), 5.0);
  ASSERT_DOUBLE_EQ(v.z(), 6.0);

  const Eigen::Vector3d omega = interface_->angular_velocity_base_in_base();
  ASSERT_NEAR(omega.x(), 7.0, 1e-3);
  ASSERT_NEAR(omega.y(), 8.0, 1e-3);
  ASSERT_NEAR(omega.z(), 9.0, 7e-3);
}

TEST_F(BulletInterfaceTest, ComputeJointTorquesStopped) {
  // Commands in data_ have defaults, thus in moteus::Mode::kStopped
  interface_->cycle(data_, [](const moteus::Output& output) {});

  // Stopped joint and no target => no velocity and no torque
  const auto& measurements = interface_->servo_reply().at("left_wheel").result;
  const double no_feedforward_torque = 0.0;
  const double no_position = std::numeric_limits<double>::quiet_NaN();
  const double target_velocity = measurements.velocity * (2.0 * M_PI);
  double tau = interface_->compute_joint_torque(
      "left_wheel", no_feedforward_torque, no_position, target_velocity, 1.0,
      1.0, 1.0);
  ASSERT_NEAR(measurements.velocity, 0.0, 1e-3);  // should be zero here
  ASSERT_NEAR(tau, 0.0, 1e-3);
}

TEST_F(BulletInterfaceTest, ComputeJointTorquesWhileMoving) {
  const double no_feedforward_torque = 0.0;
  const double no_position = std::numeric_limits<double>::quiet_NaN();
  for (auto& command : data_.commands) {
    command.mode = moteus::Mode::kPosition;
    command.position.position = no_position;
    command.position.velocity = 1.0;  // rev/s
    command.position.kp_scale = 1.0;
    command.position.kd_scale = 1.0;
    command.position.maximum_torque = 1.0;  // N.m
  }

  // Cycle a couple of times so that both wheels spin up
  interface_->cycle(data_, [](const moteus::Output& output) {});
  interface_->cycle(data_, [](const moteus::Output& output) {});
  interface_->cycle(data_, [](const moteus::Output& output) {});

  // Right wheel has no kinetic friction
  const auto& right_wheel = interface_->servo_reply().at("right_wheel").result;
  const double right_target_velocity = right_wheel.velocity * (2.0 * M_PI);
  const double right_torque = interface_->compute_joint_torque(
      "right_wheel", no_feedforward_torque, no_position, right_target_velocity,
      1.0, 1.0, 1.0);
  ASSERT_GT(right_wheel.velocity, 0.1);
  ASSERT_NEAR(right_torque, 0.0, 1e-3);

  // Left wheel has kinetic friction
  const auto& left_wheel = interface_->servo_reply().at("left_wheel").result;
  const double left_target_velocity = left_wheel.velocity * (2.0 * M_PI);
  const double left_torque = interface_->compute_joint_torque(
      "left_wheel", no_feedforward_torque, no_position, left_target_velocity,
      1.0, 1.0, 1.0);
  ASSERT_GT(left_wheel.velocity, 0.1);                  // positive velocity
  ASSERT_NEAR(left_torque, -kLeftWheelFriction, 1e-3);  // negative torque
}

TEST_F(BulletInterfaceTest, ComputeJointFeedforwardTorque) {
  const double no_position = std::numeric_limits<double>::quiet_NaN();
  for (auto& command : data_.commands) {
    command.mode = moteus::Mode::kPosition;
    command.position.position = no_position;
    command.position.velocity = 0.0;  // rev/s
    command.position.kp_scale = 0.0;
    command.position.kd_scale = 0.0;
    command.position.feedforward_torque = 0.42;  // N.m
    command.position.maximum_torque = 1.0;       // N.m
  }

  // Cycle a couple of times so that both wheels spin up
  interface_->cycle(data_, [](const moteus::Output& output) {});
  interface_->cycle(data_, [](const moteus::Output& output) {});
  interface_->cycle(data_, [](const moteus::Output& output) {});

  // Right wheel has no kinetic friction
  const auto& right_wheel_reply = interface_->servo_reply().at("right_wheel");
  ASSERT_NEAR(right_wheel_reply.result.torque, 0.42, 1e-3);
}

TEST_F(BulletInterfaceTest, JointRepliesHaveTemperature) {
  interface_->cycle(data_, [](const moteus::Output& output) {});
  for (const auto& pair : interface_->servo_reply()) {
    const auto& reply = pair.second;
    ASSERT_GT(reply.result.temperature, 0.0);
    ASSERT_LT(reply.result.temperature, 100.0);
  }
}

TEST_F(BulletInterfaceTest, JointRepliesHaveVoltage) {
  interface_->cycle(data_, [](const moteus::Output& output) {});
  for (const auto& pair : interface_->servo_reply()) {
    const auto& reply = pair.second;
    ASSERT_GT(reply.result.voltage, 10.0);  // moteus min 10 V
    ASSERT_LT(reply.result.voltage, 44.0);  // moteus max 44 V
  }
}

}  // namespace vulp::actuation
