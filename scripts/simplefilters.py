import numpy as np
from numpy.linalg import inv

class KalmanFilter:
    def setup(self, dt,
                 A, C, Q, R, P):
        self.A = A
        self.C = C
        self.Q = Q
        self.R = R
        self.P0 = P
        self.m = C.shape[0]
        self.n = A.shape[0]
        self.dt = dt
        self.initialized = False
        self.I = np.identity(self.n)
        self.x_hat = np.zeros((self.n, 1))
        self.x_hat_new = np.zeros((self.n, 1))

    def __init__(self):
        dt = 1.0 / 10
        A = np.array([[1, dt, 0], [0, 1, dt], [0, 0, 1]])
        C = np.array([[1, 0, 0]])
        Q = np.array([[0.01, 0.01, 0], [0.01, 0.01, 0], [0, 0, 0]])
        R = np.array([[100, 0, 0], [0, 100, 0], [0, 0, 100]])
        P = np.array([[0.1, 0.1, 0.1], [0.1, 10000, 00], [0.1, 10, 100]])
        self.setup(dt, A, C, Q, R, P)

    def init(self, t0, x0):
        self.x_hat = x0
        self.P = self.P0
        self.t0 = t0
        self.t = t0
        self.initialized = True

    def filter(self, y):
        if not self.initialized:
            self.init(0, np.zeros((self.n, 1)))

        self.x_hat_new = self.A * self.x_hat
        self.P = self.A * self.P * self.A.transpose() + self.Q
        K = self.P * self.C.transpose() * inv(self.C * self.P * self.C.transpose() + self.R)
        self.x_hat_new = self.x_hat_new + K * (y - self.C * self.x_hat_new)
        self.P = (self.I - K * self.C) * self.P
        self.t = self.t + self.dt

        self.x_hat = self.x_hat_new
        return self.value()

    def value(self):
        return self.x_hat[0][0]