#include <Arduino.h>

#include "scripts/anomaly.cpp"
#include "scripts/baseline.cpp"
#include "scripts/memory.cpp"

RTC_DATA_ATTR int reset_count = 0; 

void setup() {
    Serial.begin(115200);

    reset_count++;
}

void loop() {
    uint32_t time = micros();
    baseline_task();
    uint32_t elapsed = micros() - time;

    uint32_t heap = esp_get_free_heap_size();
    
    Serial.printf("T:%u H:%u R:%d\n", elapsed, heap, reset_count);

    delay(100);
}