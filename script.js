class WebBLE {
    constructor() {
        // Variables to Handle Bluetooth
        this.bleHandle = {
            bleServer : null,
            bleServiceFound : null,
            sensorCharacteristicFound : null,
        };

        // Define BLE Device Specs
        this.bleSpecs = {
            deviceName :'ESP32',
            bleService : '19b10000-e8f2-537e-4f6c-d104768a1214',
            ledCharacteristic : '19b10002-e8f2-537e-4f6c-d104768a1214',
            sensorCharacteristic: '19b10001-e8f2-537e-4f6c-d104768a1214',
        };

        this.getDomElements();
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
    }

    createButtons() {
        // Connect Button (search for BLE Devices only if BLE is available)
        this.dom.connectButton.addEventListener('click', () => {
            if(this.isWebBluetoothEnabled()){
                this.connectToDevice();
            }
        });

        // Disconnect Button
        this.dom.disconnectButton.addEventListener('click', this.disconnectDevice.bind(this));

        // Write to the ESP32 LED Characteristic
        this.dom.onButton.addEventListener('click', () => this.writeOnCharacteristic(1).bind(this));
        this.dom.offButton.addEventListener('click', () => this.writeOnCharacteristic(0).bind(this));
    }

    isWebBluetoothEnabled() {
        // Check if BLE is available in your Browser
        if(!navigator.bluetooth) {
            console.log('Web Bluetooth API is not available in this browser!');
            this.dom.bleStateContainer.innerHTML = "Web Bluetooth API is not available in this browser/device!";
            return false;
        }
        console.log('Web Bluetooth API supported in this browser.');
        return true;
    }

    // Connect to BLE Device and Enable Notifications
    connectToDevice(){
        console.log('Initializing Bluetooth...');
        navigator.bluetooth.requestDevice({
            filters: [{name: this.bleSpecs.deviceName}],
            optionalServices: [this.bleSpecs.bleService]
        })
        .then(device => {
            console.log('Device Selected:', device.name);
            this.dom.bleStateContainer.innerHTML = 'Connected to device ' + device.name;
            this.dom. bleStateContainer.style.color = "#24af37";
            device.addEventListener('gattservicedisconnected', this.onDisconnected.bind(this));
            return device.gatt.connect();
        })
        .then(gattServer =>{
            console.log("Connected to GATT Server");
            this.bleHandle.bleServer = gattServer;
            return this.bleHandle.bleServer.getPrimaryService(this.bleSpecs.bleService);
        })
        .then(service => {
            console.log("Service discovered:", service.uuid);
            this.bleHandle.bleServiceFound = service;
            return service.getCharacteristic(this.bleSpecs.sensorCharacteristic);
        })
        .then(characteristic => {
            console.log("Characteristic discovered:", characteristic.uuid);
            this.bleHandle.sensorCharacteristicFound = characteristic;
            characteristic.addEventListener('characteristicvaluechanged', this.handleCharacteristicChange.bind(this));
            characteristic.startNotifications();
            console.log("Notifications Started.");
            return characteristic.readValue();
        })
        .then(value => {
            console.log("Read value: ", value);
            const decodedValue = new TextDecoder().decode(value);
            console.log("Decoded value: ", decodedValue);
            this.dom.retrievedValue.innerHTML = decodedValue;
            this.dom.timestampContainer.innerHTML = getDateTime();
        })
        .catch(error => {
            console.log('Error: ', error);
        })
    }

    onDisconnected(event){
        console.log('Device Disconnected:', event.target.device.name);
        this.dom.bleStateContainer.innerHTML = "Device disconnected";
        this.dom.bleStateContainer.style.color = "#d13a30";
    
        connectToDevice();
    }
    
    handleCharacteristicChange(event){
        const newValueReceived = new TextDecoder().decode(event.target.value);
        console.log("Characteristic value changed: ", newValueReceived);
        this.dom.retrievedValue.innerHTML = newValueReceived;
        this.dom.timestampContainer.innerHTML = getDateTime();
    }

    writeOnCharacteristic(value){
        if(this.bleHandle.bleServer && this.bleHandle.bleServer.connected) {
            this.bleHandle.bleServiceFound.getCharacteristic(this.bleSpecs.ledCharacteristic)
            .then(characteristic => {
                console.log("Found the LED characteristic: ", characteristic.uuid);
                const data = new Uint8Array([value]);
                return characteristic.writeValue(data);
            })
            .then(() => {
                this.dom.latestValueSent.innerHTML = value;
                console.log("Value written to LEDcharacteristic:", value);
            })
            .catch(error => {
                console.error("Error writing to the LED characteristic: ", error);
            });
        } else {
            console.error ("Bluetooth is not connected. Cannot write to characteristic.")
            window.alert("Bluetooth is not connected. Cannot write to characteristic. \n Connect to BLE first!")
        }
    }

    disconnectDevice() {
        console.log("Disconnect Device.");
        if(this.bleHandle.bleServer && this.bleHandle.bleServer.connected) {
            if(this.bleHandle.sensorCharacteristicFound) {
                this.bleHandle.sensorCharacteristicFound.stopNotifications()
                    .then(() => {
                        console.log("Notifications Stopped");
                        return this.bleHandle.bleServer.disconnect();
                    })
                    .then(() => {
                        console.log("Device Disconnected");
                        this.dom.bleStateContainer.innerHTML = "Device Disconnected";
                        this.dom.bleStateContainer.style.color = "#d13a30";
    
                    })
                    .catch(error => {
                        console.log("An error occurred:", error);
                    });
            } else {
                console.log("No characteristic found to disconnect.");
            }
        } else {
            // Throw an error if Bluetooth is not connected
            console.error("Bluetooth is not connected.");
            window.alert("Bluetooth is not connected.")
        }
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
