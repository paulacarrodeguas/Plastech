#include <Wire.h>
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;
void setup() {
  Serial.begin(115200);
  Wire.begin();
  ads.begin(0x49);
  ads.setGain(GAIN_ONE);
}
void loop() {
  Serial.println(ads.readADC_SingleEnded(0));  // cambia el 0 por el canal
  delay(100);
}