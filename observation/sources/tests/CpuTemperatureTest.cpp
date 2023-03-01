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

#include "gtest/gtest.h"
#include "vulp/observation/sources/CpuTemperature.h"

namespace vulp::observation::sources {

TEST(CpuTemperature, NoWriteIfDisabled) {
  CpuTemperature cpu_temperature("/bogus/temp");
  Dictionary observation;
  ASSERT_FALSE(cpu_temperature.is_disabled());
  ASSERT_NO_THROW(cpu_temperature.write(observation));
  ASSERT_TRUE(cpu_temperature.is_disabled());
}

TEST(CpuTemperature, WriteOnce) {
  CpuTemperature cpu_temperature;
  Dictionary observation;
  ASSERT_NO_THROW(cpu_temperature.write(observation));

  // Write may have failed, e.g. in GitHub Actions
  if (observation.has(cpu_temperature.prefix())) {
    double temperature = observation(cpu_temperature.prefix());
    ASSERT_GT(temperature, 0.0);
    ASSERT_LT(temperature, 100.0);
  }
}

}  // namespace vulp::observation::sources
