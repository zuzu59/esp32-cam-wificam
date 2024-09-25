// Petite caméra de surveillance à base de mini caméra ESP32-cam
//
// ATTENTION, ce code a été testé sur une Ai Thinker ESP32-CAM. Pas testé sur les autres boards !
// Initial commit zf231111
//
#define zVERSION        "zf240923.1406"
// Il faut aussi modifier 'zWifiVersion' dans handlers.cpp !
//#define zHOST           "esp-cam-st-luc1"        // ATTENTION, tout en minuscule
#define zHOST           "esp-cam-crissier1"        // ATTENTION, tout en minuscule
// #define zIpStatic
// IPAddress zLocal_IP(192, 168, 57, 61);
// IPAddress zGateway(192, 168, 57, 1);

/*
Utilisation:

Astuce:

Installation:

  Pour les esp32-cam, il faut:
  Board: Ai Thinker ESP32-CAM
  Flash Frequency: 80MHz
  Flash Mode: QIO
  Partition Scheme: Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)
  Core Debug Level: None

Comme cet ESP32-CAM ne possède pas de connecteur USB avec convertisseur TTL il faut en ajouter un externe pour le premier flash, après on peut s'en passer en flashant via OTA
Et ponter le IO0 avec le GND pour se mettre en mode flashing !

ATTENTION à l'ordre de chargement des librairies !

Pour le WiFiManager, il faut installer cette lib depuis le lib manager sur Arduino:
https://github.com/tzapu/WiFiManager


Sources:
https://makeradvisor.com/esp32-cam-ov2640-camera/
https://fr.aliexpress.com/item/1005001322358029.html
https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/
https://github.com/SeeedDocument/forum_doc/blob/master/reg/ESP32_CAM_V1.6.pdf
https://github.com/yoursunny/esp32cam/tree/main/examples/WifiCam
https://github.com/wjsanek/wjsanek

*/



// #define DEBUG true
// #undef DEBUG



// General
const int ledPin = 33;             // the number of the LED pin
//const int buttonPin = 9;          // the number of the pushbutton pin
#define zDSLEEP         0                       // 0 ou 1 !
#include "secrets.h"
const long zIntervalzSendTelemetrieMqtt =   10000;             // Interval en mili secondes pour l'envoi de la télémétrie au MQTT
unsigned long zPrevious_MilliszSendTelemetrieMqtt = 0;       // Compteur de temps pour l'envoi de la télémétrie au MQTT


// // Source: https://randomnerdtutorials.com/esp32-static-fixed-ip-address-arduino-ide/
// // Set your Static IP address
// IPAddress local_IP(192, 168, 1, 61);
// // Set your Gateway IP address
// IPAddress gateway(192, 168, 1, 1);

// IPAddress subnet(255, 255, 255, 0);
// IPAddress primaryDNS(8, 8, 8, 8);   //optional
// IPAddress secondaryDNS(8, 8, 4, 4); //optional


// ESP32-cam
#include "WifiCam.hpp"
WebServer server(80);

// Utilisé seulement pour la configuration de la ESP32-cam
#include "esp_camera.h"
// Camera pinout
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


// Sonar Pulse
#include "zSonarpulse.h"


// WIFI
#include "zWifi.h"
// #define lowTxPower  true   // diminution de la puissance à cause de la réflexion de l'antenne sur le HTU21D directement soudé sur le esp32-c3 super mini zf240725.1800


// OTA Arduino IDE
#include <ArduinoOTA.h>
#define HOSTNAME zHOST


// OTA WEB server
#include "otaWebServer.h"


// MQTT
#include "zMqtt.h"


void setup(){
  // Pulse deux fois pour dire que l'on démarre
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); delay(zSonarPulseOn); digitalWrite(ledPin, HIGH); delay(zSonarPulseOff);
  digitalWrite(ledPin, LOW); delay(zSonarPulseOn); digitalWrite(ledPin, HIGH); delay(zSonarPulseOff);
  delay(zSonarPulseWait);

  // Start serial console
  Serial.begin(19200);
  Serial.setDebugOutput(true);       //pour voir les messages de debug des libs sur la console série !
  delay(3000);                          //le temps de passer sur la Serial Monitor ;-)
  Serial.println("\n\n\n\n**************************************\nCa commence !"); Serial.println(zHOST ", " zVERSION);

  // Start WIFI
  zStartWifi();
  //sensorValue3 = WiFi.RSSI();

  // Start OTA Arduino IDE
  ota_setup();

  // Start OTA WEB server
  otaWebServer();

  // Connexion au MQTT
  Serial.println("\n\nConnect MQTT !\n");
  ConnectMQTT();

  // go go go
  Serial.println("\nC'est parti !\n");

  Serial.println("\nOn fait un power reset de la caméra !\n");

  pinMode(PWDN_GPIO_NUM, OUTPUT);

  // Power off the ESP32-CAM
  digitalWrite(PWDN_GPIO_NUM, LOW);
  delay(2000); // Wait for 2 seconds

  // Power on the ESP32-CAM
  digitalWrite(PWDN_GPIO_NUM, HIGH);
  delay(2000); // Wait for 2 seconds



  // Configuration de la caméra
  Serial.println("camera init...");
  camera_init();

  // Start de la caméra
  addRequestHandlers();
  server.begin();
}


void loop() {
    // WEB camera server loop
    server.handleClient();
    // OTA Arduino IDE loop
    ArduinoOTA.handle();
    // OTA WEB loop
    serverOTA.handleClient();
    // Check for WIFI
    zWifi_Check_Connection();
    // Send telemetrie to MQTT
    zSendTelemetrieMqtt();
    // Un petit coup de sonar pulse sur la LED pour dire que tout fonctionne bien
    sonarPulse();
}


// Send telemetrie to MQTT
void zSendTelemetrieMqtt(){
  unsigned long currentMillis = millis();
  if(currentMillis - zPrevious_MilliszSendTelemetrieMqtt >= zIntervalzSendTelemetrieMqtt || zPrevious_MilliszSendTelemetrieMqtt > currentMillis){
    zPrevious_MilliszSendTelemetrieMqtt = currentMillis;
    Serial.println("\nzSendTelemetrieMqtt !");

    //Increment watchCount
    ++watchCount;
    sensorValue4 = watchCount;
    Serial.println("watchCount: " + String(watchCount));

    sensorValue2 = WiFi.getTxPower();
    sensorValue3 = WiFi.RSSI();

    sendSensorMqtt();
    Serial.print("TX_Power:");
    Serial.println(sensorValue2);
    Serial.print("RSSI:");
    Serial.println(sensorValue3);
    Serial.print("WatchCount:");
    Serial.println(sensorValue4);
  }
}


