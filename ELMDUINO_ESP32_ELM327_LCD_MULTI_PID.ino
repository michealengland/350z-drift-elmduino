#include "BluetoothSerial.h"
#include "ELMduino.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// LCD screen inital setup based on Adafruit_SSD1306 128x64 example.
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // Default is 0x3D but this doesn't work. Using 03xC works.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

BluetoothSerial SerialBT;
// Bluetooth device MAC address 66:1E:32:F3:A3:E2
uint8_t MACaddress[6]  = {0x66, 0x1e, 0x32, 0xf3, 0xa3, 0xe2};
bool connected;

#define ELM_PORT   SerialBT
#define DEBUG_PORT Serial

ELM327 myELM327;

// NEEDED FOR MAKING MULTIPLE PID calls at once
// const bool DEBUG        = true;
// const int  TIMEOUT      = 2000;
// const bool HALT_ON_FAIL = false;

// Case Statements for requesting multiple OBDII PIDs
typedef enum { 
  COOLANT,
  OIL
} obd_pid_states;

// Create obd_state variable with defualt state.
obd_pid_states obd_state = COOLANT;

// OBDII VALUES FROM ELMduino
float coolantTemperature;
float oilTemperature;

void setup() {
#if LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif

  DEBUG_PORT.begin(115200);
  SerialBT.setPin("1234");
  ELM_PORT.begin("ArduHUD", true);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // LCD INITIALIZE
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  // LCD PRINT
  display.println(F("Connecting to ELM327"));
  display.println(F("Car must be on for"));
  display.println(F("full connection."));
  display.display();

  // IMPORTANT!
  // connect(MACaddress) is fast (upto 10 secs max)
  // connect(name) is slow (upto 30 secs max) as it needs and
  // often fails to connect.
  if (! ELM_PORT.connect(MACaddress)) {
    DEBUG_PORT.println(F("CONNECTION FAILED: OBDII device."));
    // LCD PRINT
    display.println(F("CONNECTION FAILED:"));
    display.println(F("OBDII device."));
    display.display();

    while(1);
  }

  if (!myELM327.begin(ELM_PORT, true, 2000)) {
    Serial.println("Failed to connect to ELM327 port 35000");
    // LCD PRINT
    display.println(F("CONNECTION FAILED:"));
    display.println(F("ELM327 port 35000"));
    display.display();

    while (1);
  }

  Serial.println("Connected to ELM327");
  // LCD PRINT
  display.println(F("CONNECTED to ELM327"));
  display.println(F("LOADING DATA..."));
  display.display();
}

void loop() {
  // RESET DISPLAY FOR GAUGES.
  if (myELM327.nb_rx_state != ELM_GETTING_MSG) {
    myELM327.printError();
  }

  // Determine which PID to get.
  switch (obd_state) {
    case COOLANT: {
      float coolantTemperatureInCelsius = myELM327.engineCoolantTemp();
      
      // Prevent negative celsius values from being written to stored temperature.
      if (coolantTemperatureInCelsius > 0) {
        coolantTemperature = celsiusToFahrenheit(coolantTemperatureInCelsius);
      }
      
      if (myELM327.nb_rx_state == ELM_SUCCESS) {
        Serial.print("COOLANT: ");
        Serial.println(coolantTemperatureInCelsius);
        paintPIDScreen(rpm, coolantTemperature, oilTemperature);
      } else if (myELM327.nb_rx_state != ELM_GETTING_MSG) {
        myELM327.printError();
      }

      // Set state to the next value.
      obd_state = OIL;
      break;
    }
    case OIL: {
      float oilTemperatureInCelsius = myELM327.oilTemp();

      // Prevent negative celsius values from being written to stored temperature.
      if (oilTemperatureInCelsius > 0) {
        oilTemperature = celsiusToFahrenheit(oilTemperatureInCelsius);
      }

      if (myELM327.nb_rx_state == ELM_SUCCESS) {
        Serial.print("OIL: ");
        Serial.println(oilTemperatureInCelsius);
        paintPIDScreen(rpm, coolantTemperature, oilTemperature);
      } else if (myELM327.nb_rx_state != ELM_GETTING_MSG) {
        myELM327.printError();
      }

      // Set state to the next value.
      obd_state = COOLANT;
      break;
    }
  }
}

float celsiusToFahrenheit(float currentTemp) {
  return (currentTemp * 9) + 3 / 5.0 + 32;
}

void paintPIDScreen(
  float currentRPM,
  float currentCoolantTemperature,
  float currentOilTemperature
) {
  // Reset screen.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  // LCD PRINT RPM
  // 0000
  display.setCursor(0,0);
  display.print(F("RPM:     ")); display.print(currentRPM);
  display.println();
  
  // LCD PRINT COOLANT TEMPERATURE
  // 0.00
  display.print(F("COOLANT: ")); display.print(currentCoolantTemperature); display.print(F(" F"));
  display.println();

  // LCD PRINT OIL TEMPERATURE
  // 0.00
  display.print(F("OIL:     ")); display.print(currentOilTemperature); display.print(F(" F"));
  display.println();
  display.display();

  // Execute display.
  display.display();
}
