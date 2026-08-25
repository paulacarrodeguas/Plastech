/*
 * PLASTECH - interface monitor
 * -----------------------------
 * Firmware for the Plastech splint sensor node.
 *
 * The ESP32 reads the sensors and serves the values over its own WiFi
 * network. Connect to the network and open the page in a browser — no
 * app required. A mobile app would be more suitable for a final product,
 * but a web page is enough to verify that the sensors read correctly.
 *
 * Sensors:
 *   - 4 pressure channels (FSR) read through the ADS1115 and converted
 *     to grams. Each sensor has its own calibration (slope + intercept)
 *     and they are not interchangeable. The goal is to alert the risk of pressure sores and compartment syndrome when the interface pressure is too high.
 *   - proximal and distal temperature (2x TMP117) and their difference to prevent compartment syndrome
 *   - temperature and humidity at the skin interface (SHT45) to avoid skin problems like maceration.
 *
 * Usage:
 *   Connect to the "PLASTECH" WiFi network (password below) and open:
 *     http://192.168.4.1/          dashboard
 *     http://192.168.4.1/data      raw values: P1,P2,P3,P4,gradient,T_micro,RH
 *     http://192.168.4.1/status    sensor status + temperatures for debugging
 *
 * I2C addresses:
 *   0x44  humidity/temperature (SHT45)
 *   0x48  proximal temperature (TMP117)
 *   0x49  distal temperature (TMP117)
 *   0x4A  pressure ADC (ADS1115)
 */

#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_TMP117.h>
#include <Adafruit_SHT4x.h>

const char* AP_SSID = "PLASTECH";
const char* AP_PASS = "plastech2026";

WebServer server(80);

Adafruit_ADS1115 ads;
Adafruit_TMP117 tmp_prox;
Adafruit_TMP117 tmp_dist;
Adafruit_SHT4x sht;

bool ads_ok = false, prox_ok = false, dist_ok = false, sht_ok = false;
float last_prox = -1, last_dist = -1;

float slope[4]     = {58.4, 50.8, 49.8, 54.6};
float intercept[4] = {4268, 4275, 4645, 4263};

float toGrams(int16_t counts, int ch) {
  float g = (counts - intercept[ch]) / slope[ch];
  if (g < 0) g = 0;
  return g;
}

// P1,P2,P3,P4,gradient,T_micro,RH
String buildLine() {
  float p[4] = {0, 0, 0, 0};
  if (ads_ok) for (int i = 0; i < 4; i++) p[i] = toGrams(ads.readADC_SingleEnded(i), i);

  float gradient = 0;
  if (prox_ok) { sensors_event_t e; tmp_prox.getEvent(&e); last_prox = e.temperature; }
  if (dist_ok) { sensors_event_t e; tmp_dist.getEvent(&e); last_dist = e.temperature; }
  if (prox_ok && dist_ok) gradient = last_prox - last_dist;

  float t_micro = -1, rh = -1;
  if (sht_ok) {
    sensors_event_t hum, temp;
    sht.getEvent(&hum, &temp);
    t_micro = temp.temperature;
    rh = hum.relative_humidity;
  }

  String s = "";
  s += String(p[0],1) + "," + String(p[1],1) + "," + String(p[2],1) + "," + String(p[3],1) + ",";
  s += String(gradient,2) + ",";
  s += String(t_micro,2) + "," + String(rh,1);
  return s;
}

// ------------------------------------------------------------------
// dashboard
// ------------------------------------------------------------------
const char PAGE[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Plastech</title>
<style>
:root{
  --bg:#0B1416; --panel:#122024; --edge:#1E3238;
  --ink:#E4EFEC; --dim:#7E9A9B;
  --ok:#8FD6BA; --watch:#E8B65A; --alert:#E8685A;
  --mono:ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;
  --sans:system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;
}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--ink);font-family:var(--sans);
     -webkit-text-size-adjust:100%}
.bar{display:flex;align-items:center;gap:10px;padding:14px 18px;
     border-bottom:1px solid var(--edge);font-size:12px;letter-spacing:.14em;
     text-transform:uppercase;color:var(--dim)}
