#ifndef  BLE_HAND_CLIENT_H 
#define  BLE_HAND_CLIENT_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "aa5e8956-f569-11ed-a05b-0242ac120003"
#define DATA_UUID           "a4a623ca-f569-11ed-a05b-0242ac120003"

static char state [10] = "STOP";
static char frameI[10] = "HAND";
static char frameII[2]= ":";

static bool deviceConnected = false;
static bool oldDeviceConnected = false;

static bool check;

class bleHandClient
{
    public:
        bleHandClient();
        void begin();
        // void st art();

        void sendData(String _data);
        void sendData(uint8_t* pData, size_t length);

        String getControl();
        void setControl(String _data);

        void setCheck(bool _data);
        bool getCheck();
    private:
        BLEServer *pServer = NULL;
        BLECharacteristic *dataFrame = NULL;
        BLECharacteristic *stateControl = NULL;
        uint32_t lastTime = millis();
        uint32_t stopTimer = millis();

        uint16_t count = 0;
        uint16_t timeDelay = 10;
        String temp = "";
        bool stateNotify = false;
        bool notifyDisconnected = true;

        
};

#endif