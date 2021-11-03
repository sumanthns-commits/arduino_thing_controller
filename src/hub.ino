#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <RF24.h>

#define CE D4
#define CSN D2

const char *ssid = "";     // The SSID (name) of the Wi-Fi network you want to connect to
const char *password = ""; // The password of the Wi-Fi network
const char *host = "617e775c2ff7e600174bd7ed.mockapi.io";
const int httpsPort = 443;
const char fingerprint[] PROGMEM = "d82078fa397475dd82bd517b4efacde4ba00b4d7";
const String thingID = "1";
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
    radio.stopListening();

    WiFi.begin(ssid, password); // Connect to the network
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println(" ...");

    int i = 0;
    while (WiFi.status() != WL_CONNECTED)
    { // Wait for the Wi-Fi to connect
        delay(1000);
        Serial.print(++i);
        Serial.print(' ');
    }

    Serial.println('\n');
    Serial.println("Connection established!");
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer
}

void loop()
{
    WiFiClientSecure httpsClient; //Declare object of class WiFiClient

    Serial.println(host);

    Serial.printf("Using fingerprint '%s'\n", fingerprint);
    httpsClient.setFingerprint(fingerprint);
    httpsClient.setTimeout(2000); // 15 Seconds
    delay(1000);

    Serial.print("HTTPS Connecting");
    int r = 0; //retry counter
    while ((!httpsClient.connect(host, httpsPort)) && (r < 30))
    {
        delay(100);
        Serial.print(".");
        r++;
    }
    if (r == 30)
    {
        Serial.println("Connection failed");
    }
    else
    {
        Serial.println("Connected to web");
    }

    String link;

    //GET Data
    link = "/things/" + thingID;

    Serial.print("requesting URL: ");
    Serial.println(host + link);

    httpsClient.print(String("GET ") + link + " HTTP/1.1\r\n" +
                      "Host: " + host + "\r\n" +
                      "Connection: close\r\n\r\n");

    Serial.println("request sent");

    while (httpsClient.connected())
    {
        String line = httpsClient.readStringUntil('\n');
        Serial.println(line);
        if (line == "\r")
        {
            Serial.println("headers received");
            break;
        }
    }

    Serial.println("reply was:");
    Serial.println("==========");
    String line;
    while (httpsClient.available())
    {
        line = httpsClient.readStringUntil('\n'); //Read Line by Line
        Serial.println(line);                     //Print response
    }
    Serial.println("==========");
    Serial.println("closing connection");
    const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
    DynamicJsonDocument jsonDocument(capacity);

    // Parse JSON object
    auto error = deserializeJson(jsonDocument, line);
    if (error)
    {
        Serial.println(F("Parsing failed!"));
        return;
    }

    // Decode JSON/Extract values
    payload.minionId = jsonDocument["minionId"];
    payload.minionStatus = strcmp("on", jsonDocument["status"]) == 0 ? 1 : 0;
    transmitToMinion(&payload);
}

void transmitToMinion(minionPayload *payload)
{
    if (payload->minionId < 0)
    {
        Serial.println("Illegal payload. Minion does not exist");
        return;
    }
    radio.openWritingPipe(address);
    char buffer[100];
    if (radio.write(payload, sizeof(*payload)))
    {
        sprintf(buffer, "Successfully transmitted to minion: %d, status: %d", payload->minionId, payload->minionStatus);
    }
    else
    {
        sprintf(buffer, "Something went wrong while transmitting to minion: %d", payload->minionId);
    }
    Serial.println(buffer);
}