.bar b{color:var(--ink);font-weight:600;letter-spacing:.14em}
.live{margin-left:auto;display:flex;align-items:center;gap:7px;letter-spacing:.08em}
#pulse{width:7px;height:7px;border-radius:50%;background:var(--ok)}
.stale #pulse{background:var(--alert)}
.wrap{display:grid;gap:18px;padding:18px;max-width:900px;margin:0 auto}
@media(min-width:760px){.wrap{grid-template-columns:300px 1fr;align-items:start}}

.limb{background:var(--panel);border:1px solid var(--edge);border-radius:14px;padding:8px}
.limb svg{width:100%;height:auto;display:block}
.pt{transition:r .25s ease,fill .25s ease}
@media(prefers-reduced-motion:reduce){.pt{transition:none}}
.ptlabel{font-family:var(--mono);font-size:11px;fill:var(--ink)}
.ptname{font-family:var(--mono);font-size:8px;fill:var(--dim)}

.rows{display:grid;gap:12px}
.row{background:var(--panel);border:1px solid var(--edge);border-radius:14px;
     padding:16px 18px;display:flex;align-items:baseline;gap:14px}
.row .k{font-size:11px;letter-spacing:.12em;text-transform:uppercase;color:var(--dim)}
.row .v{margin-left:auto;font-family:var(--mono);font-size:34px;
        font-variant-numeric:tabular-nums;line-height:1}
.row .u{font-family:var(--mono);font-size:13px;color:var(--dim)}
.row.big .v{font-size:46px}
.ok .v{color:var(--ok)} .watch .v{color:var(--watch)} .alert .v{color:var(--alert)}
</style>
</head>
<body>

<div class="bar" id="bar">
  <b>Plastech</b>
  <span class="live"><span id="pulse"></span><span id="age">--</span></span>
</div>

<div class="wrap">

  <div class="limb">
    <svg viewBox="0 0 200 400">
      <path d="M78 24 C68 120 60 260 58 372 L142 372 C140 260 132 120 122 24 Z"
            fill="#16282D" stroke="#24424A" stroke-width="1.5"/>

      <circle class="pt" id="c0" cx="82"  cy="112" r="8" fill="#8FD6BA" opacity=".85"/>
      <circle class="pt" id="c1" cx="120" cy="112" r="8" fill="#8FD6BA" opacity=".85"/>
      <circle class="pt" id="c2" cx="76"  cy="264" r="8" fill="#8FD6BA" opacity=".85"/>
      <circle class="pt" id="c3" cx="126" cy="264" r="8" fill="#8FD6BA" opacity=".85"/>

      <text class="ptname" x="40"  y="98">P1</text>
      <text class="ptname" x="150" y="98">P2</text>
      <text class="ptname" x="34"  y="250">P3</text>
      <text class="ptname" x="156" y="250">P4</text>
      <text class="ptlabel" id="t0" x="40"  y="118" text-anchor="end">--</text>
      <text class="ptlabel" id="t1" x="160" y="118">--</text>
      <text class="ptlabel" id="t2" x="34"  y="270" text-anchor="end">--</text>
      <text class="ptlabel" id="t3" x="166" y="270">--</text>
    </svg>
  </div>

  <div class="rows">
    <div class="row big ok" id="r_grad">
      <span class="k">Temperature gradient</span>
      <span class="v" id="v_grad">--</span><span class="u">&deg;C</span>
    </div>
    <div class="row ok" id="r_temp">
      <span class="k">Temperature</span>
      <span class="v" id="v_temp">--</span><span class="u">&deg;C</span>
    </div>
    <div class="row ok" id="r_rh">
      <span class="k">Humidity</span>
      <span class="v" id="v_rh">--</span><span class="u">%</span>
    </div>
  </div>

</div>

<script>
const TH = {
  p:    {watch:150, alert:250},
  grad: {watch:1.0, alert:2.0},
  rh:   {watch:70,  alert:80}
};
const P_FULL = 300;

