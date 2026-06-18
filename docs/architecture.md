# Architecture

TwinGuard-Swarm-Gazebo is organized as a simulation-driven autonomy stack. The architecture separates vehicle dynamics, middleware integration, integrity estimation, supervisory control, experiment logging, and replay visualization.

## Design Goals

- Use industry-standard robotics tooling for simulation and middleware.
- Implement the latency-sensitive integrity path in C++.
- Keep autonomy logic independent of simulator-specific rendering.
- Support repeatable experiments with seeded attack scenarios.
- Generate metrics and replay videos from recorded experiment data.
- Make each module replaceable as the project moves from Gazebo/PX4 SITL toward higher-fidelity simulators.

## Layered System Model

```text
Simulation Layer
PX4 SITL + Gazebo multicopter models

Middleware Layer
ROS 2 topics, px4_msgs, px4_ros_com, Micro XRCE-DDS Agent

Autonomy Layer
C++ digital twin predictor, C++ integrity engine, C++ trust manager, formation supervisor

Experiment Layer
Attack scenarios, logging, metrics, replay generation
```

## Data Flow

1. PX4 SITL publishes vehicle state, status, GPS, and IMU streams.
2. The digital twin predicts expected per-UAV state from recent state and command history.
3. The integrity engine compares measured and predicted state.
4. The trust manager converts residual statistics into a bounded trust score.
5. The formation supervisor adjusts command authority and formation assignments.
6. The logger records state, trust, attacks, supervisor decisions, and mission metrics.
7. Replay tooling renders command-center views from recorded logs.

## Core Runtime Modules

### Digital Twin Predictor

Maintains a lightweight expected-state model for each UAV. The first implementation uses a C++ kinematic prediction model from vehicle odometry and recent setpoints. Later versions can replace this with higher-fidelity dynamics or learned prediction models.

### Integrity Engine

Computes residuals between measured state and digital-twin prediction in C++. The primary detection signal is a normalized innovation-style score that can be thresholded and smoothed over time.

### Trust Manager

Maintains stateful per-agent trust in C++. Trust decreases when residual evidence persists and recovers gradually after consistent behavior returns. The trust value is used by the supervisor rather than treated only as a visualization metric.

### Formation Supervisor

Receives per-UAV trust and mission phase, then generates formation setpoints and authority scaling. When a UAV is compromised, the supervisor isolates or deweights the agent and reassigns the remaining formation.

### Attack Injector

Introduces reproducible sensor and communication faults. Initial scenarios should include GPS spoofing, replayed state, communication dropout, and combined degradation.

### Experiment Logger

Records all state, trust, attack, and supervisor signals required for quantitative evaluation and replay generation.

## Evaluation Outputs

- Per-scenario RMSE
- Detection time
- Recovery time
- False alarm rate
- Formation error
- Mission success rate
- Per-UAV trust timeline

## Extension Path

The same autonomy layer can be connected to higher-fidelity simulation environments after the Gazebo/PX4 pipeline is stable. The key requirement is preserving the ROS 2 state, command, diagnostic, and logging interfaces defined in the topic contract.
