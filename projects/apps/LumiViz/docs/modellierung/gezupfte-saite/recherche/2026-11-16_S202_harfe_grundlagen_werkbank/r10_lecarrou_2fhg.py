"""R10: Le Carrou 2006 Thèse Gl. (4.19)-(4.21), q = (z5, w34): Eigenwerte (gegen Tab. 4.2), Guete, Punktmasse an w34,
Mobilitaet |w34/F| (gegen Abb. 4.4 A: Spitzen um 4-6e-3 m/s/N)."""
import numpy as np
M = np.array([[0.54, -5.02], [-5.02, 49.22]])
B = np.array([[5.37, -19.67], [-19.67, 205.31]])
K = np.array([[590.41, -5366.46], [-5366.46, 51310.00]]) * 1e3
# Zustandsform: s^2 M + s B + K = 0
A = np.block([[np.zeros((2, 2)), np.eye(2)], [-np.linalg.solve(M, K), -np.linalg.solve(M, B)]])
s = np.linalg.eigvals(A)
s = sorted([x for x in s if x.imag > 0], key=lambda x: x.imag)
for x in s:
    print(f"s = {x.real:.1f} + j {x.imag:.1f}  ->  f = {abs(x)/(2*np.pi):.1f} Hz, zeta = {-x.real/abs(x)*100:.2f} %, Q = {abs(x)/(-2*x.real):.1f}")
print("Tab. 4.2: s4 = -31,9 + j 976,6; s6 = -32,5 + j 1065,9")
Minv = np.linalg.inv(M)
print(f"det M = {np.linalg.det(M):.3f} kg^2; Masse, die eine Kraft an w34 sieht (quasistatisch): 1/(M^-1)_22 = {1/Minv[1,1]:.2f} kg")
print("|w34/F| [1e-3 m/s/N]:")
for f in (130, 140, 150, 155, 160, 165, 170, 175, 180, 190):
    w = 2 * np.pi * f
    H = np.linalg.inv(-w * w * M + 1j * w * B + K)
    print(f"  {f} Hz: {abs(1j*w*H[1,1])*1e3:.2f}   |V5/F|-artig |z5'/F| = {abs(1j*w*H[0,1])*1e2:.2f} e-2")
