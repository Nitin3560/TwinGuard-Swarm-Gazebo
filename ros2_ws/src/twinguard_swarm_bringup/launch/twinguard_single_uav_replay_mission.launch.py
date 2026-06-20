from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.actions import Node


def generate_launch_description():
    dataset_csv = LaunchConfiguration("dataset_csv")
    error_scale = LaunchConfiguration("error_scale")
    max_offset_m = LaunchConfiguration("max_offset_m")
    replay_rate_hz = LaunchConfiguration("replay_rate_hz")
    mission_radius_m = LaunchConfiguration("mission_radius_m")
    takeoff_altitude_m = LaunchConfiguration("takeoff_altitude_m")
    nominal_z_m = PythonExpression(["-", takeoff_altitude_m])

    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "dataset_csv",
                default_value="/home/nitin/scenario23_seq20_real_channel_timeseries.csv",
            ),
            DeclareLaunchArgument("error_scale", default_value="0.1"),
            DeclareLaunchArgument("max_offset_m", default_value="3.0"),
            DeclareLaunchArgument("replay_rate_hz", default_value="20.0"),
            DeclareLaunchArgument("mission_radius_m", default_value="3.0"),
            DeclareLaunchArgument("takeoff_altitude_m", default_value="2.0"),
            Node(
                package="twinguard_dataset_replay",
                executable="dataset_replay_node",
                name="dataset_replay_node",
                namespace="twinguard",
                output="screen",
                parameters=[
                    {
                        "dataset_csv": dataset_csv,
                        "replay_rate_hz": replay_rate_hz,
                        "error_scale": error_scale,
                        "max_offset_m": max_offset_m,
                        "loop": True,
                        "perturb_axis": "x",
                    }
                ],
                remappings=[
                    ("input_vehicle_odometry", "/fmu/out/vehicle_odometry"),
                    ("output_vehicle_odometry", "/twinguard/replay/vehicle_odometry"),
                ],
            ),
            Node(
                package="twinguard_swarm_integrity_cpp",
                executable="integrity_node_cpp",
                name="integrity_drone_0_cpp",
                namespace="twinguard",
                output="screen",
                parameters=[
                    {
                        "drone_id": 0,
                        "stale_timeout_ms": 500,
                        "prediction_dt": 0.1,
                    }
                ],
                remappings=[
                    ("fmu/out/vehicle_odometry", "/twinguard/replay/vehicle_odometry"),
                ],
            ),
            Node(
                package="twinguard_swarm_integrity_cpp",
                executable="formation_supervisor_node",
                name="formation_supervisor_node",
                namespace="twinguard",
                output="screen",
                parameters=[
                    {
                        "drone_id": 0,
                        "target_system": 1,
                        "nominal_z_m": nominal_z_m,
                        "mission_radius_m": mission_radius_m,
                        "mission_period_s": 18.0,
                        "min_authority_scale": 0.25,
                        "nominal_velocity_limit_mps": 3.0,
                        "degraded_threshold": 0.5,
                        "setpoint_rate_hz": 20.0,
                        "auto_arm": True,
                        "force_arm": True,
                        "mission_mode": "circle",
                    }
                ],
                remappings=[
                    ("fmu/out/vehicle_odometry", "/twinguard/replay/vehicle_odometry"),
                    ("fmu/in/offboard_control_mode", "/fmu/in/offboard_control_mode"),
                    ("fmu/in/trajectory_setpoint", "/fmu/in/trajectory_setpoint"),
                    ("fmu/in/vehicle_command", "/fmu/in/vehicle_command"),
                    ("trust_state", "/twinguard/trust_state"),
                    ("integrity_diagnostics", "/twinguard/integrity_diagnostics"),
                ],
            ),
        ]
    )
