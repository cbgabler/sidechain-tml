// Simulated memory pressure

void memory_stress() {
    static uint8_t *buf = (uint8_t*)malloc(2000);
  
    for (int i = 0; i < 2000; i++) {
      buf[i] = (buf[i] + i) ^ 0xAA;
    }
  }