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
            buffer : '',
            packet_data : 497 - 1, // UniCom Header size is 1
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
            type : packet.charAt(0),
            data : packet.slice(1 - packet.length)
        };
    }

    bufferToHex(buffer) {
        return [...new Uint8Array (buffer)]
            .map (b => b.toString (16).padStart (2, "0"))
            .join (" ");
    }

    handleReceived(event) {
        console.log("Received value (HEX): ", this.bufferToHex(event.target.value.buffer));
        let received = new TextDecoder().decode(event.target.value);
        console.log("Received value: ", received);
        received = this.unpack(received);
        switch(received.type) {
            case 'O':
                this.uniCom.buffer += received.data;
                this.btn.response.innerHTML = 'In progress...';
                break;
            case 'X':
                this.uniCom.buffer += received.data
                this.btn.response.innerHTML = this.uniCom.buffer;
                this.uniCom.buffer = '';
                break;
            default:
                console.log("Error: Unknown packet header!");
        }
    }

    pack(str, pos, size) {
        if(pos + size < str.length) {
            return "O" + str.substr(pos, size);
        } else {
            return "X" + str.substr(pos, size);
        }
    }

    async sendString(string) {
        let pos = 0;
        // let data_size = this.uniCom.packet_data;
        let data_size = 100;
        let segment;

        while(pos < string.length) {
            console.log("Sending packet...");
            segment = this.pack(string, pos, data_size);
            pos += data_size;
            await this.uniCom.char.writeValue(new TextEncoder().encode(segment));
        }
    }

    async requestData(value) {
        if(!webBLE.isBleConnected()) {
            return false;
        }
        if(!this.uniCom.char) {
            return false;
        }

        const str = "1111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000";
        try {
            console.log("Send Value:", value);
            await this.sendString(String.fromCharCode(value));
            console.log("Send string: ", str);
            await this.sendString(str);
        } catch(error) {
            console.error("Error requesting data: ", error);
        }
    }

    bindButtons() {
        this.btn.btnA.addEventListener('click', () => this.requestData(1));
        this.btn.btnB.addEventListener('click', () => this.requestData(2));
        this.btn.btnC.addEventListener('click', () => this.requestData(3));
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
            retrievedValue : document.getElementById('valueContainer'),
            latestValueSent : document.getElementById('valueSent'),
            bleStateContainer : document.getElementById('bleState'),
            timestampContainer : document.getElementById('timestamp'),
        };
        // Request and Response
        this.btn = {
            btnA : document.getElementById('btnA'),
            btnB : document.getElementById('btnB'),
            btnC : document.getElementById('btnC'),
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