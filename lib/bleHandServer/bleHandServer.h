#ifndef BLE_HAND_SERVER_H
#define BLE_HAND_SERVER_H

#include "BLEDevice.h"
//#include "BLEScan.h"

#include "bleHandClient.h"

#define TIME_SCAN   5

static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID    charUUID("445f2ca8-f548-11ed-a05b-0242ac120003");

static bool doConnect = false;
static bool connected = false;
static bool stateScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
static uint32_t lastTimeScanBLE = millis();
static String dataEMG;

class bleHandServer
{
    public:
        bleHandServer(bleHandClient& _handClient);
        void begin();
        void start(String _data);
        void scanBle();

        void setDataEMG(String _data);
        notify_callback notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
        bool connectToServer();
        // void setDataEMG(String _data);
        String getdataEMG();
    private:
        bleHandClient& handClient; 
        
    
};

#endif
