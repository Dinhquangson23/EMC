#include <Arduino.h>
#include <bleHandClient.h>

#define TIME_SCAN   5

static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID    charUUID("445f2ca8-f548-11ed-a05b-0242ac120003");

static bool doConnect = false;
static bool connected = false;
static bool stateScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
static uint32_t lastTimeScanBLE = millis();

static char* t;
void scanBle();
void startHandServer();

TaskHandle_t ble_client;
TaskHandle_t ble_server;

bleHandClient bleClient;

static void notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    Serial.print("data: ");
    Serial.println((char*)pData);
    bleClient.sendData(pData, length);
    
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
    stateScan = false;
    BLEDevice::getScan()->stop();
    Serial.println("Connected");
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

static bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (strcmp(advertisedDevice.getName().c_str(), "") != 0) {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());
      Serial.println("--------------");
    }
    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks

void Task_BLE_Server(void* Parameter) {
  for (;;) {
    startHandServer();
    vTaskDelay(1);
  }
}

void setup() {
    Serial.begin(115200);
    bleClient.begin();
    Serial.println("Starting Arduino BLE Client application...");
    BLEDevice::init("");
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(TIME_SCAN, true);
    stateScan = true;
    lastTimeScanBLE = millis();

    xTaskCreatePinnedToCore(Task_BLE_Server, "Task_BLE_SERVER_EMG", 10000, NULL, 1, &ble_server, 0);

}

void loop() {
    vTaskDelete(NULL);    
}


void scanBle() {
    Serial.println("Rescan Ble");
    BLEDevice::getScan()->clearResults();
    BLEDevice::getScan()->start(TIME_SCAN, true);
    lastTimeScanBLE = millis();
}

void startHandServer() {
    if (doConnect == true) {
      if (connectToServer()) {
        Serial.println(connected);
        Serial.println("Connected to the BLE Server.");
      } else {
        Serial.println("We have failed to connect to the server; there is nothin more we will do.");
      }
      doConnect = false;
    }

    if (connected == false) {
      if (stateScan == false) {
        scanBle();
        stateScan = true;
      }
    }else {
      if (bleClient.getCheck()) {
        pRemoteCharacteristic->writeValue(bleClient.getControl().c_str(), bleClient.getControl().length());
        bleClient.setCheck(false);
      }
      
    }

    if (stateScan == true) {
      if (millis() - lastTimeScanBLE > TIME_SCAN * 1000) {
        scanBle();
      }
    }
}