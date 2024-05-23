#pragma once

#include <UniBLEServer.h>
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
        UNICOM_HEADER = 8,
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

    typedef enum : uint16_t {
        NO_FLAG = 0,
        ID_FLAG = 1,
    } PacketFlag;

private:
    // It may be expanded with other data, based on flags
    typedef struct {
        uint16_t id;
    } PacketHeaderData;

    typedef struct {
        PacketType packetType;
        DataType dataType;
        PacketFlag flags;
        uint32_t length;
        PacketHeaderData data;
    } PacketHeader;

public:
    typedef struct {
        PacketFlag flags;
        PacketHeaderData data;
    } PacketExtraData;

    typedef struct {
        DataType dataType;
        vector<uint8_t> data;
        PacketExtraData extraData;
    } Packet;

    int ATT_MTU;

private:
    typedef struct {
        bool isInProgress;
        vector<uint8_t> buffer;
        uint32_t pos;
    } SendingState; 
    SendingState send;

    typedef struct {
        bool isInProgress;
        Packet packet;
        uint32_t length;
    } ReceivingState; 
    ReceivingState receive;

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
