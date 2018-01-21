#undef max
#undef min

#include <SD.h>
#include <SPI.h>
#include <WiFi101.h>
#include "secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)


#ifdef ESP8266
   #define STMPE_CS 16
   #define TFT_CS   0
   #define TFT_DC   15
   #define SD_CS    2
#endif
#ifdef ESP32
   #define STMPE_CS 32
   #define TFT_CS   15
   #define TFT_DC   33
   #define SD_CS    14
#endif
#if defined (__AVR_ATmega32U4__) || defined(ARDUINO_SAMD_FEATHER_M0) || defined (__AVR_ATmega328P__) 
   #define STMPE_CS 6
   #define TFT_CS   9
   #define TFT_DC   10
   #define SD_CS    5
#endif
#ifdef TEENSYDUINO
   #define TFT_DC   10
   #define TFT_CS   4
   #define STMPE_CS 3
   #define SD_CS    8
#endif
#ifdef ARDUINO_STM32_FEATHER
   #define TFT_DC   PB4
   #define TFT_CS   PA15
   #define STMPE_CS PC7
   #define SD_CS    PC5
#endif
#ifdef ARDUINO_NRF52_FEATHER /* BSP 0.6.5 and higher! */
   #define TFT_DC   11
   #define TFT_CS   31
   #define STMPE_CS 30
   #define SD_CS    27
#endif


//const int VBATPIN = A9;
const int VBATPIN = 9;
const int SD_CARD = 5;


//-------------------------------------------------------------------------
// Switches
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// Definitions
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// Objects
//-------------------------------------------------------------------------

//int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
IPAddress server(192,168,168,10);  // numeric IP (no DNS)
//char server[] = "www.google.com";    // name address for Google (using DNS)

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;


boolean firstPass = true;

void setup() {
 
  Serial.begin(9600);

  Serial.println( "setup()" );

    //Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8,7,4,2);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWiFiStatus();


  int time = millis();
  float voltage = readBattery();
    writeToWifi( time, voltage );


}

//--------------------------------------------------------------------------------------------
void loop() {

  if( firstPass ) {
    Serial.print( "TFT_CS: " ); Serial.println( TFT_CS );
    Serial.print( "TFT_DC: " ); Serial.println( TFT_DC );

    firstPass = false;
}
  int time = millis();
  float voltage = readBattery();




/*
  Serial.print("Battery: " ); 
  Serial.print(voltage);
  Serial.println();
*/

//  writeToSD( time, voltage );
//  writeToWifi( time, voltage );

  delay(2000);
}

//--------------------------------------------------------------------------------------------
float readBattery() {
/*
 * For the moment, the battery read input and the TFT chip select output share the same pin. 
 * Set it to an INPUT to read the battery voltage.  Then set it back to an output to
 * control the TFT.
 */
  pinMode(VBATPIN, INPUT);
  float measuredvbat = analogRead(VBATPIN);
  pinMode(VBATPIN, OUTPUT);

//  Serial.print( "Raw: " );
//  Serial.print( measuredvbat );
  float rawV = measuredvbat;
  
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage

  return measuredvbat;
}

//--------------------------------------------------------------------------------------------
void writeToSD( int time, float voltage ) {
  
  File file = SD.open( "battery.dat", FILE_WRITE );

  if( file ) {
    file.print( time );
    file.print( "," );
    file.print( voltage );
    file.println();
    file.close();
  } else {
    Serial.println( "Could not open 'battery.dat'" );
  }
}

//--------------------------------------------------------------------------------------------
void writeToWifi( int millis, float voltage ) {
  Serial.print( "WIFI: " );
  Serial.print( millis );
  Serial.print( ", " );
  Serial.print( voltage );
  Serial.println();

/*
      // Make a HTTP request:
    client.println("GET /search?q=arduino HTTP/1.1");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();
*/

  String data = "{"
      "\"deviceId\":\"19d6bcf5-6d95-466d-82f4-a91a3b0d7dc2\","
      "\"voltage\":";

    data += voltage;

    data += "}";

    
  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 8090)) {
    Serial.println("connected to server");
    // Make a HTTP request:

    Serial.print( "JSON: " );
    Serial.print( data );
    Serial.println();

    Serial.println( "-- REQUEST --------------------------------" );
    prln("POST /battery HTTP/1.1");
    prln("Host: 192.168.168.10:8090");
    prln("Content-Type: application/json");
//    prln("Connection: close");
//    prln("");

    pr("Content-Length: ");
    prln(data.length());// number of bytes in the payload
    prln("");// important need an empty line here 
    prln(data);// the payload
    
    Serial.println( "-------------------------------------------" );
    Serial.println();
/*   
    client.println("POST /battery HTTP/1.1");
    client.println("Host: 192.168.168.8:8090");
    client.println("Connection: close");
    client.println();

    client.print("Content-Length: ");
    client.println(strlen(data));// number of bytes in the payload
    client.println();// important need an empty line here 
    client.println(data);// the payload
*/
    
  } else {
    Serial.println( "Could not connect to server" );
  }

  Serial.println( "Waiting for response" );
  // if there are incoming bytes available
  // from the server, read them and print them
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  Serial.println( "Response done" );

}


//--------------------------------------------------------------------------------------------
void pr( String value ) {
  Serial.print( value );
  client.print( value );
}

//--------------------------------------------------------------------------------------------
void prln( String value ) {
  Serial.println( value );
  client.println( value );
}

//--------------------------------------------------------------------------------------------
void prln( int value ) {
  Serial.println( value );
  client.println( value );
}

//--------------------------------------------------------------------------------------------
void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

