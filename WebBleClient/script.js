class RNTService {
    constructor(webBLE) {
        // Get site dom
        this.dom = webBLE.dom;

        // RNT service
        this.rnt = {
            serviceUUID : '19b10000-e8f2-537e-4f6c-d104768a1214',
            service : null,
            counterCharUUID: '19b10001-e8f2-537e-4f6c-d104768a1214',
            counterChar : null,
            ledCharUUID : '19b10002-e8f2-537e-4f6c-d104768a1214',
            ledChar : null,
        };
    }

    async getService(bleServer) {
        try {
            this.rnt.service = await bleServer.getPrimaryService(this.rnt.serviceUUID)
            console.log("Service discovered:", this.rnt.service.uuid);
            this.initRNTcounter();
            this.initRNTled();
        } catch(error) {
            console.log('Error: ', error);
        }
    }

    getServiceUUID() {
        return this.rnt.serviceUUID;
    }

    // Get Counter Characteristics and Enable Notifications
    async initRNTcounter() {
        try {
            this.rnt.counterChar = await this.rnt.service.getCharacteristic(this.rnt.counterCharUUID);
            console.log("Characteristic discovered:", this.rnt.counterChar.uuid);
            this.rnt.counterChar.addEventListener('characteristicvaluechanged', this.handleCounter.bind(this));
            this.rnt.counterChar.startNotifications();
            console.log("Notifications Started.");
    
            // Read current value
            //! Exception! If the characteristics doesn't load fast enough, error will be generated!
            let value = await this.rnt.counterChar.readValue();
            console.log("Read value: ", value);
            const decodedValue = new TextDecoder().decode(value);
            console.log("Decoded value: ", decodedValue);
            this.dom.retrievedValue.innerHTML = decodedValue;
            this.dom.timestampContainer.innerHTML = getDateTime();
        } catch(error) {
            console.log('Error: ', error);
        }
    }

    async initRNTled() {
        try {
            this.rnt.ledChar = await this.rnt.service.getCharacteristic(this.rnt.ledCharUUID)
            console.log("Found the LED characteristic: ", this.rnt.ledChar.uuid);
        } catch(error) {
            console.error("Error: ", error);
        }
    }

    handleCounter(event){
        const newValueReceived = new TextDecoder().decode(event.target.value);
        console.log("Characteristic value changed: ", newValueReceived);
        this.dom.retrievedValue.innerHTML = newValueReceived;
        this.dom.timestampContainer.innerHTML = getDateTime();
    }

    // RNT led write function
    async writeLED(value){
        if(!webBLE.isBleConnected()) {
            return false;
        }
        if(!this.rnt.ledChar) {
            return false;
        }

        const data = new Uint8Array([value]);

        try {
            await this.rnt.ledChar.writeValue(data);
            this.dom.latestValueSent.innerHTML = value;
            console.log("Value written to LED characteristic:", value);
        } catch (error) {
            console.error("Error writing to the LED characteristic: ", error);
        }
    }

    bindButton(onButton, offButton) {
        onButton.addEventListener('click', () => this.writeLED(1));
        offButton.addEventListener('click', () => this.writeLED(0));
        this.dom.fetchOn.addEventListener('click', () => this.rnt.counterChar?.startNotifications());
        this.dom.fetchOff.addEventListener('click', () => this.stop());
    }

    stop() {
        if(this.rnt.counterChar) {
            this.rnt.counterChar.stopNotifications();
            console.log("RNT notifications stopped.");
        }
    }
};

