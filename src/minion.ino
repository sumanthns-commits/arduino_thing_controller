#include <RF24.h>

#define CE 9
#define CSN 10
#define RELAY 2

const char *MINION_ID = "r2F9p4RzR1"; // MINION_ID registered
const byte address[10] = "minion";

RF24 radio(CE, CSN);

struct MinionPayload
{
    char minionId[11];
    char status[4];
};

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

    pinMode(RELAY, OUTPUT);
    digitalWrite(RELAY, HIGH); // Keep Relay off by default
}

void loop()
{
    radio.startListening();
    Serial.println("Listening....");

    if (radio.available())
    {
        Serial.println("Received a message :)");
        MinionPayload payload;
        radio.read(&payload, sizeof(payload));
        char buffer[100];
        sprintf(buffer, "Received payload: id: %s, status: %s", payload.minionId, payload.status);
        Serial.println(buffer);
        handlePayload(&payload);
    }
    else
    {
        Serial.println("Nothing in the air :-(");
    }
    delay(1000);
}

void handlePayload(MinionPayload *payload)
{
    if (strcmp(payload->minionId, MINION_ID) != 0)
    {
        Serial.println("Not meant for me. Ignoring...");
        return;
    }

    // RELAY turns on with a LOW. Turns off with a HIGH
    if (strcmp(payload->status, "on") == 0)
    {
        Serial.println("Turning RELAY on.");
        digitalWrite(RELAY, LOW);
    }
    else if (strcmp(payload->status, "off") == 0)
    {
        Serial.println("Turning RELAY off.");
        digitalWrite(RELAY, HIGH);
    }
    else
    {
        Serial.println("Unknown status. Ignoring...");
    }
}