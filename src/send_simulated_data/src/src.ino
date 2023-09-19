#include <ModbusRTU.h>
#include <SoftwareSerial.h>

#define DE_RE 2
#define RX 12
#define TX 13

SoftwareSerial S(RX, TX);

// Create a ModbusRTU object
ModbusRTU mb;

// Define your data to be sent as an array
uint16_t dataToSend[] = {100, 200};

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
  mb.writeHreg(slaveAddress, 1, dataToSend[1], 0);  // Write to register 0
  // mb.writeHreg(slaveAddress, 2, dataToSend[1], 0);  // Write to register 1

  if (!mb.slave()) {
    mb.writeHreg(slaveAddress, 2699, dataToSend[0], 0);
    // mb.writeHreg(1, 2699, dataToSend[0], 2);
    Serial.println(InttoFloat(dataToSend[0],  dataToSend[1]));
  }
  mb.task();

  // Delay before sending data again (adjust as needed)
  delay(1000);

  yield();
}