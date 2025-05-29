//Enviamos dados temperatura e humidade do ar, humidade do solo, luminosidade(sim ou não)
//quando o solo se encontra num nivel baixo, ativa o motor de água com 5volts(ligado a um transistor)

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <base64.h>

#define DHTTYPE DHT11
#define DHTPIN 32     // amarelo
#define SOIL_PIN 35   // castanho
#define BOMBA_PIN 25  // azul
#define LUZ_PIN 34    // verde (DO do sensor de luz)

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid = "Redmi-Noira";
const char* password = "Noira1234";
const char* mqtt_server = "broker.emqx.io";

const char* topic_temp = "IPB/IoT/PW2/TurnoD/Group2/Temperatura";
const char* topic_humidade_ar = "IPB/IoT/PW2/TurnoD/Group2/HumidadeAr";
const char* topic_humidade_solo = "IPB/IoT/PW2/TurnoD/Group2/HumidadeSolo";
const char* topic_acao = "IPB/IoT/PW2/TurnoD/Group2/ComandoRega";
const char* topic_estado = "IPB/IoT/PW2/TurnoD/Group2/EstadoRega";
const char* topic_luz = "IPB/IoT/PW2/TurnoD/Group2/Luminosidade";

const int SOLO_SECO = 3500;     // Valor lido com o sensor no solo seco
const int SOLO_MOLHADO = 1700;  // Valor lido com o sensor submerso

unsigned long lastMsg = 0;
const long intervalo = 3000;

void setup_wifi() {
  delay(10);
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado!");
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  msg.trim();
  msg.replace("\"", "");

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
  digitalWrite(BOMBA_PIN, LOW);
  pinMode(LUZ_PIN, INPUT); // digital IN
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

    float temp = dht.readTemperature();
    float humAr = dht.readHumidity();

    int soloBruto = analogRead(SOIL_PIN);
    soloBruto = constrain(soloBruto, SOLO_MOLHADO, SOLO_SECO);
    int soloPerc = map(soloBruto, SOLO_SECO, SOLO_MOLHADO, 0, 100);

    int estadoLuz = digitalRead(LUZ_PIN);
    int luzBinaria = (estadoLuz == HIGH) ? 0 : 1;

    if (isnan(temp) || isnan(humAr)) {
      Serial.println("Erro na leitura do DHT");
      return;
    }

    // Publica dados
    char buffer[10];
    dtostrf(temp, 4, 1, buffer);
    client.publish(topic_temp, buffer);
    dtostrf(humAr, 4, 1, buffer);
    client.publish(topic_humidade_ar, buffer);

    itoa(soloPerc, buffer, 10);
    client.publish(topic_humidade_solo, buffer);

    char luzStr[2];
    itoa(luzBinaria, luzStr, 10);
    client.publish(topic_luz, luzStr);

    Serial.printf("Temp: %.1f ºC | Hum Ar: %.1f %% | Solo: %d %% | Luz (bin): %d\n", temp, humAr, soloPerc, luzBinaria);
  }
}
