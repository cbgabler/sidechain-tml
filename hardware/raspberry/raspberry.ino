#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("READY");
}

void loop() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command.startsWith("BRIDGE")) {
            int seconds = command.substring(7).toInt();
            Serial.printf("got BRIDGE for %d seconds\n", seconds);
        } else {
            Serial.println("ERR unknown command");
        }
    }
}
