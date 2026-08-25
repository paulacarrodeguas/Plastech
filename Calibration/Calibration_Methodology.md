# Calibration equations — FlexiForce A201-1

Four pressure channels, each calibrated individually.

`counts` = raw ADS1115 reading (GAIN_ONE). 
`grams`  = force on the sensor.

| Channel | counts = a·g + b        | grams = (counts − b) / a       | R²     | Valid range |
|---------|-------------------------|--------------------------------|--------|-------------|
| 0       | counts = 58.4·g + 4268  | grams = (counts − 4268) / 58.4 | 0.9968 | 0–300 g     |
| 1       | counts = 50.8·g + 4275  | grams = (counts − 4275) / 50.8 | 0.9957 | 0–300 g     |
| 2       | counts = 49.8·g + 4645  | grams = (counts − 4645) / 49.8 | 0.9834 | 0–300 g     |
| 3       | counts = 54.6·g + 4263  | grams = (counts − 4571) / 34.2 | 0.9967 | 0–300 g     |

## Conditions

- Rigid puck over the ~9.53 mm sensing area
- Weights: 0, 5, 10, 20, 50, 100, 200, 300 g (made on the lab balance)
- 6 repetitions per weight, averaged; fixed settling time per weight to keep
  drift consistent across points

## Protocol / method references

- Weight set and 6×10 s averaging protocol: Kao et al., Sensors 2021,
  21(17):5681, doi:10.3390/s21175681
- FSR conditioning before calibration: Tekscan FlexiForce Best Practices;
  Hall, Desmoulin & Milner, J Biomech 2008, doi:10.1016/j.jbiomech.2008.09.031
- Per-sensor calibration requirement and part-to-part variation: Tekscan
  FlexiForce Best Practices
