/*
    Smart Lamp
    Author: Supachai Chaimangua (Tor)
    UpdatedDate: 2018-10-14T14:27:00+07:00
    CreatedDate: 2018-10-14T14:27:00+07:00

    Comment:

    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

String INPUT_JSON = "";
std::string END_JSON = "}";

char SSID[20];
char PASSWORD[20];
int WIFI_STATUS = WL_IDLE_STATUS;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
       digitalWrite(2, HIGH);
    };

    void onDisconnect(BLEServer* pServer) {
       digitalWrite(2, LOW);
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    DynamicJsonBuffer jsonBuffer;
    INPUT_JSON = INPUT_JSON + String(value.c_str());
    
    if (value.find(END_JSON) != std::string::npos) {
      Serial.println("=== WiFi Configruation");
      JsonObject& root = jsonBuffer.parseObject(INPUT_JSON);
      strcpy(SSID, root["ssid"].as<String>().c_str());
      strcpy(PASSWORD, root["password"].as<String>().c_str());

      const char* ssid = "TOR-WIFI";
      const char* password = "12345789";
      //WiFi.begin(SSID, PASSWORD);
      WiFi.begin(ssid, password);

      Serial.println("- SSID: " + root["ssid"].as<String>());
      Serial.println("- PASSWORD: " + root["password"].as<String>());
      int count = 0;
      while (WiFi.status() != WL_CONNECTED && count < 30) {
        delay(1000);
        count++;
        Serial.print(".");
      }
      Serial.println("");
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
      } else {
        Serial.println("WiFi connection timeout.");
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  WiFi.mode(WIFI_MODE_STA);
  BLEDevice::init("IoTBB-SmartLamp");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pServer->setCallbacks(new MyServerCallbacks());
  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("IoTBB-SmartLamp");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
}