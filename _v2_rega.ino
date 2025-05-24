/* === DEFINIÇÕES DE BIBLIOTECAS === */
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

/* === DEFINIÇÕES DE PINOS === */
#define DHTPIN 23           // Pino de dados do DHT22
#define DHTTYPE DHT22       // Tipo do sensor
#define SOIL_PIN 34         // Potenciómetro simula humidade do solo
#define BOMBA_PIN 2         // LED simula bomba

/* === OBJETOS === */
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

/* === CONFIG WI-FI / MQTT === */
const char* ssid = "GUEST";
const char* password = "";
const char* mqtt_server = "broker.emqx.io";

/* === TÓPICOS MQTT === */
const char* topic_temp = "IPB/IoT/PW2/TurnoD/Group2/Temperatura";
const char* topic_humidade_ar = "IPB/IoT/PW2/TurnoD/Group2/HumidadeAr";
const char* topic_humidade_solo = "IPB/IoT/PW2/TurnoD/Group2/HumidadeSolo";
const char* topic_acao = "IPB/IoT/PW2/TurnoD/Group2/ComandoRega";
const char* topic_estado = "IPB/IoT/PW2/TurnoD/Group2/EstadoRega";

/* === VARIÁVEIS === */
unsigned long lastMsg = 0;
const long intervalo = 5000; // Envio de dados a cada 5 segundos

// === FUNÇÕES ===
void setup_wifi() {
  delay(10);
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado com sucesso!");
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  if (String(topic) == topic_acao) {
    if (msg == "REGAR") {
      digitalWrite(BOMBA_PIN, HIGH);
      client.publish(topic_estado, "BOMBA_LIGADA");
    } else if (msg == "NAO_REGAR") {
      digitalWrite(BOMBA_PIN, LOW);
      client.publish(topic_estado, "BOMBA_DESLIGADA");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao broker MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado!");
      client.subscribe(topic_acao);
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos.");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(BOMBA_PIN, OUTPUT);
  digitalWrite(BOMBA_PIN, LOW); // bomba desligada inicialmente

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > intervalo) {
    lastMsg = now;

    // Leitura dos sensores
    float temp = dht.readTemperature();
    float humAr = dht.readHumidity();
    int soloBruto = analogRead(SOIL_PIN);
    int soloPerc = map(soloBruto, 4095, 0, 0, 100); // solo seco = 0%, molhado = 100%

    if (isnan(temp) || isnan(humAr)) {
      Serial.println("Erro na leitura do sensor DHT");
      return;
    }

    // Enviar temperatura
    char buffer[10];
    dtostrf(temp, 4, 1, buffer);
    client.publish(topic_temp, buffer);

    // Enviar humidade do ar
    dtostrf(humAr, 4, 1, buffer);
    client.publish(topic_humidade_ar, buffer);

    // Enviar humidade do solo
    itoa(soloPerc, buffer, 10);
    client.publish(topic_humidade_solo, buffer);

    // Debug
    Serial.printf("Temp: %.1f ºC | Hum Ar: %.1f %% | Solo: %d %%\n", temp, humAr, soloPerc);
  }
}
