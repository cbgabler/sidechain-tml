// Pico monitor — UART ping-pong sanity test against the ESP32 victim.
// Wiring (per glassbox/README.md):
//   Pico GP0 (UART0 TX) -> ESP32 GPIO16 (RX2)
//   Pico GP1 (UART0 RX) <- ESP32 GPIO17 (TX2)
//   Pico GP2            -> ESP32 GPIO5  (trigger, Pico drives)
//   Pico GND            <-> ESP32 GND   (REQUIRED)

static const uint8_t PIN_TRIGGER = 2;

void setup() {
  Serial.begin(115200);                  // USB to laptop
  Serial1.setTX(0);                      // GP0 = UART0 TX
  Serial1.setRX(1);                      // GP1 = UART0 RX
  Serial1.begin(115200);                 // to ESP32

  pinMode(PIN_TRIGGER, OUTPUT);          // we drive the trigger (per docs)
  digitalWrite(PIN_TRIGGER, LOW);

  uint32_t t0 = millis();
  while (!Serial && (millis() - t0) < 3000) { delay(10); }

  Serial.println();
  Serial.println("[pico] booted, UART1 @ 115200 on GP0/GP1");
}

static int counter = 0;

void loop() {
  // 1. Pulse the trigger so a scope/the ESP32 can see "function start".
  digitalWrite(PIN_TRIGGER, HIGH);
  delayMicroseconds(50);

  // 2. Send the PING.
  Serial1.printf("PING %d\n", counter);
  Serial.printf("[pico] -> PING %d\n", counter);

  // 3. Wait up to 500 ms for a reply terminated by '\n'.
  String reply;
  reply.reserve(64);
  uint32_t deadline = millis() + 500;
  while (millis() < deadline) {
    while (Serial1.available()) {
      char c = Serial1.read();
      if (c == '\r') continue;           // tolerate CRLF
      if (c == '\n') { deadline = 0; break; }
      reply += c;
      if (reply.length() > 60) { deadline = 0; break; }   // safety cap
    }
  }

  digitalWrite(PIN_TRIGGER, LOW);

  if (reply.length() > 0) {
    Serial.printf("[pico] <- %s\n", reply.c_str());
  } else {
    Serial.println("[pico] no reply within 500 ms");
  }

  counter++;
  delay(1000);
}
