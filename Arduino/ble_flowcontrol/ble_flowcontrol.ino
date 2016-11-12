#include "CurieBle.h"

static const char* bluetoothDeviceName = "MyDevice"; 

static const int   characteristicTransmissionLength = 20; 
static const char* bleReadReceiptMessage = "ready";
static const int   bleReadReceiptMessageLength = 5;

// store details about transmission here
struct BleTransmission {
  char data[characteristicTransmissionLength];
  unsigned int length;
  const char* uuid;
};
BleTransmission bleTransmissionData;
bool bleDataWritten = false; // true if data has been received

BLEService service("180C"); 
BLECharacteristic characteristic(
  "2A56",
  BLERead | BLENotify | BLEWrite, // read, write, notify
  characteristicTransmissionLength
);

BLEPeripheral blePeripheral; 

bool bleCharacteristicSubscribed = false; // true when a client subscribes


// When data is sent from the client, it is processed here inside a callback
// it is best to handle the result of this inside the main loop
void onBleCharacteristicWritten(BLECentral& central, BLECharacteristic &characteristic) {
  bleDataWritten = true;
  
  bleTransmissionData.uuid = characteristic.uuid();
  bleTransmissionData.length = characteristic.valueLength();
  
  // Since we are playing with strings, we must use strncpy
  strncpy(bleTransmissionData.data, (char*) characteristic.value(), characteristic.valueLength());
}

void setBleCharacteristicValue(char* output, int length) {
  characteristic.setValue((const unsigned char*) output, length); 
}

void setup() {
  Serial.begin(9600); // open a Serial connection
  while (!Serial); // wait for Serial console to open
  blePeripheral.setLocalName(bluetoothDeviceName); 

  blePeripheral.setAdvertisedServiceUuid(service.uuid()); 
  blePeripheral.addAttribute(service);
  blePeripheral.addAttribute(characteristic);

  // trigger onbleCharacteristicWritten when data is sent to the characteristic
  characteristic.setEventHandler(
    BLEWritten,
    onBleCharacteristicWritten
  );
  blePeripheral.begin(); 
}

void loop() {
  // if the bleDataWritten flag has been set, print out the incoming data
  if (bleDataWritten) {
    bleDataWritten = false; // ensure only happens once
    
    Serial.print(bleTransmissionData.length);
    Serial.print(" bytes sent to characteristic ");
    Serial.print(bleTransmissionData.uuid);
    Serial.print(": ");
    Serial.println(bleTransmissionData.data);

    // send out flow control message
    setBleCharacteristicValue((char*) bleReadReceiptMessage, bleReadReceiptMessageLength);

  }
  
}

