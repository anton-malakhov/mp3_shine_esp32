## mp3_shine_esp32: Shine MP3 encoder component for ESP32

This is a reviving, tuning, and packaging by Anton Malakhov in 2023 of the Shine MP3 encoder originally written for ARM or MIPS arch devices, quite a long time ago and ported to old ESP-IDF by fkn in 2019.

Memory allocation has been optimised for the ESP32 family of microcontrollers. You should init the encoder ASAP in your code as the encoder needs large contiguous chunks of RAM.

## Performance

Use the example to evaluate your particular hardware with your specific settings. The performance was tested on the supplied example with 48K/16bit, 2 channels input, 256 bit rate output on the following hardware:
* ESP32-S3: Single core takes 53.4% of real time
* ESP32-S2 Mini: Takes 56.9% of real time
* ESP32 (WT32-ETH01): Single core takes 71.6%

## Limitations

The encoding algorithm is rather simple. In particular, it does not have any Psychoacoustic Model.

## A bit of history

This code was dug out from the dusty crates of those times before internet and github. It apparently was created by Gabriel Bouvigne sometime around the end of the 20th century. The encoder was converted circa 2001 by Pete Everett to fixed-point arithmetic for the RISC OS. Latest we know, Patrick Roberts had worked on the code to make it multi-platform and more library oriented. That was around 2006. Later the ports were found at:
* https://github.com/toots/shine
* https://github.com/fknrdcls/mp3_shine_esp32
