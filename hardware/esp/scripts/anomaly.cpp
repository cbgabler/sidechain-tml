// Anomalous exection time

float anomalous_task() {
    volatile float x = 0;
  
    for (int i = 0; i < 8000; i++) {
      if (i % 13 == 0) delayMicroseconds(50); // jitter injection, non-dev behavior
      x += sin(i);
    }
  
    return x;
  }