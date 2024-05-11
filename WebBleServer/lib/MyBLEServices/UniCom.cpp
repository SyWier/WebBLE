#include <UniCom.h>

// Universal communication callbacks
void UniCom::onStatus(NimBLECharacteristic* pCharacteristic, Status s, int code) {
    String str;
    switch(s) {
        case SUCCESS_INDICATE:
            if(sendState.isInProgress) {
                sendData();
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

    vector<uint8_t> value = pCharacteristic->getValue();

    if(value.size() == 0) {
        DEBUG_MSG("Empty packet received.\n");
        return;
    }

    // DEBUG: print value in hex format
    for(int i = 0; i<value.size(); i++) {
        DEBUG_MSG(" %02x", value[i]);
    }
    DEBUG_MSG("\n");

    PacketType packetType = (PacketType)value[0];
    switch(packetType) {
        case PACKET_HEADER:
            receiveHeader(value);
            break;
        case PACKET_DATA:
            receiveData(value);
            break;
        default:
            DEBUG_MSG("Unkown packet type\n");
            return;
    }
}

void UniCom::receiveHeader(vector<uint8_t> &value) {
    if(recState.isInProgress) {
        DEBUG_MSG("Cannot receive new header, a transaction is already in progress!\n");
        return;
    }

    // PacketHeader* packetHeader = (PacketHeader*)value.data();
    DataType dataType = (DataType)value[1];
    switch(dataType) {
        case VALUE: case STRING: case JSON:
            break;
        default:
            DEBUG_MSG("Unkown data type\n");
            return;
    }

    // Initialize buffer
    recState.isInProgress = true;
    recState.count = value[2];
    recState.counter = 0;
    recState.buffer.clear();
    recState.dataType = dataType;
}

void UniCom::receiveData(vector<uint8_t> &value) {
    if(!recState.isInProgress) {
        DEBUG_MSG("Error: received data while not expecting one!\n");
        return;
    }

    if(recState.buffer.size() + value.size() > recState.buffer.capacity()) {
        DEBUG_MSG("Error: received data is bigger then buffer!\n");
        return;
    }

    // Add data to buffer
    recState.buffer.insert(recState.buffer.end(), value.begin()+UNICOM_DATA_HEADER, value.end());

    // If all packets are received, send data to application
    recState.counter += 1;
    if(recState.counter >= recState.count) {
        // Create packet
        Packet packet = {
            .dataType = recState.dataType,
            .data = recState.buffer,
            .extraData = {.id = 0}, // No extra data yet
        };
        
        // Null character if we sends a string based packet
        if(recState.dataType == STRING || recState.dataType == JSON) {
            packet.data.push_back('\0');
        }

        callback(packet);

        recState.isInProgress = false;
    }
}

// Universal communication
UniCom::UniCom(int bufferSize /* 2000 */) {
    isInitialized = false;

    callback = nullptr;
    pService = nullptr;
    pCharacteristic = nullptr;

    sendState.buffer.reserve(bufferSize);
    sendState.pos = 0;
    sendState.isInProgress = false;

    recState.buffer.reserve(bufferSize);
    recState.count = 0;
    recState.counter = 0;
    recState.dataType = (DataType)0;
    recState.isInProgress = false;

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

    ATT_MTU = NimBLEDevice::getMTU();

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

uint8_t UniCom::getPacketCount(uint32_t length) {
    uint32_t payloadSize = ATT_MTU - ATT_OP_HEADER - UNICOM_DATA_HEADER;
    return length/payloadSize + (length%payloadSize != 0);
}

size_t UniCom::getFlagSize(PacketHeader &header) {
    // Unmap additional size based on flags selected
    size_t unmapFlag[256] = {0, 2, 4, 6};
    return unmapFlag[header.flags];
}

void UniCom::sendHeader(PacketHeader &header) {
    DEBUG_MSG("Sending header...\n");

    // Calculate packet size
    size_t length = UNICOM_HEADER + getFlagSize(header);

    // Allocate memory
    uint8_t* data = new uint8_t[length];
    uint32_t pos = 0;

    // Fill memory with data
    memcpy(data, &header, UNICOM_HEADER);
    pos += UNICOM_HEADER;

    // Optional data based on flag
    if(header.flags&ID_FLAG) {
        memcpy(data+pos, &header.extraData.id, 2);
        pos += 2;
    }

    if(header.flags&LEN_FLAG) {
        memcpy(data+pos, &header.extraData.length, 4);
        pos += 4;
    }

    // Send packet
    pCharacteristic->indicate(data, length);

    delete data;
}

void UniCom::sendData() {
    DEBUG_MSG("Sending data...\n");

    // Temporary variable for packet data
    uint32_t payloadSize = ATT_MTU - ATT_OP_HEADER - UNICOM_DATA_HEADER;
    uint32_t length = min(payloadSize, sendState.buffer.size() - sendState.pos);
    uint8_t* data = new uint8_t[length + UNICOM_DATA_HEADER];

    // Packet type
    data[0] = PACKET_DATA;

    // Set packet data
    memcpy(data + UNICOM_DATA_HEADER, sendState.buffer.data() + sendState.pos, length);
    sendState.pos += length;

    pCharacteristic->indicate(data, length+1);

    // Free up temporary variable
    delete data;

    if(sendState.pos == sendState.buffer.size()) {
        sendState.isInProgress = false;
    }
}

UniCom::PacketHeader UniCom::createHeader(DataType dataType, size_t length, PacketExtraData* extraData /* nullptr */) {
    PacketHeader header = {
        .packetType = PACKET_HEADER,
        .dataType = dataType,
        .count = getPacketCount(length),
        .flags = NO_FLAG,
    };

    if(extraData != nullptr) {
        header.flags = extraData->flags;
        header.extraData.id = extraData->id;
        header.extraData.length = extraData->length;
    }

    return header;
}

void UniCom::setupSendingState(DataType dataType, uint8_t* value, size_t length, PacketExtraData* extraData /* nullptr */) {
    if(sendState.isInProgress) {
        DEBUG_MSG("Error: Cannot initiate new transaction, a transaction is already in progress!\n");
        return;
    }

    if(length > sendState.buffer.capacity()) {
        DEBUG_MSG("Error: Data is larger then buffer!\n");
        return;
    }

    sendState.isInProgress = true;
    sendState.buffer.assign(value, value + length);
    sendState.pos = 0;

    PacketHeader header = createHeader(dataType, length, extraData);
    sendHeader(header);
}

void UniCom::sendValue(uint8_t* value, size_t length, PacketExtraData* extraData /* nullptr */) {
    DEBUG_MSG("Sending value...\n");
    setupSendingState(VALUE, value, length, extraData);
}

void UniCom::sendValue(vector<uint8_t> &value, PacketExtraData* extraData /* nullptr */) {
    sendValue(value.data(), value.size(), extraData);
}

void UniCom::sendString(String &str, PacketExtraData* extraData /* nullptr */) {
    DEBUG_MSG("Sending String...\n");
    setupSendingState(STRING, (uint8_t*)str.c_str(), str.length(), extraData);
}

void UniCom::sendJSON(JsonDocument &json, PacketExtraData* extraData /* nullptr */) {
    DEBUG_MSG("Sending JSON...\n");

    // Generate the minified JSON
    String serialized;
    serializeJson(json, serialized);

    setupSendingState(JSON, (uint8_t*)serialized.c_str(), serialized.length(), extraData);
}
