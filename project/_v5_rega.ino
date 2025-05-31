<<<<<<< HEAD
/* === DEFINIÇÕES DE BIBLIOTECAS === */
=======
//Enviamos dados temperatura e humidade do ar, humidade do solo, luminosidade(sim ou não)
//quando o solo se encontra num nivel baixo, ativa o motor de água com 5volts(ligado a um transistor)

>>>>>>> 0a07b9e51b3e083b694401c175f66c114f029a7b
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
<<<<<<< HEAD
#include <AESLib.h>
#include <base64.h> 

/* === DEFINIÇÕES DE PINOS === */
#define DHTPIN 23           // Pino de dados do DHT22
#define DHTTYPE DHT22       // Tipo do sensor
#define SOIL_PIN 34         // Potenciómetro simula humidade do solo
#define BOMBA_PIN 2         // LED simula bomba

/* === OBJETOS === */
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);
AESLib aesLib;

/* === CONFIG WI-FI / MQTT === */
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.emqx.io";
const byte aes_key[] = { // Chave AES de 16 bytes (128 bits)
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};
const byte aes_iv[] = { // Vetor de inicialização (IV) de 16 bytes
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

/* === TÓPICOS MQTT === */
=======
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

>>>>>>> 0a07b9e51b3e083b694401c175f66c114f029a7b
const char* topic_temp = "IPB/IoT/PW2/TurnoD/Group2/Temperatura";
const char* topic_humidade_ar = "IPB/IoT/PW2/TurnoD/Group2/HumidadeAr";
const char* topic_humidade_solo = "IPB/IoT/PW2/TurnoD/Group2/HumidadeSolo";
const char* topic_acao = "IPB/IoT/PW2/TurnoD/Group2/ComandoRega";
const char* topic_estado = "IPB/IoT/PW2/TurnoD/Group2/EstadoRega";
<<<<<<< HEAD
const char* topic_cripto = "Cripto/AES";

/* === VARIÁVEIS === */
unsigned long lastMsg = 0;
const long intervalo = 5000; // Envio de dados a cada 5 segundos

// === FUNÇÕES ===
=======
const char* topic_luz = "IPB/IoT/PW2/TurnoD/Group2/Luminosidade";

const int SOLO_SECO = 3500;     // Valor lido com o sensor no solo seco
const int SOLO_MOLHADO = 1700;  // Valor lido com o sensor submerso

unsigned long lastMsg = 0;
const long intervalo = 3000;

>>>>>>> 0a07b9e51b3e083b694401c175f66c114f029a7b
void setup_wifi() {
  delay(10);
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
<<<<<<< HEAD
  Serial.println("WiFi conectado com sucesso!");
=======
  Serial.println("WiFi conectado!");
>>>>>>> 0a07b9e51b3e083b694401c175f66c114f029a7b
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

<<<<<<< HEAD
=======
  msg.trim();
  msg.replace("\"", "");

>>>>>>> 0a07b9e51b3e083b694401c175f66c114f029a7b
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
<<<<<<< HEAD
      client.subscribe(topic_cripto);
=======
>>>>>>> 0a07b9e51b3e083b694401c175f66c114f029a7b
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos.");
      delay(5000);
    }
  }
}

<<<<<<< HEAD
/* === CRIPTOGRAFAR DADOS === */
String encryptAES(String data) {
  // Garantir que o tamanho do input seja múltiplo de 16 bytes (padding)
  int input_len = data.length() + 1; // +1 para o caractere nulo
  int padded_len = ((input_len + 15) / 16) * 16; // Arredonda para o próximo múltiplo de 16
  uint8_t input[padded_len] = {0};
  uint8_t output[padded_len] = {0};
  byte iv[16]; // IV para esta criptografia
  memcpy(iv, aes_iv, 16); // Copiar IV fixo

  // Copiar dados para o buffer de input
  strncpy((char*)input, data.c_str(), input_len);

  // Criptografar
  aesLib.encrypt(input, padded_len, output, aes_key, 128, iv);

  // Codificar em Base64
  String encryptedBase64 = base64::encode(output, padded_len);

  // Publicar o IV junto com os dados (opcional, para descriptografia dinâmica)
  // Para simplificar, assumimos IV fixo; se quiser IV dinâmico, publique-o separadamente
  return encryptedBase64;
}

=======
>>>>>>> 0a07b9e51b3e083b694401c175f66c114f029a7b
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(BOMBA_PIN, OUTPUT);
<<<<<<< HEAD
  digitalWrite(BOMBA_PIN, LOW); // bomba desligada inicialmente

=======
  digitalWrite(BOMBA_PIN, LOW);
  pinMode(LUZ_PIN, INPUT); // digital IN
>>>>>>> 0a07b9e51b3e083b694401c175f66c114f029a7b
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

<<<<<<< HEAD
    // Leitura dos sensores
    float temp = dht.readTemperature();
    float humAr = dht.readHumidity();
    int soloBruto = analogRead(SOIL_PIN);
    int soloPerc = map(soloBruto, 4095, 0, 0, 100); // solo seco = 0%, molhado = 100%

    if (isnan(temp) || isnan(humAr)) {
      Serial.println("Erro na leitura do sensor DHT");
      return;
    }

    // Criptografar e enviar temperatura
    char buffer[10];
    dtostrf(temp, 4, 1, buffer);
    String encryptedTemp = encryptAES(String(buffer));
    client.publish(topic_temp, encryptedTemp.c_str());

    // Criptografar e enviar umidade do ar
    dtostrf(humAr, 4, 1, buffer);
    String encryptedHumAr = encryptAES(String(buffer));
    client.publish(topic_humidade_ar, encryptedHumAr.c_str());

    // Criptografar e enviar umidade do solo
    itoa(soloPerc, buffer, 10);
    String encryptedSolo = encryptAES(String(buffer));
    client.publish(topic_humidade_solo, encryptedSolo.c_str());

    // Debug
    Serial.printf("Temp: %.1f ºC | Hum Ar: %.1f %% | Solo: %d %%\n", temp, humAr, soloPerc);
=======
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
>>>>>>> 0a07b9e51b3e083b694401c175f66c114f029a7b
  }
}
