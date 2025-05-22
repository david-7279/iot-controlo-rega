#include "DHT.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Definições de PIN e sensor da humidade do solo
#define DHTPIN 23        // Pino do DHT22
#define DHTTYPE DHT22    // Tipo do DHT

#define LEDPIN 2         // Pino do LED
#define SOILPIN 34       // Novo pino analógico para o "sensor de humidade do solo"

DHT dht(DHTPIN, DHTTYPE);

// Configurações do broker MQTT e Wi-Fi
const char* mqtt_server = "broker.emqx.io";
const char* ssid = "";
const char* password = "";
const char* mqtt_topic = "IPB/IoT/PraticalWork/Rega";

WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
}

void loop() {
  delay(2000); // Espera 2 segundos entre leituras

  float temperatura = dht.readTemperature();
  float humidade = dht.readHumidity();
  int valorSolo = analogRead(SOILPIN); // Lê valor do potenciômetro

  // Converte o valor analógico em porcentagem de umidade do solo
  float humidadeSolo = map(valorSolo, 0, 4095, 100, 0); // 100% molhado, 0% seco

  // Verifica se leitura do DHT falhou
  if (isnan(temperatura) || isnan(humidade)) {
    Serial.println("Falha na leitura do sensor DHT!");
    return;
  }

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  Serial.print("Humidade do ar: ");
  Serial.print(humidade);
  Serial.println(" %");

  Serial.print("Humidade do solo (simulada): ");
  Serial.print(umidadeSolo);
  Serial.println(" %");

  // Acende o LED apenas se ambas as condições forem verdadeiras
  if (humidade < 40 && humidadeSolo < 30) {
    digitalWrite(LEDPIN, HIGH);
    Serial.println("⚠️ Humidade baixa no ar e no solo! LED ligado.");
  } else {
    digitalWrite(LEDPIN, LOW);
  }
}
