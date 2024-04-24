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
    if(callback == nullptr)
        return;

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

void UniCom::addCallback(std::function<void(String &value)> callback) {
    this->callback = callback;
}

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
