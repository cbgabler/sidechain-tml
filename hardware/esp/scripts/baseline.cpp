// Baseline execution time

float baseline_task() {
    volatile float x = 0;
  
    for (int i = 0; i < 5000; i++) {
      x += i * 0.001f;
    }
  
    return x;
  }