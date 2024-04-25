#include <UniCom.h>
// Universal communication callbacks
void UniCom::onStatus(NimBLECharacteristic* pCharacteristic, Status s, int code) {
    String str;
    switch(s) {
        case SUCCESS_INDICATE:
            if(isInProgress) {
                sendPacket();
            }
            str = "SUCCESS_INDICATE";
            break;
        case SUCCESS_NOTIFY: str = "SUCCESS_NOTIFY"; break;
        case ERROR_INDICATE_DISABLED: str = "ERROR_INDICATE_DISABLED"; break;
        case ERROR_NOTIFY_DISABLED: str = "ERROR_NOTIFY_DISABLED"; break;
        case ERROR_GATT: str = "ERROR_GATT"; break;
        case ERROR_NO_CLIENT: str = "ERROR_NO_CLIENT"; break;
        case ERROR_INDICATE_TIMEOUT: str = "ERROR_INDICATE_TIMEOUT"; break;
        case ERROR_INDICATE_FAILURE: str = "ERROR_INDICATE_FAILURE"; break;
        default: str = "Unkown status"; break;
    }
    DEBUG_MSG("Status: %s.\n", str.c_str());
}

void UniCom::onWrite(NimBLECharacteristic* pCharacteristic) {
    if(callback == nullptr) {
        DEBUG_MSG("No callback function has been set.\n");
        return;
    }

    String str = pCharacteristic->getValue();
    char type = str[0];
    switch(type) {
        case 'O':
            inBuffer += str.substring(1);
            break;
        case 'X':
            inBuffer += str.substring(1);
            callback(inBuffer);
            inBuffer = "";
            break;
        default:
            DEBUG_MSG("Unknown packet type.\n");
    }
}

// Universal communication
UniCom::UniCom(int bufferSize /* 2000 */) {
    callback = nullptr;
    pService = nullptr;
    pCharacteristic = nullptr;

    att_mtu = NimBLEDevice::getMTU();
    att_data = att_mtu - ATT_HEADER;
    packet_data = att_data - UNICOM_HEADER;
    packet_size = packet_data + UNICOM_HEADER;
    isInProgress = false;

    inBuffer.reserve(bufferSize);
    outBuffer.reserve(bufferSize);

    init();
}

void UniCom::init() {
    DEBUG_MSG("Initializing UniCom...\n");
    if(!MyBLEServer::isInitialized) {
        DEBUG_MSG("Error: MyBLEServer hasn't started yet!\n");
        return;
    }
    static bool initialized = false;
    if(initialized) {
        DEBUG_MSG("UniCom already initialized!\n");
        return;
    }
    initialized = true;

    DEBUG_MSG("Creating UniCom Service...\n");
    pService = MyBLEServer::createService(UNI_SERVICE_UUID);

    // Create UniCom Characteristic with UUID
    DEBUG_MSG("Creating UniCom Characteristics...\n");
    pCharacteristic = pService->createCharacteristic(
        UNI_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ |
        NIMBLE_PROPERTY::READ_ENC |
        NIMBLE_PROPERTY::READ_AUTHEN |
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_ENC |
        NIMBLE_PROPERTY::WRITE_AUTHEN |
        NIMBLE_PROPERTY::INDICATE
    );

    // Set characteristic callback
    pCharacteristic->setCallbacks(this);

    // Start service
    DEBUG_MSG("Start UniCom service!\n");
    pService->start();

    // Advertice service
    MyBLEServer::adverticeService(UNI_SERVICE_UUID);
}

void UniCom::addCallback(std::function<void(String &str)> callback) {
    this->callback = callback;
}

