import { RemoveCircleRounded } from "@mui/icons-material";

class UniCom {
    constructor(webble) {
        console.log("Creating UniCom...");

        // Handle to the ble server
        this.webble = webble;

        // "Enums"
        this.sizes = {};
        this.sizes.ATT_MTU = 512; // Maximum value
        this.sizes.ATT_OP_HEADER = 3;
        this.sizes.UNICOM_HEADER = 8;
        this.sizes.UNICOM_DATA_HEADER = 1;
        this.sizes.PAYLOAD_SIZE = this.sizes.ATT_MTU - this.sizes.ATT_OP_HEADER - this.sizes.UNICOM_DATA_HEADER;
        Object.freeze(this.sizes);

        this.packetType = {};
        this.packetType.HEADER = 0x0F;
        this.packetType.DATA = 0x0D;
        Object.freeze(this.packetType);

        this.dataType = {};
        this.dataType.VALUE = 0x10;
        this.dataType.STRING = 0x20;
        this.dataType.JSON = 0x30;
        Object.freeze(this.dataType);

        this.flags = {};
        this.flags.NO_FLAG = 0;
        this.flags.ID_FLAG = 1;
        Object.freeze(this.flags);

        // Unicom service
        this.uniCom = {
            serviceUUID : '19b20000-e8f2-537e-4f6c-d104768a1214',
            service : null,
            charUUID : '19b20001-e8f2-537e-4f6c-d104768a1214',
            char : null,
        };

        // Comminucation state objects
        this.send = {
            isInProgress : false,
            buffer : new Uint8Array(2000),
            pos : 0,
        };

        this.receive = {
            isInProgress : false,
            buffer : new Uint8Array(2000),
            pos : 0,
            length : 0,
            dataType : 0,
        };

        this.packet = {
            dataType : 0,
            data : new Uint8Array(2000),
            extraData : { flags : 0 },
        }

        this.callback = () => null;
        this.isRequested = false;

        this.isInitialized = false;
    }

    async getService(bleServer) {
        try {
            // Get service
            this.uniCom.service = await bleServer.getPrimaryService(this.uniCom.serviceUUID);
            console.log("Service discovered:", this.uniCom.service.uuid);

            // Get characteristic
            this.uniCom.char = await this.uniCom.service.getCharacteristic(this.uniCom.charUUID);
            console.log("Characteristic discovered:", this.uniCom.char.uuid);
            this.uniCom.char.addEventListener('characteristicvaluechanged', this.handleReceived.bind(this));

            // Start notifications
            this.uniCom.char.startNotifications();
            console.log("Notifications Started.");

            this.isInitialized = true;
        } catch(error) {
            console.error("Error: ", error);
        }
    }

    // Helper function to separate incoming data into objects
    unpack(packet) {
        let received = {
            header : {
                packetType : packet[0],
                dataType : packet[1],
                flags : uint8ArrayToNum(packet, 2, 2),
                length : uint8ArrayToNum(packet, 4, 4),
            },
            extraData : undefined,
        };

        if(received.header.flags == this.flags.ID_FLAG) {
            received.extraData = {id: uint8ArrayToNum(packet, 8, 2)};
        }

        return received;
    }

    handleReceived(event) {
        // Convert data
        let value = new Uint8Array(event.target.value.buffer);
        let packetType = value[0];
        console.log("Received value (HEX): ", bufferToHex(value));

        // Proccess data
        switch(packetType) {
            case this.packetType.HEADER:
                this.handleHeader(value);
                break;
            case this.packetType.DATA:
                this.handleData(value);
                break;
            default:
                console.log("Error: Unknown packet type!");
        }
    }

    handleHeader(value) {
        console.log("Received header packet...");

        let received = this.unpack(value);
        let header = received.header;
        this.packet.extraData.id = received.extraData?.id;

        switch(header.dataType) {
            case this.dataType.VALUE:
            case this.dataType.STRING:
            case this.dataType.JSON:
                this.initializeNewData(header.dataType, header.length);
                break;
            default:
                console.log("Error: Unknown data type!");
        }
    }

    handleData(value) {
        console.log("Received data packet...");

        let subarr = value.subarray(1);
        this.receive.buffer.set(subarr, this.receive.pos);
        this.receive.pos += subarr.length;

        if(this.receive.pos >= this.receive.length) {
            // Fit array to data
            this.packet.dataType = this.receive.dataType;
            this.packet.data = this.receive.buffer.subarray(0, this.receive.length);

            if(!this.isRequested) {
                this.callback(this.packet);
            }

            this.receive.isInProgress = false;
        }
    }

    addCallback(func) {
        this.callback = func;
    }

    initializeNewData(dataType, length) {
        this.receive.isInProgress = true;
        this.receive.buffer = new Uint8Array(2000);
        this.receive.pos = 0;
        this.receive.length = length;
        this.receive.dataType = dataType;
    }

    getFlagSize(flags) {
        let size = 0;
        if(flags & this.flags.ID_FLAG) {
            size += 2;
        } 
        return size;
    }

