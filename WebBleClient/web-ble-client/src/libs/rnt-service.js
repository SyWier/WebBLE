class RNTService {
    constructor(webBLE) {
        this.webBLE = webBLE;

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
    }

    // RNT led write function
    async writeLED(value){
        if(!this.webBLE.isBleConnected()) {
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
