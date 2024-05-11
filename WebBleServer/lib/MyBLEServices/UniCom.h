#pragma once

#include <MyBLEServer.h>
#include <ArduinoJson.h>

using std::vector;

// Universal communication service
#define UNI_SERVICE_UUID        "19b20000-e8f2-537e-4f6c-d104768a1214"
#define UNI_CHARACTERISTIC_UUID "19b20001-e8f2-537e-4f6c-d104768a1214"

// Universal Communication
class UniCom : public NimBLECharacteristicCallbacks {
private:
    enum { // Header sizes
        ATT_OP_HEADER = 3, // ATT header size for write, read, notification, indication
        UNICOM_HEADER = 4,
        UNICOM_DATA_HEADER = 1,
    };

public:
    typedef enum : uint8_t {
        PACKET_HEADER = 0x0F,
        PACKET_DATA = 0x0D,
    } PacketType;

    typedef enum : uint8_t {
        VALUE = 0x10,
        STRING = 0x20,
        JSON = 0x30,
    } DataType;

    typedef enum : uint8_t {
        NO_FLAG = 0,
        ID_FLAG = 1,
        LEN_FLAG = 2,
    } PacketFlag;

private:
    // It may be expanded with other data, based on flags
    typedef struct {
        uint16_t id;
        uint32_t length;
    } PacketHeaderData;

    typedef struct {
        PacketType packetType;
        DataType dataType;
        uint8_t count;
        PacketFlag flags;
        PacketHeaderData extraData;
    } PacketHeader;

    typedef struct {
        bool isInProgress;
        vector<uint8_t> buffer;
        uint32_t pos;
    } SendingState; 
    SendingState sendState;

    typedef struct {
        bool isInProgress;
        vector<uint8_t> buffer;
        uint8_t count;
        uint8_t counter;
        DataType dataType;
    } ReceivingState; 
    ReceivingState recState;

public :
    // It may be expanded with other data, based on flags
    typedef struct {
        PacketFlag flags;
        uint16_t id;
        uint32_t length;
    } PacketExtraData;

    typedef struct {
        DataType dataType;
        vector<uint8_t> data;
        PacketExtraData extraData;
    } Packet;

    int ATT_MTU;

private:
    bool isInitialized;
    std::function<void(Packet packet)> callback;
    NimBLEService *pService;
    NimBLECharacteristic *pCharacteristic;

public:
    // User interface functions
    UniCom(int bufferSize = 2000);
    void init();
    void addCallback(std::function<void(Packet packet)> callback);

    void sendValue(vector<uint8_t> &value, PacketExtraData* extraData = nullptr);
    void sendValue(uint8_t* value, size_t length, PacketExtraData* extraData = nullptr);
    void sendString(String &str, PacketExtraData* extraData = nullptr);
    void sendJSON(JsonDocument &json, PacketExtraData* extraData = nullptr);

private:
    // Utility
    PacketHeader createHeader(DataType dataType, size_t length, PacketExtraData* extraData = nullptr);
    uint8_t getPacketCount(uint32_t length);
    size_t getFlagSize(PacketHeader &header);
    // Send
    void sendHeader(PacketHeader &header);
    void sendData();
    void setupSendingState(DataType dataType, uint8_t* value, size_t length, PacketExtraData* extraData = nullptr);
    // Receive
    void receiveHeader(vector<uint8_t> &value);
    void receiveData(vector<uint8_t> &value);

    // Nimble callback functions
    void onStatus(NimBLECharacteristic* pCharacteristic, Status s, int code);
    void onWrite(NimBLECharacteristic* pCharacteristic);
};
