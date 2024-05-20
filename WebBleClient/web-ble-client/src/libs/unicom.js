import { RemoveCircleRounded } from "@mui/icons-material";

class UniCom {
    constructor(webble) {
        console.log("Creating UniCom...");

        this.webble = webble;
        // "Enums"
        this.sizes = {
            att_mtu : 103, // Max 512
            att_op_header : 3,
            unicom_header : 8,
            unicom_data_header : 1,
        }
        this.payload_size = this.sizes.att_mtu - this.sizes.att_op_header - this.sizes.unicom_data_header;

        this.packetType = {
            header : 0x0F,
            data : 0x0D
        };

        this.dataType = {
            value : 0x10,
            string : 0x20,
            json : 0x30
        };

        this.flags = {
            no_flag : 0,
            id_flag : 1,
        };

        // Unicom service
        this.uniCom = {
            serviceUUID : '19b20000-e8f2-537e-4f6c-d104768a1214',
            service : null,
            charUUID : '19b20001-e8f2-537e-4f6c-d104768a1214',
            char : null,
        };

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

    uint8ArrayToNum(arr, offset, bytes) {
        if(bytes == 2)
            return new DataView(arr.buffer, offset, bytes).getUint16(0, true);
        if(bytes == 4)
            return new DataView(arr.buffer, offset, bytes).getUint32(0, true);
        return 0;
    }

    numToUint8Array(num, bytes = 4) {
        let arr = new Uint8Array(bytes);
      
        for (let i = 0; i < bytes; i++) {
          arr[i] = num % 256;
          num = Math.floor(num / 256);
        }
      
        return arr;
    }

    unpack(packet) {
        let received = {
            header : {
                packetType : packet[0],
                dataType : packet[1],
                flags : this.uint8ArrayToNum(packet, 2, 2),
                length : this.uint8ArrayToNum(packet, 4, 4),
            },
            extraData : undefined,
        };

        if(received.header.flags == this.flags.id_flag) {
            received.extraData = {id: this.uint8ArrayToNum(packet, 8, 2)};
        }

        return received;
    }

    bufferToHex(buffer) {
        return [...new Uint8Array (buffer)]
            .map (b => b.toString (16).padStart (2, "0"))
            .join (" ");
    }

    handleReceived(event) {
        // Convert data
        let value = new Uint8Array(event.target.value.buffer);
        let packetType = value[0];
        console.log("Received value (HEX): ", this.bufferToHex(value));

        // Proccess data
        switch(packetType) {
            case this.packetType.header:
                this.handleHeader(value);
                break;
            case this.packetType.data:
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
            case this.dataType.value:
            case this.dataType.string:
            case this.dataType.json:
                this.proccessStream(header.dataType, header.length);
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

            this.callback(this.packet);
        }
    }

    addCallback(func) {
        this.callback = func;
    }

    proccessStream(dataType, length) {
        this.receive.isInProgress = true;
        this.receive.buffer = new Uint8Array(2000);
        this.receive.pos = 0;
        this.receive.length = length;
        this.receive.dataType = dataType;
        // this.btn.response.innerHTML = 'In progess...';
    }

    getFlagSize(flags) {
        let size = 0;
        if(flags & this.flags.id_flag) {
            size += 2;
        } 
        return size;
    }

    async sendHeader(header) {
        if(this.isInProgress) {
            console.log("Cannot send packet! A transaction is already in progress.");
            return;
        }

        let length = this.sizes.unicom_header + this.getFlagSize(header.flags);
        let offset = 0;

        let data = new Int8Array(length);

        data[0] = this.packetType.header;
        data[1] = header.dataType;

        let flags = this.numToUint8Array(header.flags, 2);
        data[2] = flags[0];
        data[3] = flags[1];

        let lenArr = this.numToUint8Array(header.length, 4);
        data[4] = lenArr[0];
        data[5] = lenArr[1];
        data[6] = lenArr[2];
        data[7] = lenArr[3];

        offset += 8;

        if(header.flags & this.flags.id_flag) {
            data.set(this.numToUint8Array(header.data.id, 2), offset);
            offset += 2; // ID field size
        }

        console.log("Sending header (HEX): ", this.bufferToHex(data));
        await this.uniCom.char.writeValue(data);
    }

    async sendData(value) {
        let data = new Uint8Array(this.sizes.unicom_data_header + value.length);
        data[0] = this.packetType.data;
        data.set(value, this.sizes.unicom_data_header);
        await this.uniCom.char.writeValue(data);
    }

    copyExtraToHeader(header, extraData) {
        header.flags = extraData.flags;
        header.data.id = extraData.data.id;
    }

    async sendStream(data) {
        let pos = 0;

        while(pos < data.length) {
            let length = Math.min(this.payload_size, data.length-pos);
            let subarr = data.subarray(pos, pos+length);
            pos += length;
            await this.sendData(subarr);
        }
    }

    createHeader(dataType, length, extraData = null) {
        let header = {
            packetType : this.packetType.header,
            dataType : dataType,
            flags : this.flags.no_flag,
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
        await this.sendRawData(this.dataType.value, value, extraData);
    }

    async sendString(string, extraData = null) {
        console.log("Sending string...");
        await this.sendRawData(this.dataType.string, new TextEncoder().encode(string), extraData);
    }

    async sendJSON(object, extraData = null) {
        console.log("Sending json...");

        let serialized = JSON.stringify(object);
        await this.sendRawData(this.dataType.string, new TextEncoder().encode(serialized), extraData);
    }

    // async requestData(value) {
    //     if(!this.webble.isBleConnected()) {
    //         return false;
    //     }
    //     if(!this.uniCom.char) {
    //         return false;
    //     }

    //     try {
    //         console.log("Send Value:", value);
    //         await this.sendValue(new Uint8Array([value]));
    //     } catch(error) {
    //         console.error("Error requesting data: ", error);
    //     }
    // }

    bindButtons() {
        this.btn.btnR1.addEventListener('click', () => this.requestData(1));
        this.btn.btnR2.addEventListener('click', () => this.requestData(2));
        this.btn.btnR3.addEventListener('click', () => this.requestData(3));
        let extraData1 = {
            flags : this.flags.id_flag,
            data : { id : 123},
        };
        this.btn.btnS1.addEventListener('click', () => this.sendValue(new Uint8Array([10]), extraData1));
        this.btn.btnS2.addEventListener('click', () => this.sendString("1111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000"));
        // this.btn.btnS2.addEventListener('click', () => this.sendString("Hello BLE!"));
        let object = {
            username : 'Sany9',
            email : 'sanyika02@gmail.com',
            password : 'superS3cr3t'
        };
        this.btn.btnS3.addEventListener('click', () => this.sendJSON(object));
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

export default UniCom;
