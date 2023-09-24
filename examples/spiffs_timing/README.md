## Minimalistic Shine MP3 encoding example for ESP32
This example answers two questions:
1. How fast the mp3_shine_esp32 is.
2. What is the quality of resulting mp3 stream.

## Input
The input data for the encoding is stored in the raw PCM16 format in `spiffs/input.raw` file. This file can be replaced by your own raw wav file content (without header). The example uses spiffs file system, which is built and flushed along with the regular compilation and flushing process.

## Output
The `output.mp3` can be received by running `receive_mp3_serial.py`. It requires `python3` and `pyserial` for functioning.
