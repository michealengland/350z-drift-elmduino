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

float celsiusToFahrenheit(float currentTemp) {
  return ((currentTemp * 9) + 3) / 5.0 + 32;
}

// VALUES IN CELSIUS
float const COOLANT_COLD_TEMP = celsiusToFahrenheit(75);  // 167.0f - A little below operating temp.
float const COOLANT_WARM_TEMP = celsiusToFahrenheit(88);  // 190.4f - Ideal operating around 85c/185f.
float const COOLANT_HOT_TEMP  = celsiusToFahrenheit(94);  // 201.2f - Just a good place to key an eye on.
float const COOLANT_MAX_TEMP  = celsiusToFahrenheit(99);  // 210.2f - At 220 degress the car is way too hot!

float const OIL_COLD_TEMP     = celsiusToFahrenheit(75);  // 167.0f - A little below operating temp.
float const OIL_WARM_TEMP     = celsiusToFahrenheit(95);  // 203.0f - Ideal operating around 85c/185f.
float const OIL_HOT_TEMP      = celsiusToFahrenheit(110); // 230.0f - Oil is hot at this point, keep an eye out on it.
float const OIL_MAX_TEMP      = celsiusToFahrenheit(120); // 248.0f - At 120c the oil is dangerously hot! 

float const INTAKE_COLD_TEMP  = celsiusToFahrenheit(30);  // 86.0f - The car is cool.
float const INTAKE_WARM_TEMP  = celsiusToFahrenheit(48);  // 118.4f - Guesstimate of casual driving.
float const INTAKE_HOT_TEMP   = celsiusToFahrenheit(55);  // 131.0f - Just a good place to key an eye on.
float const INTAKE_MAX_TEMP   = celsiusToFahrenheit(60);  // 140.0f - The car engine bay is really hot.

ELM327 myELM327;

// Case Statements for requesting multiple OBDII PIDs
typedef enum { 
  COOLANT,
  OIL,
  INTAKE_AIR
} obd_pid_states;

// Create obd_state variable with defualt state.
obd_pid_states obd_state = COOLANT;

// OBDII VALUES FROM ELMduino
float coolantTemperature;
float oilTemperature;
float intakeTemperature;

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
  if (myELM327.nb_rx_state != ELM_GETTING_MSG) {
    myELM327.printError();
  }

  // Determine which PID to get.
  switch (obd_state) {
    case COOLANT: {
      float coolantTemperatureInCelsius = myELM327.engineCoolantTemp();
      if(coolantTemperatureInCelsius > 0) {
        coolantTemperature = celsiusToFahrenheit(coolantTemperatureInCelsius);
      }
      
      if (myELM327.nb_rx_state == ELM_SUCCESS) {
        Serial.print("COOLANT: ");
        Serial.println(coolantTemperatureInCelsius);
        paintPIDScreen();
      } else if (myELM327.nb_rx_state != ELM_GETTING_MSG) {
        myELM327.printError();
      }

      // Set state to the next value.
      obd_state = OIL;
      break;
    }
    case OIL: {
      float oilTemperatureInCelsius = myELM327.oilTemp();
      if (oilTemperatureInCelsius > 0) {
        oilTemperature = celsiusToFahrenheit(oilTemperatureInCelsius);
      }

      if (myELM327.nb_rx_state == ELM_SUCCESS) {
        Serial.print("OIL: ");
        Serial.print(oilTemperatureInCelsius);
        paintPIDScreen();
      } else if (myELM327.nb_rx_state != ELM_GETTING_MSG) {
        myELM327.printError();
      }

      // Set state to the next value.
      obd_state = INTAKE_AIR;
      break;
    }
    case INTAKE_AIR: {
      float intakeTemperatureInCelsius = myELM327.intakeAirTemp();
      if (intakeTemperatureInCelsius > 0) {
        intakeTemperature = celsiusToFahrenheit(intakeTemperatureInCelsius);
      }

      if (myELM327.nb_rx_state == ELM_SUCCESS) {
        Serial.print("INTAKE TEMP: ");
        Serial.println(intakeTemperatureInCelsius);
        paintPIDScreen();
      } else if (myELM327.nb_rx_state != ELM_GETTING_MSG) {
        myELM327.printError();
      }

      obd_state = COOLANT;
      break;
    }
  }
}

void paintPIDScreen() {
  // Reset screen.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  // LCD PRINT COOLANT TEMPERATURE
  display.print(F("COOLNT: "));
  display.print(coolantTemperature);

  if (coolantTemperature >= COOLANT_WARM_TEMP) {
    if (coolantTemperature < COOLANT_HOT_TEMP) {
      display.print(F("F WARM"));
    } else if (coolantTemperature < COOLANT_MAX_TEMP) {
      display.print(F("F HOT"));
    } else {
      display.print(F("F DNGR"));
    }
  } else if (coolantTemperature <= COOLANT_COLD_TEMP) {
    display.print(F("F COLD"));
  } else {
    display.print(F("F"));
  }
  display.println();

  // LCD PRINT OIL TEMPERATURE
  display.print(F("OIL:    "));
  display.print(oilTemperature);

  if (oilTemperature >= OIL_WARM_TEMP) {
    if (oilTemperature < OIL_HOT_TEMP) {
      display.print(F("F WARM"));
    } else if (oilTemperature < OIL_MAX_TEMP) {
      display.print(F("F HOT"));
    } else {
      display.print(F("F DNGR!"));
    }
  } else if (oilTemperature <= OIL_COLD_TEMP) {
    display.print(F("F COLD"));
  } else {
    display.print(F("F"));
  }
  display.println();

  // LCD PRINT INTAKE AIR TEMPERATURE
  display.print(F("INTAKE: "));
  display.print(intakeTemperature);

  display.print(F(" F"));
  if (intakeTemperature >= INTAKE_WARM_TEMP) {
    if (intakeTemperature < INTAKE_HOT_TEMP) {
      display.print(F("F WARM"));
    } else if (intakeTemperature < INTAKE_MAX_TEMP) {
      display.print(F("F HOT"));
    } else {
      display.print(F("F DNGR!"));
    }
  } else if (intakeTemperature <= INTAKE_COLD_TEMP) {
    display.print(F("F COLD"));
  } else {
    display.print(F("F"));
  }
  display.println();

  // EXECUTE LCD DISPLAY
  display.display();
}