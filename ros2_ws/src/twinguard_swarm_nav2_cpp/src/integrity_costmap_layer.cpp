#include "twinguard_swarm_nav2_cpp/integrity_costmap_layer.hpp"

#include <algorithm>
#include <cmath>
#include <functional>

#include "nav2_costmap_2d/cost_values.hpp"
#include "pluginlib/class_list_macros.hpp"

namespace twinguard::nav2_integration
{

void TwinGuardIntegrityLayer::onInitialize()
{
  auto node = node_.lock();
  if (!node) {
    initialized_ = false;
    current_ = true;
    return;
  }

  declareParameter("enabled", rclcpp::ParameterValue(true));
  declareParameter("trust_topic", rclcpp::ParameterValue(std::string("trust_state")));
  declareParameter("inflation_radius_m", rclcpp::ParameterValue(1.5));

  node->get_parameter(name_ + ".enabled", enabled_);
  node->get_parameter(name_ + ".trust_topic", trust_topic_);
  node->get_parameter(name_ + ".inflation_radius_m", inflation_radius_m_);

  sub_ = node->create_subscription<geometry_msgs::msg::PointStamped>(
    trust_topic_,
    rclcpp::SensorDataQoS(),
    std::bind(&TwinGuardIntegrityLayer::on_trust_state, this, std::placeholders::_1));

  current_ = true;
  initialized_ = true;
}

void TwinGuardIntegrityLayer::updateBounds(
  double robot_x,
  double robot_y,
  double /*robot_yaw*/,
  double * min_x,
  double * min_y,
  double * max_x,
  double * max_y)
{
  robot_x_ = robot_x;
  robot_y_ = robot_y;

  if (!initialized_ || !enabled_) {
    return;
  }

  const double radius = inflation_radius_m_ * (1.0 - std::clamp(authority_scale_, 0.0, 1.0));
  if (radius <= 0.0) {
    return;
  }

  *min_x = std::min(*min_x, robot_x - radius);
  *min_y = std::min(*min_y, robot_y - radius);
  *max_x = std::max(*max_x, robot_x + radius);
  *max_y = std::max(*max_y, robot_y + radius);
}

void TwinGuardIntegrityLayer::updateCosts(
  nav2_costmap_2d::Costmap2D & master_grid,
  int min_i,
  int min_j,
  int max_i,
  int max_j)
{
  if (!initialized_ || !enabled_ || !layered_costmap_ || authority_scale_ >= 0.999) {
    return;
  }

  const double distrust = 1.0 - std::clamp(authority_scale_, 0.0, 1.0);
  const double radius_m = inflation_radius_m_ * distrust;
  if (radius_m <= 0.0) {
    return;
  }

  unsigned int robot_i = 0;
  unsigned int robot_j = 0;
  if (!master_grid.worldToMap(robot_x_, robot_y_, robot_i, robot_j)) {
    return;
  }

  const double resolution = master_grid.getResolution();
  const double radius_cells = std::max(radius_m / std::max(resolution, 1e-6), 1.0);
  const unsigned char extra_cost = static_cast<unsigned char>(std::clamp(distrust * 200.0, 0.0, 200.0));

  const int start_i = std::max(min_i, 0);
  const int start_j = std::max(min_j, 0);
  const int end_i = std::min(max_i, static_cast<int>(master_grid.getSizeInCellsX()));
  const int end_j = std::min(max_j, static_cast<int>(master_grid.getSizeInCellsY()));

  for (int j = start_j; j < end_j; ++j) {
    for (int i = start_i; i < end_i; ++i) {
      const double dx = static_cast<double>(i) - static_cast<double>(robot_i);
      const double dy = static_cast<double>(j) - static_cast<double>(robot_j);
      const double distance = std::hypot(dx, dy);
      if (distance > radius_cells) {
        continue;
      }

      const double taper = 1.0 - (distance / radius_cells);
      const auto current_cost = master_grid.getCost(i, j);
      if (current_cost == nav2_costmap_2d::NO_INFORMATION ||
        current_cost == nav2_costmap_2d::LETHAL_OBSTACLE)
      {
        continue;
      }

      const auto added_cost = static_cast<unsigned char>(extra_cost * taper);
      master_grid.setCost(i, j, std::max(current_cost, added_cost));
    }
  }
}

void TwinGuardIntegrityLayer::reset()
{
  current_ = true;
}

void TwinGuardIntegrityLayer::on_trust_state(
  const geometry_msgs::msg::PointStamped::SharedPtr msg)
{
  authority_scale_ = std::clamp(msg->point.z, 0.0, 1.0);
  current_ = true;
}

}  // namespace twinguard::nav2_integration

PLUGINLIB_EXPORT_CLASS(
  twinguard::nav2_integration::TwinGuardIntegrityLayer,
  nav2_costmap_2d::Layer)
