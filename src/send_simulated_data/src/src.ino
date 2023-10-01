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
const uint8_t slaveAddress = 1;

long gasRandNumber = 0;
long waterRandNumber = 0;
long powerRandNumber = 0;

void setup() {
    pinMode(DE_RE, OUTPUT);
    // Initialize the ModbusRTU communication with the specified Serial port
    Serial.begin(115200);

    // Set the baud rate to match your RS485 network configuration
    S.begin(9600, SWSERIAL_8N1);
    mb.begin(&S, DE_RE);
    mb.master();

    // Set the Modbus slave address
    mb.slave(slaveAddress);

    // if analog input pin 0 is unconnected, random analog
    // noise will cause the call to randomSeed() to generate
    // different seed numbers each time the sketch runs.
    // randomSeed() will then shuffle the random function.
    randomSeed(analogRead(0));
}

void loop() {
    digitalWrite(DE_RE, LOW);
    mb.writeHreg(slaveAddress, 0, &dataToSend[0], 3);  // Write to register 0
    Serial.print("gas data: ");
    Serial.println(dataToSend[0]);
    Serial.print("water data: ");
    Serial.println(dataToSend[1]);
    Serial.print("power data: ");
    Serial.println(dataToSend[2]);
    Serial.println("====================");
    mb.task();
    delay(100); // Delay before sending data again
    digitalWrite(DE_RE, HIGH);

    yield();

    // generate random numbers from 10 to 20
    gasRandNumber = random(10, 21);
    waterRandNumber = random(10, 21);
    powerRandNumber = random(10, 21);
    
    dataToSend[0] = dataToSend[0] + gasRandNumber;
    dataToSend[1] = dataToSend[1] + waterRandNumber;
    dataToSend[2] = dataToSend[2] + powerRandNumber;
    delay(1000);
}