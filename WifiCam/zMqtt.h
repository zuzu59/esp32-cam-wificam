//
// MQTT
//
// zf250325.1827
//
// ATTENTION, c'est pour la famille ESP32, pas pour ESP32-C3 !
//
// Sources:
// https://dawidchyrzynski.github.io/arduino-home-assistant/documents/getting-started/installation.html#arduino-ide


float sensorValue1 = 0;  // variable to store the value coming from the sensor 1
float sensorValue2 = 0;  // variable to store the value coming from the sensor 2
float sensorValue3 = 0;  // variable to store the value coming from the sensor 3
float sensorValue4 = 0;  // variable to store the value coming from the sensor 4
float sensorValue5 = 0;  // variable to store the value coming from the sensor 5
#define TEMP_CELSIUS 0


// MQTT
#include <ArduinoHA.h>
#define DEVICE_NAME      zHOST               // toujours en minuscules !
#define SENSOR_NAME1     "_Sensor1"
#define SENSOR_NAME2     "_TX_Power"
#define SENSOR_NAME3     "_RSSI"
#define SENSOR_NAME4     "_WatchCount"
#define SENSOR_NAME5     "_Sensor5"

// #define PUBLISH_INTERVAL  2000 // how often image should be published to HA (milliseconds)

HADevice device(DEVICE_NAME);                // c'est le ID du device, il doit être unique !
HAMqtt mqtt(client, device);
unsigned long lastUpdateAt = 0;

// c'est le ID du sensor, il doit être unique !
HASensorNumber Sensor1(DEVICE_NAME SENSOR_NAME1);   // , HASensorNumber::PrecisionP1);   // c'est le nom du sensor sur MQTT ! (PrecisionP1=x.1, PrecisionP2=x.01, ...)
HASensorNumber Sensor2(DEVICE_NAME SENSOR_NAME2);   // , HASensorNumber::PrecisionP1);   // c'est le nom du sensor sur MQTT ! (PrecisionP1=x.1, PrecisionP2=x.01, ...)
HASensorNumber Sensor3(DEVICE_NAME SENSOR_NAME3);   // c'est le nom du sensor sur MQTT !
HASensorNumber Sensor4(DEVICE_NAME SENSOR_NAME4);   // c'est le nom du sensor sur MQTT !
HASensorNumber Sensor5(DEVICE_NAME SENSOR_NAME5);   // , HASensorNumber::PrecisionP1);   // c'est le nom du sensor sur MQTT ! (PrecisionP1=x.1, PrecisionP2=x.01, ...)

static void ConnectMQTT() {
    device.setName(DEVICE_NAME);                // c'est le nom du device sur Home Assistant !
    mqtt.setDataPrefix(DEVICE_NAME);             // c'est le nom du device sur MQTT !
    device.setSoftwareVersion(zVERSION);
    device.setManufacturer("espressif");
    device.setModel("esp32-c3 super mini");

    Sensor1.setIcon("mdi:numeric-off");
    Sensor1.setName(SENSOR_NAME1);           // c'est le nom du sensor sur Home Assistant !
    Sensor1.setUnitOfMeasurement("");

    Sensor2.setIcon("mdi:wifi-strength-1");
    Sensor2.setName(SENSOR_NAME2);           // c'est le nom du sensor sur Home Assistant !
    Sensor2.setUnitOfMeasurement("dBm");

    Sensor3.setIcon("mdi:wifi-strength-1");
    Sensor3.setName(SENSOR_NAME3);           // c'est le nom du sensor sur Home Assistant !
    Sensor3.setUnitOfMeasurement("dBm");

    Sensor4.setIcon("mdi:counter");
    Sensor4.setName(SENSOR_NAME4);           // c'est le nom du sensor sur Home Assistant !
    Sensor4.setUnitOfMeasurement("sum");

    Sensor5.setIcon("mdi:numeric-off");
    Sensor5.setName(SENSOR_NAME5);           // c'est le nom du sensor sur Home Assistant !
    Sensor5.setUnitOfMeasurement("");

    mqtt.begin(BROKER_ADDR, BROKER_USERNAME, BROKER_PASSWORD);
    Serial.println("MQTT connected");
}

// Envoie les senseurs au mqtt
void sendSensorMqtt(){
    mqtt.loop();
    Sensor1.setValue(sensorValue1);
    Sensor2.setValue(sensorValue2);
    Sensor3.setValue(sensorValue3);
    Sensor4.setValue(sensorValue4);
    Sensor5.setValue(sensorValue5);
}

