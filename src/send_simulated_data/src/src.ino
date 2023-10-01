#include <ModbusRTU.h>
#include <SoftwareSerial.h>

#define DE_RE 2
#define RX 12
#define TX 13

SoftwareSerial S(RX, TX);

// Create a ModbusRTU object
ModbusRTU mb;

// Define your data to be sent as an array
uint16_t dataToSend[3] = {123, 234, 345};

// Define the Modbus slave address
const uint8_t slaveAddress = 1;  // Change this to the address of your Modbus slave

float InttoFloat(uint16_t Data0,uint16_t Data1) {
    float x;
    unsigned long *p;
    p = (unsigned long*)&x;
    *p = (unsigned long)Data0 << 16 | Data1; //Big-endian
    return(x);
}

bool cbWrite(Modbus::ResultCode event, uint16_t transactionId, void* data) {
    Serial.printf_P("Request result: 0x%02X, Mem: %d\n", event, ESP.getFreeHeap());
    return true;
}

void setup() {
    pinMode(DE_RE, OUTPUT);
    // Initialize the ModbusRTU communication with the specified Serial port
    Serial.begin(115200);  // Set the baud rate to match your RS485 network configuration
    S.begin(9600, SWSERIAL_8N1);
    mb.begin(&S, DE_RE);
    mb.master();

    // Set the Modbus slave address
    mb.slave(slaveAddress);
}

void loop() {
    digitalWrite(DE_RE, LOW);
    // Send data to the Modbus slave using the "mb.writeHreg" function
    // Serial.println("slaveAddress, 0, dataToSend[0]");
    // mb.writeHreg(slaveAddress, 0, dataToSend[0]);  // Write to register 0
    // Serial.println(dataToSend[0]);
    // Serial.println("=====dataToSend[0] ended=====");
    // mb.task(); delay(1000); yield();
    Serial.println("slaveAddress, 0, &dataToSend[0], 3");
    mb.writeHreg(slaveAddress, 0, &dataToSend[0], 3);  // Write to register 0
    Serial.println("=====dataToSend[0 , 1 , 2] ended=====");

    mb.task();

    // Delay before sending data again (adjust as needed)
    delay(1000);
    yield();
    digitalWrite(DE_RE, HIGH);
    delay(500);
}