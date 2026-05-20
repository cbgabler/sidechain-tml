// ESP32 victim — UART ping-pong responder for GlassBox bring-up.
// Wiring (per glassbox/README.md):
//   ESP32 GPIO16 (RX2) <- Pico GP0 (TX)
//   ESP32 GPIO17 (TX2) -> Pico GP1 (RX)
//   ESP32 GPIO5        <- Pico GP2  (trigger INPUT, Pico drives)
//   ESP32 GND          <-> Pico GND

static const int PIN_TRIGGER_IN = 5;

void setup() {
  Serial.begin(115200);                                    // USB debug
  Serial2.begin(115200, SERIAL_8N1, 16, 17);               // (baud, cfg, rxPin, txPin)
  pinMode(PIN_TRIGGER_IN, INPUT);                          // Pico drives the trigger

  delay(200);
  Serial.println();
  Serial.println("[esp32] booted, UART2 @ 115200 on GPIO16(RX)/GPIO17(TX)");
}

static uint32_t last_heartbeat_ms = 0;
static uint32_t pings_seen = 0;

void loop() {
  // Heartbeat to USB so we know the ESP32 is alive even if no PING arrives.
  uint32_t now = millis();
  if (now - last_heartbeat_ms >= 1000) {
    last_heartbeat_ms = now;
    Serial.printf("[esp32] alive  ms=%lu  pings_seen=%lu  trigger=%d\n",
                  (unsigned long)now, (unsigned long)pings_seen,
                  digitalRead(PIN_TRIGGER_IN));
  }

  if (Serial2.available()) {
    String s = Serial2.readStringUntil('\n');
    s.trim();                                              // drop \r and trailing whitespace
    if (s.length() == 0) return;

    pings_seen++;
    Serial2.printf("PONG %s\n", s.c_str());                // reply to Pico
    Serial.printf("[esp32] <- %s   -> PONG %s\n", s.c_str(), s.c_str());
  }
}