// void UniCom::createPacket() {
//     PacketHeader packetHeader = {
//         .packetType = PACKET_DATA,
//         .dataType = VALUE,
//         .flags = NO_FLAG,
//         .count = 0,
//     };
//     DEBUG_MSG("\nSize of header: %d\n", sizeof(packetHeader));
//     DEBUG_MSG("Size of packet type: %d\n", sizeof(packetHeader.packetType));
//     DEBUG_MSG("Size of data type: %d\n", sizeof(packetHeader.dateType));
//     DEBUG_MSG("Size of flag: %d\n", sizeof(packetHeader.flags));
//     DEBUG_MSG("Size of count: %d\n\n", sizeof(packetHeader.count));

//     std::vector<uint8_t> value;
//     String str = "Hello";

//     value.assign((uint8_t*)&packetHeader, (uint8_t*)&packetHeader+3);
//     value.insert(value.end(), str.begin(), str.end());

//     pCharacteristic->indicate(value);
// }

void UniCom::sendPacket() {
    // Need to send out the string + att header (3 byte) in each packet
    DEBUG_MSG("Sending packet...\n");
    if(str_pos + packet_data < outBuffer.length()) {
        String str = "O" + outBuffer.substring(str_pos, str_pos + packet_data);
        str_pos += packet_data;
        pCharacteristic->indicate(str);
    } else {
        DEBUG_MSG("Lasts packet!\n");
        // Packet Header + Packet Data
        String str = "X" + outBuffer.substring(str_pos, str_pos + packet_data);
        pCharacteristic->indicate(str);
        isInProgress = false;
    }
}

size_t UniCom::getFlagSize(PacketHeader packetHeader) {
    // Unmap additional size based on flags selected
    size_t unmapFlag[256] = {0, 2, 4, 6};
    return unmapFlag[packetHeader.flags];
}

void UniCom::sendPacket(PacketHeader packetHeader, PacketHeaderData packetHeaderData) {
    // Calculate packet size
    size_t length = PACKET_HEADER_SIZE + getFlagSize(packetHeader) + packetHeaderData.packetData.size;
    if(length > att_mtu - ATT_HEADER) {
        DEBUG_MSG("Packet size is bigger then MTU!\n");
        return;
    }

    // Allocate memory
    uint8_t* data = new uint8_t[length];
    uint32_t pos = 0;

    // Fill memory with data
    memcpy(data, &packetHeader, PACKET_HEADER_SIZE);
    pos += PACKET_HEADER_SIZE;

    // Optional data based on flag
    if(packetHeader.flags&ID_FLAG) {
        memcpy(data+pos, &packetHeaderData.id, 2);
        pos += 2;
    }

    if(packetHeader.flags&LEN_FLAG) {
        memcpy(data+pos, &packetHeaderData.length, 4);
        pos += 4;
    }

    // Optional data based on packet data type
    if(packetHeader.dataType == VALUE) {
        memcpy(data+pos, packetHeaderData.packetData.data, packetHeaderData.packetData.size);
        pos += packetHeaderData.packetData.size;
    }

    // Send packet
    pCharacteristic->indicate(data, length);
}

void UniCom::sendValue(uint8_t* value, size_t length) {
    PacketHeader packetHeader = {
        .packetType = PACKET_DATA,
        .dataType = VALUE,
        .flags = NO_FLAG,
        .count = 0,
    };

    PacketHeaderData packetHeaderData {
        .packetData = {
            .data = value,
            .size = length
        }
    };

    sendPacket(packetHeader, packetHeaderData);
}

void UniCom::sendValue(vector<uint8_t> &value) {
    sendValue(value.data(), value.size());
}

void UniCom::sendString(String &str) {
    DEBUG_MSG("Sending String...\n");
    if(isInProgress) {
        DEBUG_MSG("Transaction is already in progress!\n");
        return;
    }
    isInProgress = true;

    outBuffer = str;
    str_pos = 0;

    sendPacket();
}

void UniCom::sendJSON(JsonDocument &json) {
    DEBUG_MSG("Sending JSON...\n");
    // Generate the minified JSON and send it
    String str;
    serializeJson(json, str);
    DEBUG_MSG("Serialized JSON: %s\n", str.c_str());
    sendString(str);
}
