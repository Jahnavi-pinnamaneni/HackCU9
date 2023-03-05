#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100_PulseOximeter.h"
#include <WiFi.h>
#include <HTTPClient.h>

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

/******************************WiFi Section**********************/
const char* ssid = "Samsung";
const char* password = "12345678";

void call();
void post(int p1, int p2);
void led();
void wifi_init();
void update_cloud();
/****************************************************************/
void setup()
{
  Serial.begin(115200);
  max_30100_init();
  oled_init();
  wifi_init();
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
  update_cloud();
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

void call() {

    const char* serverName = "https://api.thingspeak.com/apps/thinghttp/send_request?api_key=ZQTEEL0S4RIUK90S";

    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
    
      // Your Domain name with URL path or IP address with path
      http.begin(serverName);

      // api_key=(thinghttp_api_key)&number={phone_number_to_call}
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");      
      // Send HTTP POST request
      int httpResponseCode = http.GET();
      
      /*
      // If you need an HTTP request with a content type: application/json, use the following:
      http.addHeader("Content-Type", "application/json");
      // JSON data to send with HTTP POST
      String httpRequestData = "{\"api_key\":\"" + apiKey + "\",\"field1\":\"" + String(random(40)) + "\"}";           
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);*/
     
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }  
}

void post(int p1, int p2){
  const char* serverName = "https://api.thingspeak.com/update?api_key=CV4RCEVWHA6GEWQ2";
  if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
    
      // Your Domain name with URL path or IP address with path
      http.begin(serverName);
      
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "&field1=" +String(p1) +"&field2=" + String(p2);           
      // Send HTTP POST request00
      int httpResponseCode = http.POST(httpRequestData);
      
      /*
      // If you need an HTTP request with a content type: application/json, use the following:
      http.addHeader("Content-Type", "application/json");
      // JSON data to send with HTTP POST
      String httpRequestData = "{\"api_key\":\"" + apiKey + "\",\"field1\":\"" + String(random(40)) + "\"}";           
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);*/
     
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
      Serial.println("Exiting post");
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}

void led(){
    
  if(WiFi.status()== WL_CONNECTED)
    digitalWrite(2, HIGH);
  else
    digitalWrite(2, LOW);
}
void wifi_init()
{
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Initializing pulse oximeter..");
  // Initialize sensor
  if (!pox.begin()) {
      Serial.println("FAILED");
      for(;;);
  } else {
      Serial.println("SUCCESS");
  }
}

void update_cloud()
{
  if(bpm == 0 || spo2 == 0)
    return;
  post(bpm, spo2);
  //delay(2000);
  
  if(spo2 > 94 && spo2 < 100)
  {
    call();
    delay(10000);
    exit(1);
  }
  bpm = 0;
  spo2 = 0;

  Serial.println("Exiting update_cloud");
}