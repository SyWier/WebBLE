#include <UniCom.h>
// Universal communication callbacks
void UniCom::onStatus(NimBLECharacteristic* pCharacteristic, Status s, int code) {
    String str;
    switch(s) {
        case SUCCESS_INDICATE:
            if(state.isInProgress == SENDING) {
                sendExtPacket();
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

    if(state.isInProgress == SENDING) {
        DEBUG_MSG("Sending already in progress.\n");
        return;
    }

    vector<uint8_t> value = pCharacteristic->getValue();
    DEBUG_MSG("Received value:");
    for(int i = 0; i<value.size(); i++) {
        DEBUG_MSG(" %02x", value[i]);
    }
    DEBUG_MSG("\n");

    Packet packet = {
        .dataType = VALUE,
        .extraData = {.id = 0},
    };
    packet.data = pCharacteristic->getValue();
    packet.data.push_back('\0');

    callback(packet);

    // DEBUG_MSG("Value received: %s.\n", str.c_str());

    // if(str.length() > state.length) {
    //     DEBUG_MSG("String is larger then buffer!\n");
    //     return;
    // }
    // memcpy(state.buffer, str.c_str(), str.length());


    // char type = str[0];
    // switch(type) {
    //     case 'O':
    //         inBuffer += str.substring(1);
    //         break;
    //     case 'X':
    //         inBuffer += str.substring(1);
    //         callback(inBuffer);
    //         inBuffer = "";
    //         break;
    //     default:
    //         DEBUG_MSG("Unknown packet type.\n");
    // }
}

// Universal communication
UniCom::UniCom(int bufferSize /* 2000 */) {
    isInitialized = false;

    callback = nullptr;
    pService = nullptr;
    pCharacteristic = nullptr;

    state.isInProgress = IDLE;
    state.buffer.reserve(bufferSize);
    state.pos = 0;
    state.count = 0;
    state.counter = 0;

    init();
}

void UniCom::init() {
    DEBUG_MSG("Initializing UniCom...\n");
    if(!MyBLEServer::isInitialized) {
        DEBUG_MSG("Error: MyBLEServer hasn't started yet!\n");
        return;
    }

    if(isInitialized) {
        DEBUG_MSG("UniCom already initialized!\n");
        return;
    }
    isInitialized = true;

    att_mtu = NimBLEDevice::getMTU();

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

void UniCom::addCallback(std::function<void(Packet packet)> callback) {
    this->callback = callback;
}

// void UniCom::sendPacket() {
//     // Need to send out the string + att header (3 byte) in each packet
//     DEBUG_MSG("Sending packet...\n");
//     if(str_pos + packet_data < outBuffer.length()) {
//         String str = "O" + outBuffer.substring(str_pos, str_pos + packet_data);
//         str_pos += packet_data;
//         pCharacteristic->indicate(str);
//     } else {
//         DEBUG_MSG("Lasts packet!\n");
//         // Packet Header + Packet Data
//         String str = "X" + outBuffer.substring(str_pos, str_pos + packet_data);
//         pCharacteristic->indicate(str);
//         isInProgress = false;
//     }
// }

uint8_t UniCom::getPacketCount(uint32_t length) {
    return length/att_mtu + (length%att_mtu != 0);
}

size_t UniCom::getFlagSize(PacketHeader &header) {
    // Unmap additional size based on flags selected
    size_t unmapFlag[256] = {0, 2, 4, 6};
    return unmapFlag[header.flags];
}

void UniCom::sendPacket(PacketHeader &header, PacketHeaderData &headerData) {
    DEBUG_MSG("Sending packet...\n");

    // Calculate packet size
    size_t length = PACKET_HEADER_SIZE + getFlagSize(header) + headerData.data.size();
    if(length > att_mtu - ATT_HEADER) {
        DEBUG_MSG("Packet size is bigger then MTU!\n");
        return;
    }

    // Allocate memory
    uint8_t* data = new uint8_t[length];
    uint32_t pos = 0;

    // Fill memory with data
    memcpy(data, &header, PACKET_HEADER_SIZE);
    pos += PACKET_HEADER_SIZE;

    // Optional data based on flag
    if(header.flags&ID_FLAG) {
        memcpy(data+pos, &headerData.id, 2);
        pos += 2;
    }

    if(header.flags&LEN_FLAG) {
        memcpy(data+pos, &headerData.length, 4);
        pos += 4;
    }

    // Optional data based on packet data type
    if(header.dataType == VALUE) {
        memcpy(data+pos, headerData.data.data(), headerData.data.size());
        pos += headerData.data.size();
    }

    // Send packet
    pCharacteristic->indicate(data, length);

    delete data;
}

void UniCom::sendExtPacket() {
    DEBUG_MSG("Sending ext packet...\n");

    // Temporary variable for packet data
    int payloadSize = att_mtu - ATT_HEADER - 1;
    int length;
    uint8_t* data = new uint8_t[payloadSize + 1];

    // Packet type
    data[0] = PACKED_EXT_DATA;

    // Test if we run out of buffer
    if(state.pos + payloadSize <= state.buffer.size()) {
        length = payloadSize;
    } else {
        length = state.buffer.size() - state.pos;
    }

    // Set packet data
    memcpy(data + 1, state.buffer.data() + state.pos, length);
    state.pos += length;

    pCharacteristic->indicate(data, length+1);

    // Free up temporary variable
    delete data;

    if(state.pos == state.buffer.size()) {
        state.isInProgress = IDLE;
    }
}

void UniCom::sendValue(uint8_t* value, size_t length) {
    DEBUG_MSG("Sending value...\n");
    if(state.isInProgress) {
        DEBUG_MSG("Transaction is already in progress!\n");
        return;
    }

    PacketHeader header = {
        .packetType = PACKET_DATA,
        .dataType = VALUE,
        .flags = NO_FLAG,
        .count = 0,
    };

    PacketHeaderData headerData;
    headerData.data.assign(value, value+length);

    sendPacket(header, headerData);
}

void UniCom::sendValue(vector<uint8_t> &value) {
    sendValue(value.data(), value.size());
}

void UniCom::sendString(String &str) {
    DEBUG_MSG("Sending String...\n");
    if(state.isInProgress) {
        DEBUG_MSG("Transaction is already in progress!\n");
        return;
    }

    if(str.length() > state.buffer.capacity()) {
        DEBUG_MSG("String is larger then buffer!\n");
        return;
    }

    PacketHeader packetHeader = {
        .packetType = PACKET_DATA,
        .dataType = STRING,
        .flags = NO_FLAG,
        .count = getPacketCount(str.length()),
    };

    PacketHeaderData packetHeaderData;

    state.buffer.assign(str.c_str(), str.c_str() + str.length());
    state.pos = 0;
    state.isInProgress = SENDING;

    sendPacket(packetHeader, packetHeaderData);
}

void UniCom::sendJSON(JsonDocument &json) {
    DEBUG_MSG("Sending JSON...\n");
    // Generate the minified JSON and send it
    String str;
    serializeJson(json, str);
    DEBUG_MSG("Serialized JSON: %s\n", str.c_str());
    sendString(str);
}
