#include <MyPasswordManager.h>

// Unicom Callback
// ???

void MyPasswordManager::init() {
    DEBUG_MSG("Initializing MyPasswordManager...\n");
    uniCom.init();
    uniCom.addCallback([=](UniCom::Packet packet) {getPacket(packet);});
    // Alternatively use bind:
    // uniCom.addCallback(std::bind(&MyPasswordManager::getPacket, &myPasswordManager, std::placeholders::_1));
}

void MyPasswordManager::getPacket(UniCom::Packet packet) {
    DEBUG_MSG("Read value...\n");
    if(!UniBLEServer::isAuthenticated) {
        DEBUG_MSG("Device is not authenticated!\n");
        return;
    }

    if(packet.data.size() == 0) {
        DEBUG_MSG("Invalid message received. (Insufficient lenght.)\n");
        return;
    }

    DEBUG_MSG("Message received:");

    switch(packet.dataType) {
        case UniCom::VALUE:
            for(int i = 0; i<packet.data.size(); i++) {
                DEBUG_MSG(" %02x", packet.data[i]);
            }
            DEBUG_MSG("\n");
            typeDecoder(packet.data[0]);
            break;
        case UniCom::STRING:
        case UniCom::JSON:
            DEBUG_MSG(" %s\n", packet.data.data());
            break;
        default:
            DEBUG_MSG("Unknown data type.\n");
            return;
    }

}

void MyPasswordManager::typeDecoder(int type) {
    DEBUG_MSG("Type: %d\n", type);
    String val;
    std::vector<uint8_t> value;
    UniCom::PacketExtraData extraData;
    switch(type) {
        case 1: 
            value.push_back('a');
            value.push_back('b');
            value.push_back('c');
            extraData.flags = UniCom::ID_FLAG;
            extraData.data.id = 123;
            uniCom.sendValue(value, &extraData);
            break;
        case 2:
            val = "1111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000";
            uniCom.sendString(val);
            break;
        case 3:
            sendUserInfo();
            break;
        default:
            DEBUG_MSG("Unknown button\n");
            return;
    }
}

void MyPasswordManager::sendPassword() {
    // Allocate the JSON document
    JsonDocument json;

    // Add values in the document
    json["username"] = "Pelda Peter";
    json["password"] = "Pa55w0rd";

    uniCom.sendJSON(json);
}

void MyPasswordManager::sendUserInfo() {
    // Allocate the JSON document
    JsonDocument json;

    // Add values in the document
    json["username"] = "Minta Mano";
    json["email"] = "menomano@mmm.com";
    json["website"] = "mmm.banya.com";

    uniCom.sendJSON(json);
}
