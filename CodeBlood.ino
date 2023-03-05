#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

/******************* MAX 30100 Section ***********************/

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

void setup()
{
  Serial.begin(115200);
  max_30100_init();
}
 
void loop()
{
  // Make sure to call update as fast as possible
  pox.update();
  
  // Asynchronously dump heart rate and oxidation levels to the serial
  // For both, a value of 0 means "invalid"
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
  Serial.print("Heart rate:");
  Serial.print(pox.getHeartRate());
  Serial.print("bpm / SpO2:");
  Serial.print(pox.getSpO2());
  Serial.println("%");
  
  tsLastReport = millis();
  }
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
