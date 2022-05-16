// Import string
#include <string.h>
// Import NoDelay
#include <NoDelay.h>
// Import DHT library
#include "DHT.h"
// Import WIFI
#include <WiFi.h>
// Import HTTPClient
#include <HTTPClient.h>
// Const to the pump water relay
#define PIN_RELEVADOR 2
// Const to the light bulb relay
#define PIN_RELEVADOR_FOCO 15
// Const to baud rate
#define BAUD_RATE 9600
// Const to soil moisture sensor
#define SOIL_SENSOR 36
// Const to DHT11 temperature and moisture
#define PIN_DHT 0
// Const to the pause
#define PERIODO 1000
// Config for wifi
#define WIFI_SSID "Dinosaurio"
#define WIFI_PASS "dinosaurio1612"
// Endpoint link for the API
#define HOST "https://greenhouse-empotrados.herokuapp.com/sensors/api/"
// Last time when api call was made
unsigned long lastTime = 0;
// Delay for the api call
#define timerDelay 10000
// NoDelay instance for get data
noDelay pausa(PERIODO);
// NoDelay instance for make api call
noDelay pausa2(15000);
// DHT instance
DHT dht(PIN_DHT, DHT11);
// Variable for soil humidity data
int soilMoistureSensorData;
// Variable for temperature data
float actualTemperature;
// Const for maximun temperature
const float LIMIT_TEMPERATURE = 23;
// Const for maximum solil humidity
int limit = 450;

/**
 * @brief Setup function, to setup the hardware
 * 
 */
void setup(){
	// Init baud rate
	Serial.begin(BAUD_RATE);
	// Init connection to wifi
	WiFi.begin(WIFI_SSID, WIFI_PASS);
	// Print in serial port to if connection to wifi
	Serial.println("Connecting");
	// While not connected to wifi print dots
	while (WiFi.status() != WL_CONNECTED){
		delay(500);
		Serial.print(".");
	}
	// Print in serial port private ip
	Serial.println("");
	Serial.print("Connected to WiFi network with IP Address: ");
	Serial.println(WiFi.localIP());

	Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");

	// Set relay pin to output
	pinMode(PIN_RELEVADOR, OUTPUT);
	// Set light bulb pin to output
	pinMode(PIN_RELEVADOR_FOCO, OUTPUT);
	// Set relay as off
	digitalWrite(PIN_RELEVADOR, LOW);
	// Set relay as off
	digitalWrite(PIN_RELEVADOR_FOCO, LOW);
	
	// Begin DHT11
	dht.begin();
}

/**
 * @brief Read temperature
 * 
 * @return float Temperature
 */
float leeHumedadTemperatura(){
	// Read temperature from dht11
	float tc = dht.readTemperature();
	// Return temperature
	return tc;
}

/**
 * @brief Make API call to save sensor data
 * 
 * @param sensor_id First sensor id
 * @param value First sensor value
 * @param sensor_id2 Second sensor id
 * @param value2 Second sensor value
 */
void guardarInformacionSensor(int sensor_id, float value, int sensor_id2, float value2){

	// Send an HTTP POST request every 10 seconds
	if ((millis() - lastTime) > timerDelay){
		// Check WiFi connection status
		if (WiFi.status() == WL_CONNECTED){
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

			// Check the response code
			Serial.print("HTTP Response code: ");
			Serial.println(httpResponseCode);
			// Data to send with HTTP POST to send second sensor data (humidity)
			String payload2 = "{\"sensor\":\"" + String(sensor_id2) + "\",\"data\":\"" + String(value2) + "\"}";
			// Send HTTP POST request to the server
			httpResponseCode = http.POST(payload2);
			// Check the response code
			Serial.print("HTTP Response code: ");
			Serial.println(httpResponseCode);

			// Free resources
			http.end();
		}
		else{
			Serial.println("WiFi Disconnected");
		}
		lastTime = millis();
	}
}

/**
 * @brief Loop function, to read data and make API call
 * 
 */
void loop(){
	// If the time lapse is over, read all data, temperature and humidity
	if (pausa.update()){
		// Read DHT11 temperature and humidity
		actualTemperature = leeHumedadTemperatura();
		Serial.println("Temperatura actual:");
		Serial.println(actualTemperature);

		// Read soil moisture sensor
		soilMoistureSensorData = analogRead(SOIL_SENSOR);
		// soilMoistureSensorData = map(soilMoistureSensorData,550,0,0,100);

		Serial.println("Sensor de humedad del suelo: ");
		Serial.println(soilMoistureSensorData);
	}


	// If actual temperature is greater than LIMIT_TEMPERATURE, turn on the light bulb
	if (actualTemperature < LIMIT_TEMPERATURE){
		// Serial.println("Relevador foco endendida");
		// Turn on the light bulb
		digitalWrite(PIN_RELEVADOR_FOCO, HIGH);
	}
	else{
		// Serial.println("Relevador foco apagado");
		// Turn off the light bulb
		digitalWrite(PIN_RELEVADOR_FOCO, LOW);
	}


	// If soil moisture sensor value is greater than limit, turn on the pump
	if (soilMoistureSensorData > limit){
		// Serial.println("Bomba encendida");
		// Turn on the pump
		digitalWrite(PIN_RELEVADOR, HIGH);
	}
	// If soil moisture sensor value is less than limit, turn off the pump
	else{
		// Serial.println("Bomba apagada");
		// Turn off the pump
		digitalWrite(PIN_RELEVADOR, LOW);
	}

	// Make API call to save sensor data
	if(pausa2.update()){
		// Save sensor data
		guardarInformacionSensor(1, actualTemperature, 2, soilMoistureSensorData);
	}
}