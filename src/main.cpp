#include "config.h"

#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "MHZ19.h"

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

  client.setServer(MQTT_SERVER, MQTT_PORT);

  digitalWrite(LED_BUILTIN, HIGH);
  checkConnection();
}

void loop()
{
  checkConnection();
  client.loop();

  measurement_t m = sensor.getMeasurement();

  if (!sensorReady)
  {
    if (m.co2_ppm == 410 || m.co2_ppm == -1)
    {
      Serial.println("CO2 sensor not ready...");
      delay(UPDATE_INTERVAL_MS);
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

  delay(UPDATE_INTERVAL_MS);
}
