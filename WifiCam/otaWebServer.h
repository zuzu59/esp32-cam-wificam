//
// OTA WEB server
//
// zf240923.0809
//
// Sources:
// https://lastminuteengineers.com/esp32-ota-web-updater-arduino-ide/

#include <WebServer.h>
#include <Update.h>


/*
 * Login page
 */
#include "otaLoginIndex.h"


/*
 * Server Index Page
 */
#include "otaServerIndex.h"


WebServer serverOTA(8080);

static void otaWebServer() {
  /*return index page which is stored in serverIndex */
  serverOTA.on("/", HTTP_GET, []() {
    serverOTA.sendHeader("Connection", "close");
    serverOTA.send(200, "text/html", loginIndex);
  });
  serverOTA.on("/serverIndex", HTTP_GET, []() {
    serverOTA.sendHeader("Connection", "close");
    serverOTA.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  serverOTA.on("/update", HTTP_POST, []() {
    serverOTA.sendHeader("Connection", "close");
    serverOTA.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    
    // Serial.println("On arrête la caméra !");
    // esp_camera_deinit();
    // delay(1000);

    HTTPUpload& upload = serverOTA.upload();

    // Serial.print("upload.status: ");
    // Serial.println(upload.status);
    // delay(1000);

    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Serial.println("error UPDATE_SIZE_UNKNOWN");
        Update.printError(Serial);
        delay(1000);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Serial.println("error UPLOAD_FILE_WRITE");
        Update.printError(Serial);
        delay(1000);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        // Serial.println("error UPLOAD_FILE_END1");
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        delay(1000);
      } else {
        Serial.println("error UPLOAD_FILE_END2");
        Update.printError(Serial);
        delay(1000);
      }
    }
  });
  serverOTA.begin();
}
