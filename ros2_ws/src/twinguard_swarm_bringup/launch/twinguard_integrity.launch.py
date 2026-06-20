from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    mission_mode = LaunchConfiguration("mission_mode")
    mission_radius_m = LaunchConfiguration("mission_radius_m")
    mission_period_s = LaunchConfiguration("mission_period_s")
    static_obstacle_enabled = LaunchConfiguration("static_obstacle_enabled")
    static_obstacle_x_m = LaunchConfiguration("static_obstacle_x_m")
    static_obstacle_y_m = LaunchConfiguration("static_obstacle_y_m")
    static_obstacle_z_m = LaunchConfiguration("static_obstacle_z_m")
    static_obstacle_radius_m = LaunchConfiguration("static_obstacle_radius_m")

    return LaunchDescription(
        [
            DeclareLaunchArgument("mission_mode", default_value="hold"),
            DeclareLaunchArgument("mission_radius_m", default_value="3.0"),
            DeclareLaunchArgument("mission_period_s", default_value="18.0"),
            DeclareLaunchArgument("static_obstacle_enabled", default_value="false"),
            DeclareLaunchArgument("static_obstacle_x_m", default_value="1.5"),
            DeclareLaunchArgument("static_obstacle_y_m", default_value="0.0"),
            DeclareLaunchArgument("static_obstacle_z_m", default_value="-2.0"),
            DeclareLaunchArgument("static_obstacle_radius_m", default_value="1.0"),
            Node(
                package="twinguard_swarm_integrity_cpp",
                executable="integrity_node_cpp",
                name="integrity_drone_0_cpp",
                namespace="drone_0/twinguard",
                output="screen",
                parameters=[
                    {
                        "drone_id": 0,
                        "stale_timeout_ms": 500,
                        "prediction_dt": 0.1,
                    }
                ],
                remappings=[
                    ("fmu/out/vehicle_odometry", "/drone_0/fmu/out/vehicle_odometry"),
                ],
            ),
            Node(
                package="twinguard_swarm_integrity_cpp",
                executable="formation_supervisor_node",
                name="formation_supervisor_drone_0",
                namespace="drone_0/twinguard",
                output="screen",
                parameters=[
                    {
                        "drone_id": 0,
                        "target_system": 1,
                        "nominal_x_m": 0.0,
                        "nominal_y_m": 0.0,
                        "nominal_z_m": -2.0,
                        "nominal_velocity_limit_mps": 3.0,
                        "degraded_threshold": 0.5,
                        "stale_timeout_ms": 500,
                        "setpoint_rate_hz": 10.0,
                        "auto_arm": False,
                        "force_arm": False,
                        "mission_mode": ParameterValue(mission_mode, value_type=str),
                        "mission_radius_m": ParameterValue(mission_radius_m, value_type=float),
                        "mission_period_s": ParameterValue(mission_period_s, value_type=float),
                        "static_obstacle_enabled": ParameterValue(
                            static_obstacle_enabled, value_type=bool
                        ),
                        "static_obstacle_x_m": ParameterValue(static_obstacle_x_m, value_type=float),
                        "static_obstacle_y_m": ParameterValue(static_obstacle_y_m, value_type=float),
                        "static_obstacle_z_m": ParameterValue(static_obstacle_z_m, value_type=float),
                        "static_obstacle_radius_m": ParameterValue(
                            static_obstacle_radius_m, value_type=float
                        ),
                    }
                ],
                remappings=[
                    ("fmu/out/vehicle_odometry", "/drone_0/fmu/out/vehicle_odometry"),
                    ("fmu/in/offboard_control_mode", "/drone_0/fmu/in/offboard_control_mode"),
                    ("fmu/in/trajectory_setpoint", "/drone_0/fmu/in/trajectory_setpoint"),
                    ("fmu/in/vehicle_command", "/drone_0/fmu/in/vehicle_command"),
                ],
            ),
        ]
    )