const COL = {ok:'#8FD6BA', watch:'#E8B65A', alert:'#E8685A'};
const level = (x,t) => x >= t.alert ? 'alert' : x >= t.watch ? 'watch' : 'ok';
const setRow = (el,lv) => { el.className = 'row ' + (el.id==='r_grad'?'big ':'') + lv; };

let lastOk = 0;

async function tick(){
  try{
    const r = await fetch('/data',{cache:'no-store'});
    const v = (await r.text()).trim().split(',').map(Number);
    if(v.length !== 7 || v.some(n => Number.isNaN(n))) throw new Error('bad line');

    for(let i=0;i<4;i++){
      const g = v[i], lv = level(g, TH.p);
      const c = document.getElementById('c'+i);
      c.setAttribute('r', 8 + 16*Math.min(1, g/P_FULL));
      c.setAttribute('fill', COL[lv]);
      document.getElementById('t'+i).textContent = g.toFixed(0);
    }

    const grad = v[4];
    v_grad.textContent = (grad>0?'+':'') + grad.toFixed(2);
    setRow(r_grad, level(Math.abs(grad), TH.grad));

    v_temp.textContent = v[5].toFixed(1);
    v_rh.textContent   = v[6].toFixed(0);
    setRow(r_rh, level(v[6], TH.rh));

    lastOk = Date.now();
    bar.classList.remove('stale');
  }catch(e){
    if(Date.now() - lastOk > 3000) bar.classList.add('stale');
  }
  age.textContent = lastOk ? Math.round((Date.now()-lastOk)/1000)+'s' : 'no signal';
}
tick();
setInterval(tick, 500);
</script>
</body>
</html>)HTML";

void handleRoot() {
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html", String(FPSTR(PAGE)));
}

void handleData() {
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/plain", buildLine());
}

void handleStatus() {
  String live = buildLine();
  String s = "Plastech sensor node\n\n";
  s += "ADC:           "; s += ads_ok  ? "OK" : "NOT FOUND"; s += "\n";
  s += "Temp proximal: "; s += prox_ok ? "OK" : "NOT FOUND"; s += "\n";
  s += "Temp distal:   "; s += dist_ok ? "OK" : "NOT FOUND"; s += "\n";
  s += "Microclimate:  "; s += sht_ok  ? "OK" : "NOT FOUND"; s += "\n\n";
  s += "Order: P1,P2,P3,P4,gradient,T_micro,RH\n";
  s += "Live:  " + live + "\n\n";
  s += "Temps (debug): " + String(last_prox,2) + " / " + String(last_dist,2) + "\n";
  s += "-1 means that sensor did not start.\n";
  server.send(200, "text/plain", s);
}

void scanI2C() {
  Serial.println("--- I2C scan ---");
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("  device at 0x"); Serial.println(addr, HEX);
      found++;
    }
  }
  if (found == 0) Serial.println("  nothing found - check wiring");
  Serial.println("----------------");
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.println();
  Serial.print("Hotspot up: "); Serial.println(AP_SSID);
  Serial.print("Open http://"); Serial.print(WiFi.softAPIP()); Serial.println("/");

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/status", handleStatus);
  server.begin();
  Serial.println("Server started");

  Wire.begin();
  Wire.setTimeOut(50);
  scanI2C();

  ads_ok = ads.begin(0x4A);
  if (ads_ok) ads.setGain(GAIN_ONE);
  Serial.print("ADC: ");            Serial.println(ads_ok  ? "OK" : "FAIL");

  prox_ok = tmp_prox.begin(0x48);
  Serial.print("Temp proximal: ");  Serial.println(prox_ok ? "OK" : "FAIL");

  dist_ok = tmp_dist.begin(0x49);
  Serial.print("Temp distal: ");    Serial.println(dist_ok ? "OK" : "FAIL");

  sht_ok = sht.begin();
  Serial.print("Microclimate: ");   Serial.println(sht_ok  ? "OK" : "FAIL");

  Serial.println("Setup complete");
}

void loop() {
  server.handleClient();
}
