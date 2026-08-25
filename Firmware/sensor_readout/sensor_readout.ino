/*
 * PLASTECH - basic sensor readout
 *
 * Reads all the sensors on the Plastech splint node and prints the values
 * to the Serial Monitor. Pressure is converted to grams using a per-sensor
 * calibration (each FSR has its own slope and intercept, they are not
 * interchangeable). 
 * Open the Serial Monitor at 115200 baud to see the values.
 *
 * I2C addresses:
 *   0x44  humidity/temperature (SHT45)
 *   0x48  proximal temperature (TMP117)
 *   0x49  distal temperature (TMP117)
 *   0x4A  pressure ADC (ADS1115)
 */

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_TMP117.h>
#include <Adafruit_SHT4x.h>

Adafruit_ADS1115 ads;
Adafruit_TMP117 tmp_prox;
Adafruit_TMP117 tmp_dist;
Adafruit_SHT4x sht;

float slope[4]     = {58.4, 50.8, 49.8, 54.6};
float intercept[4] = {4268, 4275, 4645, 4263};

float toGrams(int16_t counts, int ch) {
  float g = (counts - intercept[ch]) / slope[ch];
  if (g < 0) g = 0;
  return g;
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Wire.begin();

  ads.begin(0x4A);
  ads.setGain(GAIN_ONE);
  tmp_prox.begin(0x48);
  tmp_dist.begin(0x49);
  sht.begin();
}

void loop() {
  float p[4];
  for (int i = 0; i < 4; i++) p[i] = toGrams(ads.readADC_SingleEnded(i), i);

  sensors_event_t ep, ed;
  tmp_prox.getEvent(&ep);
  tmp_dist.getEvent(&ed);
  float gradient = ep.temperature - ed.temperature;

  sensors_event_t hum, temp;
  sht.getEvent(&hum, &temp);

  Serial.print("P1: "); Serial.print(p[0], 1); Serial.print(" g   ");
  Serial.print("P2: "); Serial.print(p[1], 1); Serial.print(" g   ");
  Serial.print("P3: "); Serial.print(p[2], 1); Serial.print(" g   ");
  Serial.print("P4: "); Serial.print(p[3], 1); Serial.print(" g   ");
  Serial.print("T_prox: "); Serial.print(ep.temperature, 2); Serial.print(" C   ");
  Serial.print("T_dist: "); Serial.print(ed.temperature, 2); Serial.print(" C   ");
  Serial.print("gradient: "); Serial.print(gradient, 2); Serial.print(" C   ");
  Serial.print("T_skin: "); Serial.print(temp.temperature, 2); Serial.print(" C   ");
  Serial.print("RH: "); Serial.print(hum.relative_humidity, 1); Serial.println(" %");

  delay(500);
}