class UniCom {
    constructor(webBLE) {
        // "Enums"
        this.packetType = {
            data : 0x0D,
            extData : 0xED
        };

        this.dataType = {
            value : 0x10,
            string : 0x20,
            json : 0x30
        };

        this.flags = {
            no_flag : 0,
            id_flag : 1,
            len_flag : 2
        };

        // Get site dom
        this.btn = webBLE.btn;

        // Unicom service
        this.uniCom = {
            serviceUUID : '19b20000-e8f2-537e-4f6c-d104768a1214',
            service : null,
            charUUID : '19b20001-e8f2-537e-4f6c-d104768a1214',
            char : null,
        };

        this.send = {
            buffer : new Uint8Array(2000),
            isInProgress : false,
            pos : 0,
        };

        this.receive = {
            dataType : 0,
            buffer : new Uint8Array(2000),
            isInProgress : false,
            pos : 0,
            count : 0,
            counter : 0
        };

        // this.data_size = 497 - 1; // UniCom Header size is 1
        this.data_size = 99; // UniCom Header size is 1
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
        } catch(error) {
            console.error("Error: ", error);
        }
    }

    unpack(packet) {
        return {
            header : {
                packetType : packet[0],
                dataType : packet[1],
                flags : packet[2],
                count : packet[3]
            },
            data : packet.subarray(4)
        };
    }

    bufferToHex(buffer) {
        return [...new Uint8Array (buffer)]
            .map (b => b.toString (16).padStart (2, "0"))
            .join (" ");
    }

    handleReceived(event) {
        // Convert data
        let value = new Uint8Array(event.target.value.buffer);
        let received = this.unpack(value);
        if(received.header.count > 0) {

        }
        let packetType = value[0];
        console.log("Received value (HEX): ", this.bufferToHex(value));

        // Proccess data
        switch(packetType) {
            case this.packetType.data:
                this.handleData(value);
                break;
            case this.packetType.extData:
                this.handleExtData(value);
                break;
            default:
                console.log("Error: Unknown packet type!");
        }
    }

    handleData(value) {
        let received = this.unpack(value);
        let header = received.header;
        switch(received.header.dataType) {
            case this.dataType.value:
                this.proccessValue(received.data);
                break;
            case this.dataType.string:
                this.proccessStream(header.dataType, header.count);
                break;
            case this.dataType.json:
                this.proccessStream(header.dataType, header.count);
                break;
            default:
                console.log("Error: Unknown data type!");
        }
    }

    handleExtData(value) {
        console.log("Received ext data...");

        let subarr = value.subarray(1);
        this.receive.buffer.set(subarr, this.receive.pos);
        this.receive.pos += subarr.length;

        this.receive.counter += 1;
        if(this.receive.count == this.receive.counter) {
            this.receive.buffer = this.receive.buffer.subarray(0, this.receive.pos);

            switch(this.receive.dataType) {
                case this.dataType.string:
                    this.btn.response.innerHTML = new TextDecoder().decode(this.receive.buffer);
                    break;
                case this.dataType.json:
                    let jsonString = new TextDecoder().decode(this.receive.buffer);
                    let json = JSON.parse(jsonString);
                    let jsonPretty = JSON.stringify(json, null, 2);
                    this.btn.response.innerHTML = jsonPretty;
                    console.log(json);
                    break;
                default:
                    console.log("Unkown data type received");
                    console.log(this.bufferToHex(this.receive.buffer));
            }
        }
    }

    proccessValue(data) {
        this.receive.buffer = data;
        this.btn.response.innerHTML = this.receive.buffer;
    }

    proccessStream(dataType, count) {
        this.receive.dataType = dataType;
        this.receive.buffer = new Uint8Array(2000);
        this.receive.pos = 0;
        this.receive.count = count;
        this.receive.counter = 0;
        this.receive.isInProgress = true;
        this.btn.response.innerHTML = 'In progess...';
    }

    async sendPacket(header, value = null) {
        if(this.isInProgress) {
            console.log("Cannot send packet! A transaction is already in progress.");
            return;
        }

        let length = (value == null) ? 4 : 4+value.length;

        let data = new Int8Array(length);
        data[0] = this.packetType.data;
        data[1] = header.dataType;
        data[2] = header.flags;
        data[3] = header.count;
        if(value != null) {
            data.set(value, 4);
        }

        console.log("Sending value (HEX): ", this.bufferToHex(data));
        await this.uniCom.char.writeValue(data);
    }

    async sendExtPacket(value) {
        let data = new Uint8Array(1 + value.length);
        data[0] = this.packetType.extData;
        data.set(value, 1);
        await this.uniCom.char.writeValue(data);
    }

    async sendValue(value) {
        console.log("Sending value...");

        if(this.send.isInProgress) {
            console.log("Cannot send value! A transaction is already in progress!");
            return;
        }

        this.send.isInProgress = true;

        let header = {
            dataType : this.dataType.value,
            flags : this.flags.no_flag,
            count : 0,
        };

        await this.sendPacket(header, value);

        this.send.isInProgress = false;
    }

    async sendStream(data) {
        let pos = 0;

        while(pos < data.length) {
            let length = Math.min(this.data_size, data.length-pos);
            let subarr = data.subarray(pos, pos+length);
            pos += length;
            await this.sendExtPacket(subarr);
        }
    }

    async sendString(string) {
        console.log("Sending string...");

        if(this.isInProgress) {
            console.log("Cannot send string! A transaction is already in progress!");
            return;
        }

        this.send.isInProgress = true;

        let header = {
            dataType : this.dataType.string,
            flags : this.flags.no_flag,
            count : Math.ceil(string.length/this.data_size),
        };

        await this.sendPacket(header);
        await this.sendStream(new TextEncoder().encode(string));

        this.send.isInProgress = false;
    }

    async sendJSON(object) {
        console.log("Sending json...");

        this.send.isInProgress = true;

        let serialized = JSON.stringify(object);

        let header = {
            dataType : this.dataType.json,
            flags : this.flags.no_flag,
            count : Math.ceil(serialized.length/this.data_size),
        };

        
        await this.sendPacket(header);
        await this.sendStream(new TextEncoder().encode(serialized));

        this.send.isInProgress = false;
    }

    async requestData(value) {
        if(!webBLE.isBleConnected()) {
            return false;
        }
        if(!this.uniCom.char) {
            return false;
        }

        try {
            console.log("Send Value:", value);
            await this.sendValue(new Uint8Array([value]));
        } catch(error) {
            console.error("Error requesting data: ", error);
        }
    }

    bindButtons() {
        this.btn.btnR1.addEventListener('click', () => this.requestData(1));
        this.btn.btnR2.addEventListener('click', () => this.requestData(2));
        this.btn.btnR3.addEventListener('click', () => this.requestData(3));
        this.btn.btnS1.addEventListener('click', () => this.sendValue(new Uint8Array([10])));
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

class MyPasswordManager {

};

class WebBLE {
    constructor() {
        // Variables to Handle Bluetooth
        this.ble = {
            deviceName :'ESP32',
            device : null,
            server : null,
            att_data : 497, // Max MTU - Max ATT Header = 512 - 15 = 497
        };

        this.getDomElements();

        // RNT Service
        this.rntService = new RNTService(this);

        // UniCom service
        this.unicomService = new UniCom(this);
        
        this.createButtons();
    }

    getDomElements() {
        // DOM Elements
        this.dom = {
            connectButton : document.getElementById('connectBleButton'),
            disconnectButton : document.getElementById('disconnectBleButton'),
            onButton : document.getElementById('onButton'),
            offButton : document.getElementById('offButton'),
            fetchOn : document.getElementById('fetchOn'),
            fetchOff : document.getElementById('fetchOff'),
            retrievedValue : document.getElementById('valueContainer'),
            latestValueSent : document.getElementById('valueSent'),
            bleStateContainer : document.getElementById('bleState'),
            timestampContainer : document.getElementById('timestamp'),
        };
        // Request and Response
        this.btn = {
            btnR1 : document.getElementById('btnR1'),
            btnR2 : document.getElementById('btnR2'),
            btnR3 : document.getElementById('btnR3'),
            btnS1 : document.getElementById('btnS1'),
            btnS2 : document.getElementById('btnS2'),
            btnS3 : document.getElementById('btnS3'),
            response : document.getElementById('response'),
        }
    }

    createButtons() {
        // Connect Button (search for BLE Devices only if BLE is available)
        this.dom.connectButton.addEventListener('click', this.connectToDevice.bind(this));

        // Disconnect Button
        this.dom.disconnectButton.addEventListener('click', this.disconnectDevice.bind(this));

        // Write to the ESP32 LED Characteristic
        this.rntService.bindButton(this.dom.onButton, this.dom.offButton);

        // Request data
        this.unicomService.bindButtons();
    }

    // Check if BLE is available in your Browser
    isWebBluetoothEnabled() {
        if(!navigator.bluetooth) {
            console.log('Web Bluetooth API is not available in this browser!');
            this.dom.bleStateContainer.innerHTML = "Web Bluetooth API is not available in this browser/device!";
            return false;
        }
        console.log('Web Bluetooth API supported in this browser.');
        return true;
    }

    isBleConnected() {
        if(this.ble.server && this.ble.server.connected) {
            return true;
        } else {
            console.error ("Bluetooth is not connected.");
            window.alert("Bluetooth is not connected!");
            return false;
        }
    }

    // Connect to BLE Device
    async connectToDevice() {
        if(!this.isWebBluetoothEnabled()){
            return false;
        }

        try {
            console.log('Initializing Bluetooth...');
            this.ble.device = await navigator.bluetooth.requestDevice({
                filters: [{namePrefix: this.ble.deviceName}],
                //filters: [{name: this.rnt.deviceName}],
                optionalServices: [
                    this.rntService.getServiceUUID(),
                    this.unicomService.getServiceUUID()
                ]
            });
            console.log('Device Selected:', this.ble.device.name);
    
            this.dom.bleStateContainer.innerHTML = 'Connected to device ' + this.ble.device.name;
            this.dom.bleStateContainer.style.color = "#24af37";
            this.ble.device.addEventListener('gattservicedisconnected', this.onDisconnected.bind(this));
    
            this.ble.server = await this.ble.device.gatt.connect();
            console.log("Connected to GATT Server");

            this.rntService.getService(this.ble.server);
            this.unicomService.getService(this.ble.server);
        } catch(error) {
            console.log('Error: ', error);
        }
    }

    async disconnectDevice() {
        if(!this.isBleConnected()) {
            return false;
        }

        console.log("Disconnect Device.");

        this.rntService.stop();
        this.unicomService.stop();

        try {
            await this.ble.server.disconnect();
            console.log("Device Disconnected");
            this.dom.bleStateContainer.innerHTML = "Device Disconnected";
            this.dom.bleStateContainer.style.color = "#d13a30";
        } catch(error) {
            console.log("An error occurred:", error);
        }
    }

    onDisconnected(event) {
        console.log('Device Disconnected:', event.target.device.name);
        this.dom.bleStateContainer.innerHTML = "Device disconnected";
        this.dom.bleStateContainer.style.color = "#d13a30";
    }

}

function getDateTime() {
    var currentdate = new Date();
    var day = ("00" + currentdate.getDate()).slice(-2); // Convert day to string and slice
    var month = ("00" + (currentdate.getMonth() + 1)).slice(-2);
    var year = currentdate.getFullYear();
    var hours = ("00" + currentdate.getHours()).slice(-2);
    var minutes = ("00" + currentdate.getMinutes()).slice(-2);
    var seconds = ("00" + currentdate.getSeconds()).slice(-2);

    var datetime = year + "/" + month + "/" + day + " at " + hours + ":" + minutes + ":" + seconds;
    return datetime;
}

let webBLE = new WebBLE;
