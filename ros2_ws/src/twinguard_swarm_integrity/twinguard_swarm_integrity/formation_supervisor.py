from __future__ import annotations

import rclpy
from diagnostic_msgs.msg import DiagnosticArray, DiagnosticStatus, KeyValue
from rclpy.node import Node


class FormationSupervisor(Node):
    """Placeholder formation supervisor for PX4 offboard setpoint binding."""

    def __init__(self) -> None:
        super().__init__("twinguard_formation_supervisor")
        self.declare_parameter("num_drones", 3)
        self.diag_pub = self.create_publisher(DiagnosticArray, "mission_diagnostics", 10)
        self.timer = self.create_timer(0.2, self._tick)

    def _tick(self) -> None:
        diag = DiagnosticArray()
        status = DiagnosticStatus()
        status.name = "twinguard/formation_supervisor"
        status.level = DiagnosticStatus.OK
        status.message = "ready_for_px4_offboard_binding"
        status.values = [KeyValue(key="mode", value="formation_hold")]
        diag.status.append(status)
        self.diag_pub.publish(diag)


def main(args=None) -> None:
    rclpy.init(args=args)
    node = FormationSupervisor()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()
