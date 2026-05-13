#include <Arduino.h>

#include <scripts/anomaly.h>
#include <scripts/baseline.h>
#include <scripts/memory.h>

void setup() {
    Serial.begin(115200);


}

void loop() {
    Serial.print("Time: ");
    uint32_t time = micros();
    baseline_task();
    uint32_t elapsed = micros() - time;

    Serial.println(time);

    uint32_t heap = esp_get_free_heap_size();
    
    Serial.printf("T:%u H:%u R:%d\n", elapsed, heap, reset_count);

    delay(100);
}