#pragma once

#include <array>
#include <string>

namespace twinguard::offboard
{

enum class SupervisorMode { INIT, ARM_AND_OFFBOARD, NOMINAL, DEGRADED_HOLD, RECOVERING };

struct SetpointCommand
{
  std::array<double, 3> position{0.0, 0.0, 0.0};
  double yaw{0.0};
  double velocity_limit{0.0};
  bool hold{false};
};

struct CircleMissionParams
{
  double center_x{0.0};
  double center_y{0.0};
  double altitude_m{2.0};
  double radius_m{3.0};
  double period_s{18.0};
  std::string mode{"circle"};
};

class OffboardSupervisor
{
public:
  OffboardSupervisor(double nominal_velocity_limit = 3.0, double degraded_threshold = 0.5);

  SetpointCommand step(
    double authority_scale,
    const std::string & fault_label,
    const std::array<double, 3> & current_position,
    const std::array<double, 3> & nominal_setpoint,
    double yaw = 0.0);

  SupervisorMode mode() const { return mode_; }

private:
  double nominal_velocity_limit_;
  double degraded_threshold_;
  SupervisorMode mode_{SupervisorMode::INIT};
};

const char * to_string(SupervisorMode mode);

std::array<double, 3> circle_mission_setpoint(
  const CircleMissionParams & params,
  double elapsed_s,
  double authority_scale);

double circle_mission_yaw(
  const CircleMissionParams & params,
  double elapsed_s,
  double authority_scale);

}  // namespace twinguard::offboard
