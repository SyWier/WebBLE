#pragma once

#include <MyBLEServer.h>
#include <ArduinoJson.h>

using std::vector;

// Universal communication service
#define UNI_SERVICE_UUID        "19b20000-e8f2-537e-4f6c-d104768a1214"
#define UNI_CHARACTERISTIC_UUID "19b20001-e8f2-537e-4f6c-d104768a1214"

// Callback function for user interface
class UniComCallback {
public:
    virtual void readValue(String &value);
};

// Universal Communication
class UniCom : public NimBLECharacteristicCallbacks{
private:
    enum {
        ATT_HEADER = 3, // ATT header size for write, read, notification, indication
        UNICOM_HEADER = 1,
        PACKET_HEADER_SIZE = 4,
        PACKET_VIRTUAL_MAX_SIZE,
    };

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

    typedef struct {
        PacketType packetType;
        DataType dataType;
        PacketFlag flags;
        uint8_t count;
    } PacketHeader;

    typedef struct {
        uint8_t* data;
        size_t size;
    } PacketData;

    typedef struct {
        uint16_t id;
        uint32_t length;
        PacketData packetData;
    } PacketHeaderData;

        // uint16_t ID;
        // uint32_t length;
        // std::vector<uint8_t> data;

    int att_mtu;
    int att_data;
    int packet_data;
    int packet_size;

    String inBuffer;
    String outBuffer;
    int str_pos;
    bool isInProgress;

private:
    std::function<void(String &str)> callback;
    NimBLEService *pService;
    NimBLECharacteristic *pCharacteristic;

public:
    UniCom(int bufferSize = 2000);
    void init();
    void addCallback(std::function<void(String &str)> callback);
    
    // void createPacket();

    size_t getFlagSize(PacketHeader packetHeader);
    void sendPacket(PacketHeader packetHeader, PacketHeaderData packetHeaderData);
    void sendExtPacket(const uint8_t* value, size_t length);

    void sendPacket();
    void sendValue(vector<uint8_t> &value);
    void sendValue(uint8_t* value, size_t length);
    void sendString(String &str);
    void sendJSON(JsonDocument &json);
    
    // Nimble callback functions
    void onStatus(NimBLECharacteristic* pCharacteristic, Status s, int code);
    void onWrite(NimBLECharacteristic* pCharacteristic);
};
