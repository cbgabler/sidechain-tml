Project to develop a Tiny ML model on a chip to sucessfully stop in progress sidechain attacks/remote code execution.

## Scope
### Hardware
This project is a sideband hardware security monitor. The Pico does **not share memory, OS, or a bus** with the ESP32. A compromised ESP32 for mock testing cannot tamper with the detector. 

### Models
I am using a random forest (hopefully switching to a 1D-CNN or LSTM) trained on the temporal sequences of the hardware telemetry.

## TODO
1. ESP32 harness (harness.ino) — emit structured telemetry: T:<us> H:<bytes> R:<resets>\n over UART on each loop iteration. This is the blocker right now.
2. Pico harness (raspberry/harness.ino) — window buffering, feature extraction, model inference, and the EN-pin kill switch.
