from __future__ import annotations

from dataclasses import dataclass

import numpy as np


@dataclass
class TrustState:
    trust: float = 1.0
    residual: float = 0.0
    authority_scale: float = 1.0


class DigitalTwinPredictor:
    """Constant-velocity digital twin used before PX4-specific binding."""

    def __init__(self, dt: float = 0.02):
        self.dt = float(dt)
        self.state = np.zeros(6, dtype=float)
        self.initialized = False

    def reset(self, position: np.ndarray, velocity: np.ndarray | None = None) -> None:
        self.state[:3] = np.asarray(position, dtype=float)
        self.state[3:] = np.asarray(velocity if velocity is not None else np.zeros(3), dtype=float)
        self.initialized = True

    def predict(self) -> np.ndarray:
        if self.initialized:
            self.state[:3] += self.state[3:] * self.dt
        return self.state.copy()

    def correct_velocity(self, velocity: np.ndarray, alpha: float = 0.2) -> None:
        self.state[3:] = (1.0 - alpha) * self.state[3:] + alpha * np.asarray(velocity, dtype=float)


class TrustManager:
    def __init__(self, alpha: float = 1.2, beta: float = 0.90, min_authority: float = 0.15):
        self.alpha = float(alpha)
        self.beta = float(beta)
        self.min_authority = float(min_authority)
        self.state = TrustState()

    def update(self, measured_position: np.ndarray, predicted_position: np.ndarray) -> TrustState:
        residual = float(np.linalg.norm(np.asarray(measured_position) - np.asarray(predicted_position)))
        integrity = float(np.exp(-self.alpha * residual))
        trust = float(np.clip(self.beta * self.state.trust + (1.0 - self.beta) * integrity, 0.0, 1.0))
        authority = float(np.clip(trust, self.min_authority, 1.0))
        self.state = TrustState(trust=trust, residual=residual, authority_scale=authority)
        return self.state
