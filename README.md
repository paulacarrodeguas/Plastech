# Plastech

Plastech is a 100% biodegradable, cellulose-based splint meant to replace
plaster and fiberglass casts. It has two layers: a flexible film that wraps
the limb and a filler that hardens inside it. 

My main work focused on the materials (preparing methylcellulose
formulations with different additives and characterizing them with rheology).
One part of the project was also a sensing system for the skin–splint
interface.

## Why sensors

Once a limb is in a cast, what happens against the skin is no longer
visible, so problems build up without warning. The sensing system measures
the parameters behind the most common complications:

- **Pressure** over bony prominences, where sustained pressure leads to
  pressure sores. Interface pressure also correlates with intracompartmental
  pressure, so it is relevant to compartment syndrome (Uslu & Apan, 2000).
- **Temperature gradient** between a proximal and a distal point, proposed
  as an early non-invasive sign of compartment syndrome (Tepordei et al.,
  2024).
- **Humidity** at the skin, since a damp interface causes maceration.

The FlexiForce A201-1 has been used for cast pressure monitoring before
(Tuan et al., 2019), which supports its use here.

## Sensor placement

Four pressure sensors, balancing the main bony prominences (pressure sores)
with the compartments that matter for compartment syndrome:

- **P1 — radial styloid** (bony prominence)
- **P2 — ulnar styloid** (bony prominence)
- **P3 — volar / flexor compartment** (compartment syndrome)
- **P4 — dorsal / extensor compartment** (compartment syndrome)

The two temperature sensors are placed as far apart as the forearm splint
allows, one near the elbow and one near the wrist, to capture the
proximal–distal gradient. Tepordei et al. measured this across the whole
limb; here the same principle is applied at forearm scale, with the sensors
as far apart as possible. The humidity sensor sits at the skin.

## Calibration

Each pressure sensor was calibrated individually against known weights
(0–300 g), six readings per weight, 10 s at 10 Hz. Each FSR has its own
slope and intercept. So pressure in grams comes from a per-sensor linear 
fit (R² = 0.98–0.997). The script and the equations are in `calibration/`.

## Code

`firmware/` contains two sketches:

- **plastech_sensor_node** — The ESP32 creates its own WiFi
  network and serves a small dashboard. A mobile app would be better suited for a final
  product, but a web page was enough to show that the sensors read correctly.
- **plastech_basic_readout** — prints the readings to the
  Serial Monitor (115200 baud). No WiFi, useful to check everything works, could be used in a future to send the values so an app processes them.

To use the dashboard: connect to the **PLASTECH** network (password is in
the code) and open `http://192.168.4.1/`. `/data` returns the raw line and
`/status` shows which sensors started.

## Hardware

- 4× FlexiForce A201-1 (pressure)
- MCP6004 op-amp (amplifies and linearizes the signal)
- ADS1115 16-bit ADC
- ESP32-S3 Feather
- 2× TMP117 (temperature, ±0.1 °C)
- SHT45 (humidity, ±1.0% RH)
- Li-Ion 3.7 V battery

I²C: 0x44 SHT45, 0x48 TMP117 proximal, 0x49 TMP117 distal, 0x4A ADS1115.
Schematic is in `hardware/`.

## Folders

```
firmware/     Arduino sketches
calibration/  calibration scripts, equations, plots
hardware/     schematic, block diagram
rheology/     rheology data and plots
photos/       prototype and sample photos
```

## References

- Tuan, C.-C. et al. (2019). *Sensors* 19(10), 2417.
  https://doi.org/10.3390/s19102417
- Uslu, M. M. & Apan, A. (2000). *Arch. Orthop. Trauma Surg.*
  https://doi.org/10.1007/s004020050472
- Tepordei, R.-T. et al. (2024). *J. Pers. Med.* 14(5), 477.
  https://doi.org/10.3390/jpm14050477
- Tekscan. FlexiForce Sensors — Best Practices in Electrical Integration.
  https://www.tekscan.com/resources/datasheets-guides/best-practices-electrical-integration-flexiforce-sensor

