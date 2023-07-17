# 350z Elmduino Project
This is a simple Arduino project based that relies on the [ELMduino library](https://github.com/PowerBroker2/ELMduino/tree/master) to help diagnose information on my 350z on the track. This should work for any car that supports OBDII.

## Things you'll need
1. [ESP32 Board](https://www.amazon.com/gp/product/B07WCG1PLV/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) - I got this one, but there are other options out there. You just need bluetooth.
2. [ELM 327 Bluetooth Module](https://www.amazon.com/dp/B09B2Q7WJS?psc=1&ref=ppx_yo2ov_dt_b_product_details) - It was cheap and seems to work well. Connects very quickly.
3. [128x64 LCD](https://www.amazon.com/HiLetgo-Serial-128X64-Display-Color/dp/B06XRBTBTB/ref=sr_1_5?crid=IBGQ13ZIZ2JU&keywords=128x64+lcd&qid=1689625209&sprefix=128x64+lcd%2Caps%2C196&sr=8-5) - You don't have to have an LCD for this to work but it's very helfpul for displaying the data. This [article](https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/) helped me get the LCD up and running on the ESP32. The LCD Pintout, I used is as follows:

```
// LCD hookup.
LCD | Board
SCK | D21 (SCL on board)
SDA | D22 (SDA on board)
GND | GND
VDD | 3.3v
```

## Getting Started
1. Clone the repo
2. Select your ESP32 board and install necessary libraries
3. Create a file called `arduino_secrets.h` in the root directory using the template below.
4. Update the values for your MacAddress and pass code if needed.

```
#define SECRET_SSID "OBD2"; // Default.
#define SECRET_PASS "1234"; // Default.

// Replace this with your own.
// Bluetooth device MAC address AA:BB:CC:11:22:33
#define SECRET_MAC_ADRESS {0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33};
```

## Finding the ELM327 Mac Address:
1. Plugin the ELM327 unit and connect to it your computers bluetooth. It should show up as "OBDII".
2. Enter the password if it requires one. The default passcode for ELM327 bluetooth model is usually `1234`. If that doesn't work, try `12345` or `0000`.
3. For mac users, you can find the MAC address using the command `system_profiler SPBluetoothDataType` in Terminal.
4. You should be able to spot by looking for OBDII.
5. Once you have the MAC address, go to your bluetooth settings and forget "OBDII".
6. Next, update the MAC address in the `arduino_secrets` file. Each pair of values will have an `0x` as seen below.
```
// AA BB CC 11 22 33
#define SECRET_MAC_ADRESS {0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33};
```

## Troubleshooting
- Due to challenges connecting this to ELM327 wifi consistently I recommend bluetooth or a wired connection. For more details see [ELMduino Wifi Issue]](https://github.com/PowerBroker2/ELMduino/issues/52).
- Make sure the car is started with OBDII ELM327 inserted into the port before starting the ESP32.
- If it doesn't connect the first time, try restarting the ESP32.
- Using the `ELM_PORT.connect(ssid)` as seen in the ELMduino ESP32 example often failed to connect and was very slow. I used the solution documented [here](https://github.com/espressif/arduino-esp32/blob/b92c58d74b151c7a3b56db4e78f2d3c90c16446f/libraries/BluetoothSerial/examples/SerialToSerialBTM/SerialToSerialBTM.ino#L27-L31) to resolve this.
- Querying certain data together such as RPM and temperatures was a little slow. I didn't really need RPM anymore, so I removed it from my example.