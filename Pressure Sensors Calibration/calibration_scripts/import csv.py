import csv
import os
import numpy as np
import matplotlib.pyplot as plt

CHANNEL = 3
SAVE_DIR = r"C:\Users\nelly\OneDrive\Documentos\Plastech\Plastech_prototype"

filename = os.path.join(SAVE_DIR, f"calibration_ch{CHANNEL}.csv")

data = []
with open(filename, newline="") as f:
    reader = csv.reader(f)
    next(reader)  # skip header
    for row in reader:
        if len(row) >= 3:
            data.append((float(row[0]), float(row[1]), float(row[2])))

data.sort()
g = np.array([d[0] for d in data])
c = np.array([d[1] for d in data])
e = np.array([d[2] for d in data])

a, b = np.polyfit(g, c, 1)
r2 = 1 - np.sum((c - (a * g + b)) ** 2) / np.sum((c - np.mean(c)) ** 2)

print(f"counts = {a:.3f} * grams + {b:.1f}")
print(f"grams = (counts - {b:.1f}) / {a:.3f}")
print(f"R2 = {r2:.5f}")

plt.figure(figsize=(8, 5.5))
plt.errorbar(g, c, yerr=e, fmt="o", capsize=4, markersize=7, label="data")
xf = np.linspace(g.min(), g.max(), 100)
plt.plot(xf, a * xf + b, "-", label=f"y = {a:.1f}x + {b:.0f}\nR2 = {r2:.4f}")
plt.xlabel("Force (g)")
plt.ylabel("ADC counts")
plt.title(f"Calibration channel {CHANNEL} - FlexiForce A201-1")
plt.legend()
plt.grid(alpha=0.3)
plt.tight_layout()
png = os.path.join(SAVE_DIR, f"calibration_ch{CHANNEL}.png")
plt.savefig(png, dpi=150)
print(f"saved: {png}")
plt.show()