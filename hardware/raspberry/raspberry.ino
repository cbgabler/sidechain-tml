#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("READY");
}

void loop() {
    if (Serial.available() > 0){
        String command = Serial.readStringUntil('\n');
        command = command.trim();
    }

    if (command.startsWith("BRIDGE")) {
        int val = command(7).toInt;
        Serial.println("Got BRIDGE for %c seconds", val)
    } else {
        Serial.println("Error: Unknown command");
    }
}
