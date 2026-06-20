#include "twinguard_swarm_nav2_cpp/is_agent_trustworthy.hpp"

#include <algorithm>
#include <functional>
#include <string>

#include "behaviortree_cpp_v3/bt_factory.h"

namespace twinguard::nav2_integration
{

IsAgentTrustworthy::IsAgentTrustworthy(
  const std::string & name,
  const BT::NodeConfiguration & config)
: BT::ConditionNode(name, config)
{
  node_ = config.blackboard->get<rclcpp::Node::SharedPtr>("node");

  std::string topic{"trust_state"};
  getInput("trust_topic", topic);
  sub_ = node_->create_subscription<geometry_msgs::msg::PointStamped>(
    topic,
    rclcpp::SensorDataQoS(),
    std::bind(&IsAgentTrustworthy::on_trust_state, this, std::placeholders::_1));
}

BT::NodeStatus IsAgentTrustworthy::tick()
{
  double threshold = 0.5;
  double stale_timeout_s = 1.0;
  getInput("authority_threshold", threshold);
  getInput("stale_timeout_s", stale_timeout_s);

  if (!has_data_) {
    return BT::NodeStatus::FAILURE;
  }

  const auto now = node_->get_clock()->now();
  if ((now - last_update_).seconds() > std::max(stale_timeout_s, 0.0)) {
    return BT::NodeStatus::FAILURE;
  }

  return latest_authority_scale_ >= threshold ?
    BT::NodeStatus::SUCCESS :
    BT::NodeStatus::FAILURE;
}

void IsAgentTrustworthy::on_trust_state(
  const geometry_msgs::msg::PointStamped::SharedPtr msg)
{
  latest_authority_scale_ = std::clamp(msg->point.z, 0.0, 1.0);
  has_data_ = true;
  last_update_ = node_->get_clock()->now();
}

}  // namespace twinguard::nav2_integration

BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<twinguard::nav2_integration::IsAgentTrustworthy>(
    "IsAgentTrustworthy");
}
