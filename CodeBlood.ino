#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100_PulseOximeter.h"

/******************* MAX 30100 Section ***********************/
int bpm = 0;
int spo2 = 0;

void max_30100_init();

#define REPORTING_PERIOD_MS 1000
 
// PulseOximeter is the higher level interface to the sensor
// it offers:
// * beat detection reporting
// * heart rate calculation
// * SpO2 (oxidation level) calculation
PulseOximeter pox;
 
uint32_t tsLastReport = 0;
/****************************************************************/

/******************* Oled Section ***********************/
void oled_init();
void update_oled();

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, -1);

/****************************************************************/

void setup()
{
  Serial.begin(115200);
  max_30100_init();
  oled_init();
}
 
void loop()
{
  // Make sure to call update as fast as possible
  pox.update();
  
  // Asynchronously dump heart rate and oxidation levels to the serial
  // For both, a value of 0 means "invalid"
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
  Serial.print("Heart rate:");
  bpm = pox.getHeartRate();
  Serial.print(bpm);
  Serial.print("bpm / SpO2:");
  spo2 = pox.getSpO2();
  Serial.print(spo2);
  Serial.println("%");
  
  tsLastReport = millis();
  }

  update_oled();
}

void update_oled()
{
  if (bpm == 0 || spo2==0)
    return;
    
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("bpm = ");

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(40, 0);
  display.print(bpm);
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.print("SpO2 = ");

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(40, 10);
  display.print(spo2);
  
  display.display();
}

void oled_init()
{
  Wire1.begin(33, 32); // SDA = GPIO33, SCL = GPIO32
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x64
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.setTextSize(1);
  display.println("Code Blood!");
  display.display();
}

void max_30100_init()
{
  Serial.print("Initializing pulse oximeter..");
  
  // Initialize the PulseOximeter instance
  // Failures are generally due to an improper I2C wiring, missing power supply
  // or wrong target chip
  if (!pox.begin()) {
  Serial.println("FAILED");
  for(;;);
  } else {
  Serial.println("SUCCESS");
  }
  
  // The default current for the IR LED is 50mA and it could be changed
  // by uncommenting the following line. Check MAX30100_Registers.h for all the
  // available options.
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
}
