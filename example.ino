// This is a basic example to call RPM only.
// Try this as index.ino
// Use Serial Monitor with 115200 baudrate set in the Arduino IDE. This should match line 41.

#include "BluetoothSerial.h"
#include "ELMduino.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "arduino_secrets.h"

// Make sure to update the arduino_secrets.h file with the values for your ELM327.
// char SSID[] = SECRET_SSID; // Network name is not used since we're using mac address.
char PASS[] = SECRET_PASS; // Bluetooth passcode
uint8_t MAC_ADDRESS[6] = SECRET_MAC_ADRESS; // Bluetooth Mac Address

// LCD screen inital setup based on Adafruit_SSD1306 128x64 example.
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // Default is 0x3D but this doesn't work. Using 03xC works.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

BluetoothSerial SerialBT;
bool connected;

#define ELM_PORT   SerialBT
#define DEBUG_PORT Serial

// Initial value for RPM.
uint32_t rpm = 0;
ELM327 myELM327;

void setup() {
#if LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif

  DEBUG_PORT.begin(115200);
  SerialBT.setPin(PASS);
  ELM_PORT.begin("ArduHUD", true);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // O
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  display.setTextSize(5);
  display.println(F("350z"));
  display.setTextSize(1);
  display.println(F("Elmduino"));
  display.println(F("By Mike England"));
  display.println(F("v0.0.1"));
  display.display();
  delay(3000);

  // LCD INITIALIZE
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  // LCD PRINT
  display.println(F("Connecting to ELM327"));
  display.println(F("Car must be on for"));
  display.println(F("full connection."));
  display.println();
  display.display();

  // IMPORTANT!
  // connect(MACaddress) is fast (upto 10 secs max)
  // connect(name) is slow (upto 30 secs max) as it needs and
  // often fails to connect.
  if (! ELM_PORT.connect(MAC_ADDRESS)) {
    DEBUG_PORT.println(F("CONNECTION FAILED: OBDII device."));
    // LCD PRINT
    display.println(F("CONNECTION FAILED:"));
    display.println(F("OBDII device."));
    display.println();
    display.display();

    while(1);
  }

  if (!myELM327.begin(ELM_PORT, true, 2000)) {
    Serial.println("Failed to connect to ELM327 port 35000");
    // LCD PRINT
    display.println(F("CONNECTION FAILED:"));
    display.println(F("ELM327 port 35000"));
    display.println();
    display.display();

    while (1);
  }

  Serial.println("Connected to ELM327");
  // LCD PRINT
  display.println(F("CONNECTED to ELM327"));
  display.println(F("LOADING DATA..."));
  display.println();
  display.display();
}

void loop() {
  float tempRPM = myELM327.rpm();

  if (myELM327.nb_rx_state == ELM_SUCCESS)
  {
    rpm = (uint32_t)tempRPM;
    Serial.print("RPM: "); Serial.println(rpm);
      display.println(F("RPM:"));
      display.println(F(rpm));
      display.display();
  }
  else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    myELM327.printError();
      display.println(F("ERROR READING RPM"));
      display.display();
}
