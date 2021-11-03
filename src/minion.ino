#include <RF24.h>

#define CE 9
#define CSN 10
#define LED 2

RF24 radio(CE, CSN);
const uint64_t address = 0xE6E6E6E6E6E6;

void setup()
{
    Serial.begin(115200); // Start the Serial communication to send messages to the computer
    delay(10);
    while (!Serial)
    {
    };

    // initialize the transceiver on the SPI bus
    if (!radio.begin())
    {
        Serial.println(F("radio hardware is not responding!!"));
        while (1)
        {
        } // hold in infinite loop
    }
    radio.setPALevel(RF24_PA_LOW);
    radio.openReadingPipe(1, address);

    pinMode(LED, OUTPUT);
}

void loop()
{
    radio.startListening();
    Serial.println("Listening....");
    // char status[10];
    if (radio.available())
    {
        while (radio.available())
        {
            Serial.println("Received a message :)");
            // uint8_t bytes = radio.getPayloadSize();
            int statusInt;
            radio.read(&statusInt, sizeof(statusInt));
            char buffer[100];
            sprintf(buffer, "Received status: %d", statusInt);
            Serial.println(buffer);
            handleStatus(statusInt);
        }
    }
    else
    {
        Serial.println("Nothing in the air :-(");
    }
    delay(1000);
}

void handleStatus(int statusInt)
{
    if (statusInt == 1)
    {
        Serial.println("Turning LED on.");
        digitalWrite(LED, HIGH);
    }
    else
    {
        Serial.println("Turning LED off.");
        digitalWrite(LED, LOW);
    }
}