#pragma once

#include <string>

#include "geometry_msgs/msg/point_stamped.hpp"
#include "nav2_costmap_2d/layer.hpp"
#include "nav2_costmap_2d/layered_costmap.hpp"
#include "rclcpp/rclcpp.hpp"

namespace twinguard::nav2_integration
{

class TwinGuardIntegrityLayer : public nav2_costmap_2d::Layer
{
public:
  TwinGuardIntegrityLayer() = default;

  void onInitialize() override;
  void updateBounds(
    double robot_x, double robot_y, double robot_yaw,
    double * min_x, double * min_y, double * max_x, double * max_y) override;
  void updateCosts(
    nav2_costmap_2d::Costmap2D & master_grid,
    int min_i, int min_j, int max_i, int max_j) override;
  void reset() override;
  bool isClearable() override { return false; }

private:
  void on_trust_state(const geometry_msgs::msg::PointStamped::SharedPtr msg);

  rclcpp::Subscription<geometry_msgs::msg::PointStamped>::SharedPtr sub_;
  double authority_scale_{1.0};
  bool initialized_{false};
  double inflation_radius_m_{1.5};
  std::string trust_topic_{"trust_state"};
  double robot_x_{0.0};
  double robot_y_{0.0};
};

}  // namespace twinguard::nav2_integration
