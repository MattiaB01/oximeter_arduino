#include <HTTPClient.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#define REPORTING_PERIOD_MS     1000

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
PulseOximeter pox;

uint32_t tsLastReport = 0;

const char* serverName = "http://192.168.1.14:8086/query";

const char* server2 ="http://192.168.1.14:8086/write?db=ESP32_Sensore";

HTTPClient http;
HTTPClient http2;


void connessione(){
  
  const char* ssid = "ssid";
  const char* password = "password";
  
 
  
  WiFi.begin(ssid, password);


  Serial.println("In attesa di connessione...");
  while (WiFi.status() != WL_CONNECTED) 
      {
       delay(500);
       Serial.println(".");
      }

}

void iniziaServer()
{
  
  http.begin(serverName);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String httpRequestData = "q=CREATE DATABASE ESP32_Sensore";    
  int httpResponseCode = http.POST(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  http.end();
}

void inviaDati(String s1,String s2)
{
      http2.begin(server2);
      http2.addHeader("Content-Type", "application/x-www-form-urlencoded");
      
      String dati="Paziente1,Sensore=Max30100 Pulsazioni="+s1+",SPO2="+s2;      
      
      
      int httpResponseCode=http2.POST(dati);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      
      http2.end();

}


// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("Dito rilevato...");
}




void setup()
{
    Serial.begin(115200);

    Serial.println("Initializing pulse oximeter..");
    connessione();
    iniziaServer();
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
    //   by uncommenting the following line. Check MAX30100_Registers.h for all the
    //   available options.
    // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);
    
}


int i;
String s1,s2;


void loop()
{
    
    // Make sure to call update as fast as possible
    
    
  
    pox.update();

    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        String battiti=String(pox.getHeartRate());
        String spo2=String( pox.getSpO2());
        Serial.print("Heart rate:");
        Serial.print(battiti);
        Serial.print("bpm / SpO2:");
        Serial.print(spo2);
        Serial.println("%");
        s1=battiti;
        s2=spo2;
        tsLastReport = millis();
        //inviaDati(battiti,spo2);
        i++;
    }
    
    if ((s2!="0")&&(i>15))
    {
    Serial.println("invio dati...");
    inviaDati(s1,s2);
    i=0;
    pox.begin();
    }
}
