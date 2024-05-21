class WebBLE {
    constructor(deviceName = "ESP32") {
        console.log("Creating WebBLE...");

        // Variables to Handle Bluetooth
        this.deviceName = deviceName;
        this.device = null;
        this.server = null;
        this.att_data = 497 // Max MTU - Max ATT Header = 512 - 15 = 497

        // Services to work with
        this.services = [];
        this.serviceUUIDs = [];

        // Status change callback
        this.callback = null;
    }

    // Check if BLE is available in your Browser
    isWebBluetoothEnabled() {
        if(!navigator.bluetooth) {
            console.log('Web Bluetooth API is not available in this browser!');
            window.alert("Web Bluetooth API is not available in this browser!");
            return false;
        }
        console.log('Web Bluetooth API supported in this browser.');
        return true;
    }

    // Check if BLE is already connected
    isBleConnected() {
        if(this.server && this.server.connected) {
            return true;
        } else {
            console.error("Device is not connected!");
            return false;
        }
    }

    // Connect to BLE Device
    async connectToDevice() {
        if(!this.isWebBluetoothEnabled()){
            return;
        }
        if(this.isBleConnected()) {
            console.error("Device already connected!");
            window.alert("Device already connected!");
            return;
        }

        try {
            console.log('Initializing Bluetooth...');

            // Connect to device
            this.device = await navigator.bluetooth.requestDevice({
                //filters: [{namePrefix: this.deviceName}],
                filters: [{ services: this.serviceUUIDs }],
            });
            console.log('Device Selected:', this.device.name);

            // Bind onDisconnected event listener
            this.device.addEventListener('gattserverdisconnected', this.onDisconnected.bind(this));
    
            // Connect to the gatt server
            this.server = await this.device.gatt.connect();
            console.log("Connected to GATT Server");
            this.callback(true);

            // Get all the added services
            this.services.forEach(service => service.getService(this.server));
        } catch(error) {
            console.log('Error: ', error);
        }

        
    }

    // Disconnect from BLE device
    async disconnectDevice() {
        if(!this.isBleConnected()) {
            window.alert("Bluetooth is not connected!");
            return;
        }

        console.log("Disconnect Device...");

        try {
            // Stop all services
            await Promise.all(this.services.map(async (service) => {
                await service.stop();
            }));

            // Disconnect from device
            await this.server.disconnect();
        } catch(error) {
            console.log("An error occurred:", error);
        }
    }

    // Function that is called when the device disconnects
    onDisconnected(event) {
        this.server = null;
        console.log('Device Disconnected.');
        this.callback(false);
    }

    // Add a service to WebBLE to discover and handle
    addService(service) {
        this.services.push(service);
        this.serviceUUIDs.push(service.getServiceUUID());
        console.log("Adding service with UUID:", service.getServiceUUID());
    }

    // Add callback for 
    addCallback(func) {
        this.callback = func;
    }
};

export default WebBLE;
