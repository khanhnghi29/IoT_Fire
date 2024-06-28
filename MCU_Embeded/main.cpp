#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MQUnifiedsensor.h>
#include <DHT.h>
#include <Wire.h>

// Define DHT sensors
#define DHTPIN1 D1
#define DHTPIN2 D5
#define DHTTYPE DHT11
DHT dht_1(DHTPIN1, DHTTYPE);
DHT dht_2(DHTPIN2, DHTTYPE);

// Define MQ2 sensor
#define Board ("ESP8266")
#define Pin (A0) // Analog input 3 of your arduino
#define Type ("MQ-2")
#define Voltage_Resolution (3.3) // 3V3 <- IMPORTANT
#define ADC_Bit_Resolution (10)  // For ESP8266
#define RatioMQ2CleanAir (60)
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);
MQUnifiedsensor mq2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

// Define devices
#define D1PIN D4
#define ERROR D6

// Wifi configuration
const char *ssid = "Hung1";
const char *pass = "1234567812";

// MQTT Broker configuration
const char *mqttServer = "192.168.0.102"; // 192.168.0.102-homeAP || 192.168.43.203-mobile
const int mqttPort = 1883;
const char *mqttUser = "";
const char *mqttPass = "";
const char *mqttClientId = "";

long lastMsg = 0;

// Init client

WiFiClient espClient; // init espClient
PubSubClient client(espClient);

// Connect Wifi
void initWifi()
{
  WiFi.mode(WIFI_STA);

  Serial.println();
  Serial.print("Connecting Wifi..");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to Wifi! IP address: ");
  Serial.println(WiFi.localIP());
}

// Connect MQTT Broker
void mqttConn()
{
  while (!client.connected())
  {
    Serial.println();
    Serial.print("Connecting to MQTT Broker..");
    if (client.connect(mqttClientId, mqttUser, mqttPass))
    {
      Serial.println("Connected to MQTT Broker");
      client.subscribe("d1");
      client.subscribe("d2");
      client.subscribe("warning");
    }
    else
    {
      Serial.println("Failed to connect! Retry in 3 second...");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

// MQTT Callback
void callback(char *topic, byte *payload, unsigned int length)
{
  if (strcmp(topic, "d1") == 0)
  {
    if ((char)payload[0] == '1')
    {
      Serial.println("D1 ON");
      digitalWrite(D1PIN, HIGH);
    }
    else
    {
      Serial.println("D1 OFF");
      digitalWrite(D1PIN, LOW);
    }
  }

  if (strcmp(topic, "warning") == 0)
  {
    if ((char)payload[0] == '1')
    {
      Serial.println("Warning");
      digitalWrite(ERROR, HIGH);
    }
    else
    {
      Serial.println("UnWarning");
      digitalWrite(ERROR, LOW);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  initWifi();
  Wire.begin();
  pinMode(ERROR, OUTPUT);
  digitalWrite(ERROR, LOW);
  pinMode(D1PIN, OUTPUT);
  digitalWrite(D1PIN, LOW);
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  dht_1.begin();
  dht_2.begin();
// Setup for MQ2 Sensor
  MQ2.init();
  mq2.init();
  MQ2.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ2.setA(3811.9);
  MQ2.setB(-3.113); // Configure the equation to to calculate Benzene concentration
  mq2.setRegressionMethod(1);
  mq2.setA(30000000);
  mq2.setB(-8.308);
  float calcR0 = 0;
  float CalcR0 = 0;
  for (int i = 1; i <= 10; i++)
  {
    MQ2.update(); // Update data, the arduino will read the voltage from the analog pin
    mq2.update();
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    CalcR0 += mq2.calibrate(RatioMQ2CleanAir);
  }
  MQ2.setR0(calcR0 / 10);
  mq2.setR0(CalcR0 / 10);
}

void loop()
{
  if (!client.connected())
    mqttConn();
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 3000)
  {
    MQ2.update(); // Update data, the arduino will read the voltage from the analog pin
    mq2.update();
    // Read sensors data
    float smoke = mq2.readSensor();
    float gas = MQ2.readSensor();
    int t1 = dht_1.readTemperature();
    int h1 = dht_1.readHumidity();
    int t2 = dht_2.readTemperature();
    int h2 = dht_2.readHumidity();
    int average_t = (t1 + t2) / 2;
    int average_h = (h1 + h2) / 2;
    // Combine data to t,h,l
    String ssData = String(average_t) + "," + String(average_h) + "," + String(gas) + "," + String(smoke);

    client.publish("sensors", ssData.c_str());
    Serial.println(ssData);
    lastMsg = now;
  }
}