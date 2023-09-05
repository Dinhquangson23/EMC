#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define DATA_UUID           "445f2ca8-f548-11ed-a05b-0242ac120003"

TaskHandle_t send_data;

char state [6];
char frame1[12] = "EMG:";

String temp;
BLEServer* pServer = NULL;
BLECharacteristic* dataFrame = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint16_t timeDelay = 100;
uint16_t count = 0;
uint32_t lastTime = millis();
uint32_t stopTimer = millis();

void connecting();
void disconnect();
void sendData();
void checkTimer();

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Connected");
      strcpy(state, "STOP");
      deviceConnected = true;
      
    };
    void onDisconnect(BLEServer* pServer) {
      Serial.println("Disconnected");
      Serial.println(count);
      count = 0;
      deviceConnected = false;
      BLEDevice::startAdvertising();
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      strcpy(state, value.c_str());
    }
};

void task_Send_Data(void* Parameter) {
  for (;;) {
    if (deviceConnected) {
      if (strcmp(state, "START") == 0) {
        sendData();
        checkTimer();
      }else if (strcmp(state, "STOP") == 0) {
        // Serial.println("STOPPED");
        count = 0;
      }
    }
    vTaskDelay(1);
  }
}

void setup() {
  Serial.begin(115200);

  BLEDevice::init("EMG");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  dataFrame = pService->createCharacteristic(DATA_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
  dataFrame->setCallbacks(new MyCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
  
  Serial.println(frame1);
  xTaskCreatePinnedToCore(task_Send_Data, "task_sendata", 10000, NULL, 1, &send_data, 0);
}

void loop() {
  vTaskDelete(NULL);  
}

void sendData() {
  if (millis() - lastTime > timeDelay) {
    temp = frame1 + String(++count);
    dataFrame->setValue(temp.c_str());
    dataFrame->notify();
    Serial.println(temp);
    // Serial.println(count);
    lastTime = millis();
  }
}

void checkTimer() {
  static uint8_t countTimer = 0;
  if (countTimer < 20) {
    if (millis() - stopTimer > 30000) {
      countTimer++;
      stopTimer = millis();
    }
  }else {
    Serial.println("STOP");
    dataFrame->setValue("STOP");
    dataFrame->notify();
    strcpy(state, "STOP");
    countTimer = 0;
    count = 0;
    delay(100);
  }
}