import serial
import time
import csv
import os
import numpy as np

PORT = "COM5"
BAUD = 115200
CHANNEL = 0
REPEATS = 6
SECONDS_PER_READING = 10
HZ = 10
SETTLE_SECONDS = 15
SAVE_DIR = r"C:\Users\nelly\OneDrive\Documentos\Plastech\Plastech_prototype"


def read_average(ser, seconds, hz):
    values = []
    t0 = time.time()
    while time.time() - t0 < seconds:
        line = ser.readline().decode(errors="ignore").strip()
        if line:
            try:
                values.append(float(line))
            except ValueError:
                pass
        time.sleep(1.0 / hz / 2)
    if values:
        return np.mean(values), np.std(values), len(values)
    return None, None, 0


def main():
    os.makedirs(SAVE_DIR, exist_ok=True)

    ser = serial.Serial(PORT, BAUD, timeout=1)
    time.sleep(2)
    ser.reset_input_buffer()

    data = []
    filename = os.path.join(SAVE_DIR, f"calibration_ch{CHANNEL}.csv")

    print(f"Channel {CHANNEL} - {REPEATS} reps x {SECONDS_PER_READING}s")
    print("Enter weight in grams, or 'done'.\n")

    while True:
        entry = input(">> weight (g) or 'done': ").strip()
        if entry.lower() in ("done", "d", "q"):
            break
        try:
            grams = float(entry.replace(",", "."))
        except ValueError:
            print("   invalid number")
            continue

        print(f"   settling {SETTLE_SECONDS}s...")
        ser.reset_input_buffer()
        time.sleep(SETTLE_SECONDS)

        reps = []
        for r in range(1, REPEATS + 1):
            mean, std, n = read_average(ser, SECONDS_PER_READING, HZ)
            if mean is None:
                print(f"   rep {r}: no data")
                continue
            reps.append(mean)
            print(f"   rep {r}/{REPEATS}: {mean:.1f} counts (n={n})")

        if reps:
            m = np.mean(reps)
            s = np.std(reps)
            data.append((grams, m, s))
            print(f"   {grams} g = {m:.1f} counts (+/-{s:.1f})\n")

    ser.close()

    if not data:
        print("no data recorded")
        return

    with open(filename, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["grams", "counts", "std"])
        for g, c, d in data:
            w.writerow([g, f"{c:.1f}", f"{d:.1f}"])
    print(f"\nsaved: {filename}")

    data.sort()
    g = np.array([d[0] for d in data])
    c = np.array([d[1] for d in data])
    e = np.array([d[2] for d in data])

    a, b = np.polyfit(g, c, 1)
    r2 = 1 - np.sum((c - (a * g + b)) ** 2) / np.sum((c - np.mean(c)) ** 2)

    print(f"\ncounts = {a:.3f} * grams + {b:.1f}")
    print(f"grams = (counts - {b:.1f}) / {a:.3f}")
    print(f"R2 = {r2:.5f}")

    try:
        import matplotlib.pyplot as plt
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
    except ImportError:
        print("(install matplotlib for the plot)")


if __name__ == "__main__":
    main()