#include "twinguard_swarm_integrity_cpp/trust_scorer.hpp"

#include <array>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "diagnostic_msgs/msg/diagnostic_array.hpp"
#include "diagnostic_msgs/msg/diagnostic_status.hpp"
#include "diagnostic_msgs/msg/key_value.hpp"
#include "geometry_msgs/msg/point_stamped.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

namespace twinguard::integrity
{

class IntegrityNode : public rclcpp::Node
{
public:
  IntegrityNode()
  : Node("integrity_node_cpp")
  {
    drone_id_ = declare_parameter<int>("drone_id", 0);
    measured_position_ = declare_parameter<std::vector<double>>(
      "initial_measured_position", {0.0, 0.0, 1.5});
    predicted_position_ = declare_parameter<std::vector<double>>(
      "initial_predicted_position", {0.0, 0.0, 1.5});

    diagnostics_pub_ = create_publisher<diagnostic_msgs::msg::DiagnosticArray>(
      "integrity_diagnostics", 10);
    trust_pub_ = create_publisher<geometry_msgs::msg::PointStamped>(
      "trust_state", 10);

    timer_ = create_wall_timer(100ms, [this]() { publish_score(); });
  }

private:
  static std::array<double, 3> to_array(const std::vector<double> & values)
  {
    std::array<double, 3> out{0.0, 0.0, 0.0};
    for (std::size_t i = 0; i < out.size() && i < values.size(); ++i) {
      out[i] = values[i];
    }
    return out;
  }

  static diagnostic_msgs::msg::KeyValue kv(const std::string & key, const std::string & value)
  {
    diagnostic_msgs::msg::KeyValue item;
    item.key = key;
    item.value = value;
    return item;
  }

  void publish_score()
  {
    const auto measured = to_array(measured_position_);
    const auto predicted = to_array(predicted_position_);
    const TrustState state = scorer_.update(measured, predicted);

    diagnostic_msgs::msg::DiagnosticStatus status;
    status.name = "twinguard_integrity_drone_" + std::to_string(drone_id_);
    status.hardware_id = "uav_" + std::to_string(drone_id_);
    status.level = state.fault_label == "nominal" ?
      diagnostic_msgs::msg::DiagnosticStatus::OK :
      diagnostic_msgs::msg::DiagnosticStatus::WARN;
    status.message = state.fault_label;
    status.values.push_back(kv("trust", std::to_string(state.trust)));
    status.values.push_back(kv("residual", std::to_string(state.residual)));
    status.values.push_back(kv("authority_scale", std::to_string(state.authority_scale)));

    diagnostic_msgs::msg::DiagnosticArray diagnostics;
    diagnostics.header.stamp = get_clock()->now();
    diagnostics.status.push_back(status);
    diagnostics_pub_->publish(diagnostics);

    geometry_msgs::msg::PointStamped trust_state;
    trust_state.header.stamp = diagnostics.header.stamp;
    trust_state.header.frame_id = "map";
    trust_state.point.x = state.trust;
    trust_state.point.y = state.residual;
    trust_state.point.z = state.authority_scale;
    trust_pub_->publish(trust_state);
  }

  int drone_id_{0};
  std::vector<double> measured_position_;
  std::vector<double> predicted_position_;
  TrustScorer scorer_;
  rclcpp::Publisher<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr diagnostics_pub_;
  rclcpp::Publisher<geometry_msgs::msg::PointStamped>::SharedPtr trust_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace twinguard::integrity

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<twinguard::integrity::IntegrityNode>());
  rclcpp::shutdown();
  return 0;
}
