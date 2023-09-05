#include <Arduino.h>
#include <bleHandClient.h>

bleHandClient::bleHandClient() {}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Connected");
      deviceConnected = true;
      
    };
    void onDisconnect(BLEServer* pServer) {
      Serial.println("Disconnected Client");
      deviceConnected = false;
      BLEDevice::startAdvertising();
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      Serial.println(value.c_str());
      strcpy(state, value.c_str());
      check = true;
    }
};

void bleHandClient::begin() {
    BLEDevice::init("HAND");

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

    Serial.println(frameI);
}

void bleHandClient::sendData(String _data) {
    temp = _data;
    dataFrame->setValue(temp.c_str());
    dataFrame->notify();
}

void bleHandClient::sendData(uint8_t* pData, size_t length) {
    dataFrame->setValue(pData, length);
    dataFrame->notify();
}

String bleHandClient::getControl() {
    return state;
}

void bleHandClient::setControl(String _data) {
    // state = _data;
    strcpy(state, _data.c_str());
}

void bleHandClient::setCheck(bool _data) {
    check = _data;
}

bool bleHandClient::getCheck() {
    return check;
}