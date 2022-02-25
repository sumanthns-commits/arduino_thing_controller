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
const char *host = "65052d7tk3.execute-api.ap-southeast-2.amazonaws.com";
const char *accessToken = ""; // Access token generated from hub app
const int httpsPort = 443;
const char fingerprint[] PROGMEM = "854cad33e0bd72a1ebe28dc08948b6f9425bd562";
const String thingID = "1";
const char *things[1] = {"r2F9p4RzR1"};
RF24 radio(CE, CSN);

const byte address[10] = "minion";

struct MinionPayload
{
    char thingId[11];
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
    WiFiClientSecure httpsClient; // Declare object of class WiFiClient

    Serial.println(host);

    Serial.printf("Using fingerprint '%s'\n", fingerprint);
    httpsClient.setFingerprint(fingerprint);
    httpsClient.setTimeout(2000); // 15 Seconds
    delay(1000);

    Serial.print("HTTPS Connecting");
    int r = 0; // retry counter
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

    // GET Data
    link = "/machineapi/machine";

    Serial.print("requesting URL: ");
    Serial.println(host + link);

    httpsClient.print(String("GET ") + link + " HTTP/1.1\r\n" +
                      "Host: " + host + "\r\n" +
                      "Content-Type: application/json\r\n" +
                      "Authorization: Basic " + accessToken + "\r\n" +
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
        line = httpsClient.readStringUntil('\n'); // Read Line by Line
        Serial.println(line);                     // Print response
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
    JsonArray thingsArr = jsonDocument["things"].as<JsonArray>();
    for (JsonVariant value : thingsArr)
    {
        bool exists = std::find(std::begin(things), std::end(things), value["thingId"]) != std::end(things);
        if (exists)
        {
            MinionPayload payload;
            strcpy(payload.thingId, value["thingId"]);
            strcpy(payload.status, value["status"]);
            transmitToMinion(&payload);
        }
    }
}

void transmitToMinion(MinionPayload *payload)
{
    radio.openWritingPipe(address);
    char buffer[100];
    if (radio.write(payload, sizeof(*payload)))
    {
        sprintf(buffer, "Successfully transmitted to minion: %s", payload->thingId);
    }
    else
    {
        sprintf(buffer, "Something went wrong while transmitting to minion: %s", payload->thingId);
    }
    Serial.println(buffer);
}