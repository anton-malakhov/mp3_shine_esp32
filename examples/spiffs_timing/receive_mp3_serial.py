#!/usr/bin/env python3
import serial
import serial.tools.list_ports
from time import sleep

try:
    ser = None
    # Find and open the COM port
    ports = serial.tools.list_ports.comports()
    port = next((p.device for p in ports), None)
    if port is None:
        raise ValueError("No COM port found.")

    high = False
    low = True

    ser = serial.Serial(port, baudrate=115200, bytesize=8, parity='N', stopbits=1, xonxoff=False, rtscts=False, dsrdtr=False)
    ser.setDTR(high)     # Set dtr to reset state (affected by rts)
    ser.setRTS(low)      # Set rts/dtr to the reset state
    # ser.setDTR(ser.dtr)   # usbser.sys workaround
    ser.reset_input_buffer()
    sleep(0.2)
    ser.setRTS(high)     # Set rts/dtr to the working state
    print("Serial connection established with", port)

    while True:
        # Read a line of data from the serial port
        line = ser.readline().decode().strip()

        if line:
            print('>', line)

        if line.startswith("Ready to send mp3 stream"):
            print("Requesting mp3 stream from the device, press BOOT key if using USB CDC.")
            ser.setDTR(low)
            ser.write(b">")

        if line.startswith("Sending "):
            sz = int(line.split(' ')[1])
            break

    print("Receiving, size =", sz)
    eos = "Sent {} bytes\r\n".format(sz).encode()
    mp3buf = ser.read_until(eos, sz*2).replace(eos, b'').replace(b"\r\n", b"\n")
    assert(len(mp3buf) == sz)

    with open("output.mp3", "wb") as file:
        file.write(mp3buf)
    print("MP3 stream has been written to the file")

except ValueError as ve:
    print("Error:", str(ve))

except serial.SerialException as se:
    print("Serial port error:", str(se))

except Exception as e:
    print("An error occurred:", str(e))

finally:
    # Close the serial connection
    if not ser is None and ser.is_open:
        ser.close()
        print("Serial connection closed.")
