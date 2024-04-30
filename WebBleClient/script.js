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
        // Get site dom
        this.btn = webBLE.btn;

        // Unicom service
        this.uniCom = {
            serviceUUID : '19b20000-e8f2-537e-4f6c-d104768a1214',
            service : null,
            charUUID : '19b20001-e8f2-537e-4f6c-d104768a1214',
            char : null,
        };

        this.progressType = {
            idle : 0,
            sending : 1,
            receiving : 2
        };

        this.send = {
            buffer : new Uint8Array(2000),
            isInProgress : false,
            pos : 0,
        };

        this.receive = {
            buffer : new Uint8Array(2000),
            isInProgress : false,
            count : 0,
            counter : 0
        };

        this.buffer = '';
        this.counter = 0;
        this.isInProgress = this.progressType.idle;
        this.data_size = 497 - 1; // UniCom Header size is 1

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

    pack(str, pos, size) {
        if(pos + size < str.length) {
            return "O" + str.substr(pos, size);
        } else {
            return "X" + str.substr(pos, size);
        }
    }

    bufferToHex(buffer) {
        return [...new Uint8Array (buffer)]
            .map (b => b.toString (16).padStart (2, "0"))
            .join (" ");
    }

    handleReceived(event) {
        if(this.isInProgress == this.progressType.sending) {
            console.log("Cannot receive value! ")
        }
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
        switch(received.header.dataType) {
            case this.dataType.value:
                this.proccessValue(received.data);
                break;
            case this.dataType.string:
                this.proccessString();
                break;
            case this.dataType.json:
                this.proccessJSON();
                break;
            default:
                console.log("Error: Unknown data type!");

        }
    }

    handleExtData(value) {
        this.buffer += value.subarray(4);
        this.counter += 1;
        this.btn.response.innerHTML = this.uniCom.buffer;
        this.uniCom.buffer = '';
    }

    proccessValue(data) {
        this.buffer = data;
        this.btn.response.innerHTML = this.buffer;
    }

    proccessString() {
        this.buffer = '';
        this.counter = 0;
        this.isInProgress = true;
    }

    proccessJSON() {
        this.buffer = '';
        this.counter = 0;
        this.isInProgress = true;
    }

    async sendValue(value) {
        console.log("Sending value...");
        if(this.isInProgress != this.progressType.idle) {
            console.log("Cannot send value! A transaction is already in progress!");
            return;
        }
        this.isInProgress = this.progressType.sending;

        let data = new Int8Array(4+value.length);
        data[0] = this.packetType.data;
        data[1] = this.dataType.value;
        data[2] = this.flags.no_flag;
        data[3] = 0;
        data.set(value, 4);
        console.log("Sending value (HEX): ", this.bufferToHex(data));

        await this.uniCom.char.writeValue(data);

        this.isInProgress = this.progressType.idle;
    }

    async sendString(string) {
        console.log("Sending string...");
        if(this.isInProgress != this.progressType.idle) {
            console.log("Cannot send string! A transaction is already in progress!");
            return;
        }
        this.isInProgress = this.progressType.sending;

        let pos = 0;
        let data_size = 100; // this.data_size for original

        let data = new Int8Array(4);
        data[0] = this.packetType.data;
        data[1] = this.dataType.string;
        data[2] = this.flags.no_flag;
        data[3] = Math.ceil(string.length/data_size);
        await this.uniCom.char.writeValue(data);

        while(pos < string.length) {
            console.log("Sending packet...");

            let length = Math.min(data_size, string.length-pos);
            let segment = new Uint8Array(length + 1);
            segment[0] = this.packetType.extData;
            segment.set(new TextEncoder().encode(string.substr(pos, length)), 1);
            pos += length;
            await this.uniCom.char.writeValue(segment);
        }

        this.isInProgress = this.progressType.idle;
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
        this.btn.btnS2.addEventListener('click', () => this.sendString("Hello BLE!"));
        // this.btn.btnS3.addEventListener('click', () => this.requestData(1));
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
