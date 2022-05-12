#include <string.h>
#include <NoDelay.h>
#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>

// const unsigned int PIN_RELEVADOR = 5;
#define PIN_RELEVADOR 2
// const unsigned int PIN_RELEVADOR_FOCO = 6;
#define PIN_RELEVADOR_FOCO 15
// const unsigned int BAUD_RATE = 9600;
#define BAUD_RATE 9600
// const unsigned int SOIL_SENSOR = A0;
#define SOIL_SENSOR 36
// const unsigned int PIN_DHT = 2;
#define PIN_DHT 0
// const long PERIODO = 1000;
#define PERIODO 1000
// Crea una instancia de la clase noDelay
// que determina si han transcurrido PERIODO ms

// Config for wifi
#define WIFI_SSID "Tuli"
#define WIFI_PASS "24dfg5647mt"
#define HOST "https://greenhouse-empotrados.herokuapp.com/sensors/api/"
unsigned long lastTime = 0;
#define timerDelay 10000

noDelay pausa(PERIODO);
noDelay pausa2(15000);
DHT dht(PIN_DHT, DHT11);

int soilMoistureSensorData;
float actualTemperature;
const float LIMIT_TEMPERATURE = 26;
int limit = 450;

void setup()
{

	Serial.begin(BAUD_RATE);

	WiFi.begin(WIFI_SSID, WIFI_PASS);
	Serial.println("Connecting");
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.print("Connected to WiFi network with IP Address: ");
	Serial.println(WiFi.localIP());

	Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");

	pinMode(PIN_RELEVADOR, OUTPUT);
	pinMode(PIN_RELEVADOR_FOCO, OUTPUT);
	// Establece la velocidad de transmisión del puerto serie al
	// valor BAUD_RATE
	digitalWrite(PIN_RELEVADOR, LOW);
	digitalWrite(PIN_RELEVADOR_FOCO, LOW);

	dht.begin();
}

float leeHumedadTemperatura()
{
	float tc = dht.readTemperature();
	return tc;
}

void guardarInformacionSensor(int sensor_id, float value, int sensor_id2, float value2)
{

	// Send an HTTP POST request every 10 seconds
	if ((millis() - lastTime) > timerDelay)
	{
		// Check WiFi connection status
		if (WiFi.status() == WL_CONNECTED)
		{
			// WiFiClient client;
			HTTPClient http;

			// Your Domain name with URL path or IP address with path
			http.begin(HOST);

			// Specify content-type header
			http.addHeader("Content-Type", "application/json");
			// // Data to send with HTTP POST
			String payload = "{\"sensor\":\"" + String(sensor_id) + "\",\"data\":\"" + String(value) + "\"}";
			// // Send HTTP POST request
			int httpResponseCode = http.POST(payload);
			// int httpResponseCode = http.GET();
			// String payload = http.getString();
			// Serial.print("Response content: ");
			// Serial.println(payload);
			

			Serial.print("HTTP Response code: ");
			Serial.println(httpResponseCode);
			String payload2 = "{\"sensor\":\"" + String(sensor_id2) + "\",\"data\":\"" + String(value2) + "\"}";

			httpResponseCode = http.POST(payload2);

			Serial.print("HTTP Response code: ");
			Serial.println(httpResponseCode);

			// Free resources
			http.end();
		}
		else
		{
			Serial.println("WiFi Disconnected");
		}
		lastTime = millis();
	}
}

void loop()
{

	if (pausa.update())
	{
		actualTemperature = leeHumedadTemperatura();
		Serial.println("Temperatura actual:");
		Serial.println(actualTemperature);

		soilMoistureSensorData = analogRead(SOIL_SENSOR);
		soilMoistureSensorData = map(soilMoistureSensorData,550,0,0,100);

		Serial.println("Sensor de humedad del suelo: ");
		Serial.println(soilMoistureSensorData);
	}



	if (actualTemperature < LIMIT_TEMPERATURE)
	{
		// Serial.println("Relevador foco endendida");
		digitalWrite(PIN_RELEVADOR_FOCO, HIGH);
	}
	else
	{
		// Serial.println("Relevador foco apagado");
		digitalWrite(PIN_RELEVADOR_FOCO, LOW);
	}


	if (soilMoistureSensorData > limit)
	{
		// Serial.println("Bomba encendida");
		digitalWrite(PIN_RELEVADOR, HIGH);
	}
	else
	{
		// Serial.println("Bomba apagada");
		digitalWrite(PIN_RELEVADOR, LOW);
	}
	if(pausa2.update())
	{
		guardarInformacionSensor(1, actualTemperature, 2, soilMoistureSensorData);
	}
}
