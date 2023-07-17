# 350z Elmduino Project
This is a simple project to help me be aware of possible issues while running my 350z on the track.

## Things you'll need
1. [ESP32 Board](https://www.amazon.com/gp/product/B07WCG1PLV/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) - I got this one, but there are other options out there. You just need bluetooth.
2. [ELM 327 Bluetooth Module](https://www.amazon.com/dp/B09B2Q7WJS?psc=1&ref=ppx_yo2ov_dt_b_product_details) - It was cheap and seems to work well. Connects very quickly.
3. [128x64 LCD] - You don't have to have an LCD for this to work but it's very helfpul for displaying the data. The LCD Pintout, I used is as follows:

This [article](https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/) helped me get the LCD up and running on the ESP32. 

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
3. Update secret variables as needed.

## Troubleshooting
- Due to challenges connecting this to ELM327 wifi consistently I recommend bluetooth or a wired connection. For more details see [ELMduino Wifi Issue]](https://github.com/PowerBroker2/ELMduino/issues/52).
- Make sure the car is started with OBDII ELM327 inserted into the port before starting the ESP32.
- If it doesn't connect the first time, try restarting the ESP32.

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