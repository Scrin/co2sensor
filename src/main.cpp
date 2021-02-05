#include "config.h"

#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "MHZ19.h"

#ifdef HTTPUpdateServer
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>

ESP8266WebServer httpUpdateServer(80);
ESP8266HTTPUpdateServer httpUpdater;
#endif

MHZ19 sensor(RX_PIN, TX_PIN);
WiFiClient espClient;
PubSubClient client(espClient);

bool sensorReady = false;

void checkConnection()
{
  if (client.connected())
  {
    return;
  }
  digitalWrite(LED_BUILTIN, LOW);
  int retries = 0;
  while (!client.connected())
  {
    if (retries < 150)
    {
      Serial.print("Attempting MQTT connection...");
      if (client.connect(MQTT_CLIENT_NAME, MQTT_USER, MQTT_PASS, MQTT_CLIENT_NAME "/availability", 0, true, "offline"))
      {
        Serial.println("connected");
        client.publish(MQTT_CLIENT_NAME "/availability", "online", true);
      }
      else
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        retries++;
        delay(5000);
      }
    }
    else
    {
      ESP.restart();
    }
  }
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\nStarting");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  sensor.setAutoCalibration(false);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

#ifdef HTTPUpdateServer
  MDNS.begin(MQTT_CLIENT_NAME);

  httpUpdater.setup(&httpUpdateServer, USER_HTTP_USERNAME, USER_HTTP_PASSWORD);
  httpUpdateServer.begin();

  MDNS.addService("http", "tcp", 80);
#endif

  client.setServer(MQTT_SERVER, MQTT_PORT);

  checkConnection();
}

void loop()
{
  checkConnection();
  client.loop();

  measurement_t m = sensor.getMeasurement();

  if (!sensorReady)
  {
    if (m.co2_ppm == 410    /* MH-Z19B magic number during warmup */
        || m.co2_ppm == 500 /* MH-Z19C magic number during warmup */
        || m.co2_ppm == 512 /* MH-Z19C magic number on first query */
        || m.co2_ppm == -1) /* Invalid data, happens when the MCU on the sensor is still booting */
    {
      Serial.println("CO2 sensor not ready...");
      for (int i = 0; i < UPDATE_INTERVAL_MS; i += 10)
      {
        digitalWrite(LED_BUILTIN, LOW);
        delay(1);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(9);
      }
      return;
    }
    else
    {
      sensorReady = true;
    }
  }

  if (m.state == -1)
  {
    Serial.println("CO2 sensor in invalid state!");
    return;
  }

  char buf[256];
  snprintf(buf, 256,
           "{\"mcu_name\":\"" MQTT_CLIENT_NAME "\","
           "\"co2\":%d,"
           "\"temperature\":%d,"
           "\"status\":%d}",
           m.co2_ppm, m.temperature, m.state);
  client.publish(MQTT_CLIENT_NAME "/state", buf);

#ifdef HTTPUpdateServer
  for (int i = 0; i < UPDATE_INTERVAL_MS; i += 10)
  {
    httpUpdateServer.handleClient();
    delay(10);
  }
#else
  delay(UPDATE_INTERVAL_MS);
#endif
}