    async sendHeader(header) {
        if(this.isInProgress) {
            console.log("Cannot send packet! A transaction is already in progress.");
            return;
        }

        let length = this.sizes.UNICOM_HEADER + this.getFlagSize(header.flags);
        let offset = 0;

        let data = new Int8Array(length);

        data[0] = this.packetType.HEADER;
        data[1] = header.dataType;

        let flags = numToUint8Array(header.flags, 2);
        data[2] = flags[0];
        data[3] = flags[1];

        let lenArr = numToUint8Array(header.length, 4);
        data[4] = lenArr[0];
        data[5] = lenArr[1];
        data[6] = lenArr[2];
        data[7] = lenArr[3];

        offset += 8;

        if(header.flags & this.flags.ID_FLAG) {
            data.set(numToUint8Array(header.data.id, 2), offset);
            offset += 2; // ID field size
        }

        console.log("Sending header (HEX): ", bufferToHex(data));
        await this.uniCom.char.writeValue(data);
    }

    async sendData(value) {
        let data = new Uint8Array(this.sizes.UNICOM_DATA_HEADER + value.length);
        data[0] = this.packetType.DATA;
        data.set(value, this.sizes.UNICOM_DATA_HEADER);
        await this.uniCom.char.writeValue(data);
    }

    copyExtraToHeader(header, extraData) {
        header.flags = extraData.flags;
        header.data.id = extraData.data.id;
    }

    async sendStream(data) {
        let pos = 0;

        while(pos < data.length) {
            let length = Math.min(this.sizes.PAYLOAD_SIZE, data.length-pos);
            let subarr = data.subarray(pos, pos+length);
            pos += length;
            await this.sendData(subarr);
        }
    }

    createHeader(dataType, length, extraData = null) {
        let header = {
            packetType : this.packetType.HEADER,
            dataType : dataType,
            flags : this.flags.NO_FLAG,
            length : length,
        };

        if(extraData != null) {
            header.data = {};
            header.data.id = extraData.data.id;
        }

        return header;
    }

    async sendRawData(dataType, value, extraData = null) {
        console.log("Sending data...");

        if(!this.webble.isBleConnected()) {
            return false;
        }

        if(!this.isInitialized) {
            return false;
        }

        if(this.send.isInProgress) {
            console.log("Cannot send value! A transaction is already in progress!");
            return;
        }

        this.send.isInProgress = true;

        let header = this.createHeader(dataType, value.length, extraData);
        await this.sendHeader(header, extraData);
        await this.sendStream(value);

        this.send.isInProgress = false;
    }

    async sendValue(value, extraData = null) {
        console.log("Sending value...");
        await this.sendRawData(this.dataType.VALUE, value, extraData);
    }

    async sendString(string, extraData = null) {
        console.log("Sending string...");
        await this.sendRawData(this.dataType.STRING, new TextEncoder().encode(string), extraData);
    }

    async sendJSON(object, extraData = null) {
        console.log("Sending json...");

        let serialized = JSON.stringify(object);
        await this.sendRawData(this.dataType.STRING, new TextEncoder().encode(serialized), extraData);
    }

    async requestData(command, extraData = null) {
        if(this.isRequested) {
            console.log("A request is already is in progess!");
        }

        this.isRequested = true;
        await this.sendRawData(this.dataType.VALUE, command, extraData);
        while(this.receive.isInProgress) {
            await new Promise(resolve => setTimeout(resolve, 500));
        }
        this.isRequested = false;
    }

    async requestValue(command, extraData = null) {
        console.log("Requesting value...");

        await this.requestData(command, extraData = null);

        return this.packet;
    }

    async requestString(command, extraData = null) {
        console.log("Requesting string...");

        await this.requestData(command, extraData = null);

        this.packet.data = new TextDecoder().decode(this.packet.data);
        return this.packet;
    }

    async requestJSON(command, extraData = null) {
        console.log("Requesting JSON...");

        await this.requestData(command, extraData = null);

        let jsonString = new TextDecoder().decode(this.packet.data);
        this.packet.data = JSON.parse(jsonString);
        return this.packet;
    }

    getServiceUUID() {
        return this.uniCom.serviceUUID;
    }

    stop() {
        if(this.uniCom.char) {
            this.uniCom.char.stopNotifications();
            console.log("UniCom notifications stopped.");
        }
    }
};


function uint8ArrayToNum(arr, offset, bytes) {
    if(bytes == 2)
        return new DataView(arr.buffer, offset, bytes).getUint16(0, true);
    if(bytes == 4)
        return new DataView(arr.buffer, offset, bytes).getUint32(0, true);
    return 0;
}

function numToUint8Array(num, bytes = 4) {
    let arr = new Uint8Array(bytes);
  
    for (let i = 0; i < bytes; i++) {
      arr[i] = num % 256;
      num = Math.floor(num / 256);
    }
  
    return arr;
}

function bufferToHex(buffer) {
    return [...new Uint8Array (buffer)]
        .map (b => b.toString (16).padStart (2, "0"))
        .join (" ");
}

export default UniCom;
