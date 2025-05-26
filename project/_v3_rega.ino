#include <DHT.h>

// === Pinos ===
#define SOIL_MOISTURE_PIN 35  // Sensor de umidade do solo
#define BOMB_PIN 25           // Controle da bomba
#define DHT_PIN 32            // Sensor DHT11

// === Configuração do DHT11 ===
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(BOMB_PIN, OUTPUT);
  digitalWrite(BOMB_PIN, LOW);

  dht.begin();
}

void loop() {
  // === Leitura do solo ===
  int moistureValue = analogRead(SOIL_MOISTURE_PIN);
  int soilMoisturePercent = map(moistureValue, 4095, 1500, 0, 100);
  soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);

  Serial.print("Umidade do solo: ");
  Serial.print(soilMoisturePercent);
  Serial.println("%");

  // === Leitura do ar ===
  float temperature = dht.readTemperature();
  float airHumidity = dht.readHumidity();

  if (isnan(temperature) || isnan(airHumidity)) {
    Serial.println("Erro ao ler o DHT11!");
    digitalWrite(BOMB_PIN, LOW);  // Segurança: desliga bomba
  } else {
    Serial.print("Temperatura: ");
    Serial.print(temperature);
    Serial.println("°C");

    Serial.print("Umidade do ar: ");
    Serial.print(airHumidity);
    Serial.println("%");

    // === Lógica inteligente ===
    if (soilMoisturePercent == 0) {
      Serial.println("Solo muito seco → LIGANDO a bomba.");
      digitalWrite(BOMB_PIN, HIGH);
    }
    else if (soilMoisturePercent <= 35) {
      if (airHumidity > 70) {
        Serial.println("Solo seco e ar úmido → Desliga.");
        digitalWrite(BOMB_PIN, LOW);
      } else {
        Serial.println("Solo seco e ar seco → liga a bomba.");
        digitalWrite(BOMB_PIN, HIGH);
      }
    }
    else {
      Serial.println("Solo úmido → NÃO liga a bomba.");
      digitalWrite(BOMB_PIN, LOW);
    }
  }

  Serial.println("--------------------------");
  delay(3000);  // Espera 3 segundos antes da próxima leitura
}
