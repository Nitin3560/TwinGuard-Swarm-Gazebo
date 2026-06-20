#pragma once

#include <string>

#include "behaviortree_cpp_v3/condition_node.h"
#include "geometry_msgs/msg/point_stamped.hpp"
#include "rclcpp/rclcpp.hpp"

namespace twinguard::nav2_integration
{

class IsAgentTrustworthy : public BT::ConditionNode
{
public:
  IsAgentTrustworthy(const std::string & name, const BT::NodeConfiguration & config);

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("trust_topic", "trust_state", "Topic publishing TwinGuard trust_state"),
      BT::InputPort<double>("authority_threshold", 0.5, "Minimum authority_scale to report SUCCESS"),
      BT::InputPort<double>("stale_timeout_s", 1.0, "Maximum trusted age for trust_state data"),
    };
  }

  BT::NodeStatus tick() override;

private:
  void on_trust_state(const geometry_msgs::msg::PointStamped::SharedPtr msg);

  rclcpp::Node::SharedPtr node_;
  rclcpp::Subscription<geometry_msgs::msg::PointStamped>::SharedPtr sub_;
  double latest_authority_scale_{0.0};
  bool has_data_{false};
  rclcpp::Time last_update_{0, 0, RCL_ROS_TIME};
};

}  // namespace twinguard::nav2_integration
