#include <RF24.h>

#define CE 9
#define CSN 10
#define LED 2
#define DEVICE_ID 1

RF24 radio(CE, CSN);
const byte address[10] = "minion";

struct minionPayload
{
    int16_t minionId;
    int16_t minionStatus;
} payload;

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

    if (radio.available())
    {
        Serial.println("Received a message :)");
        radio.read(&payload, sizeof(payload));
        char buffer[100];
        sprintf(buffer, "Received status: id: %d, status: %d", payload.minionId, payload.minionStatus);
        Serial.println(buffer);
        handlePayload(&payload);
    }
    else
    {
        Serial.println("Nothing in the air :-(");
    }
    delay(1000);
}

void handlePayload(minionPayload *payload)
{
    if (payload->minionId != DEVICE_ID)
    {
        Serial.println("Not meant for me. Ignoring...");
        return;
    }
    if (payload->minionStatus == 1)
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