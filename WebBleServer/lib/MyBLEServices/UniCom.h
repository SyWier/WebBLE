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
    enum {
        ATT_HEADER = 3, // ATT header size for write, read, notification, indication
        UNICOM_HEADER = 1,
        PACKET_HEADER_SIZE = 4,
        PACKET_VIRTUAL_MAX_SIZE,
    };

public:
    typedef enum : uint8_t {
        PACKET_DATA = 0x0D,
        PACKED_EXT_DATA = 0xED,
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
    typedef struct {
        PacketType packetType;
        DataType dataType;
        PacketFlag flags;
        uint8_t count;
    } PacketHeader;

    typedef struct {
        uint16_t id;
        uint32_t length;
        vector<uint8_t> data;
    } PacketHeaderData;

    typedef enum : uint8_t {
        IDLE = 0,
        SENDING = 1,
        RECEIVING = 2,
    } ProgressType;

    typedef struct {
        ProgressType isInProgress;
        vector<uint8_t> buffer;
        uint32_t pos;
        uint8_t count;
        uint8_t counter;
    } Communicationtate; 
    Communicationtate state;

public :
    // It may be expanded with other data, based on flags
    typedef struct {
        uint16_t id;
    } PacketExtraData;

    typedef struct {
        DataType dataType;
        vector<uint8_t> data;
        PacketExtraData extraData;
    } Packet;

    int att_mtu;

private:
    bool isInitialized;
    std::function<void(Packet packet)> callback;
    NimBLEService *pService;
    NimBLECharacteristic *pCharacteristic;

public:
    UniCom(int bufferSize = 2000);
    void init();
    void addCallback(std::function<void(Packet packet)> callback);

    void sendValue(vector<uint8_t> &value);
    void sendValue(uint8_t* value, size_t length);
    void sendString(String &str);
    void sendJSON(JsonDocument &json);

private:
    uint8_t getPacketCount(uint32_t length);
    size_t getFlagSize(PacketHeader &header);
    void sendPacket(PacketHeader &header, PacketHeaderData &headerData);
    void sendExtPacket();

    // Nimble callback functions
    void onStatus(NimBLECharacteristic* pCharacteristic, Status s, int code);
    void onWrite(NimBLECharacteristic* pCharacteristic);
};
