/* === DEFINIÇÕES DE BIBLIOTECAS === */
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <mbedtls/aes.h>
#include <base64.h>

/* === DEFINIÇÕES DE PINOS === */
#define DHTPIN 23           // Pino de dados do DHT22
#define DHTTYPE DHT22       // Tipo do sensor
#define SOIL_PIN 34         // Potenciómetro simula umidade do solo
#define BOMBA_PIN 2         // LED simula bomba

/* === OBJETOS === */
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

/* === CONFIG WI-FI / MQTT === */
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.emqx.io";

// Chave AES (16 bytes para AES-128)
const byte aes_key[] = {
  0x45, 0xE0, 0x2E, 0xF8, 0xED, 0x0C, 0x5C, 0xFE,
  0xAC, 0xAA, 0x51, 0xBF, 0xD0, 0x97, 0x90, 0xDF
};
// IV fixo de 16 bytes
const byte aes_iv[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

/* === TÓPICOS MQTT === */
const char* topic_temp = "IPB/IoT/PW2/TurnoD/Group2/Temperatura";
const char* topic_humidade_ar = "IPB/IoT/PW2/TurnoD/Group2/HumidadeAr";
const char* topic_humidade_solo = "IPB/IoT/PW2/TurnoD/Group2/HumidadeSolo";
const char* topic_acao = "IPB/IoT/PW2/TurnoD/Group2/ComandoRega";
const char* topic_estado = "IPB/IoT/PW2/TurnoD/Group2/EstadoRega";

/* === VARIÁVEIS === */
unsigned long lastMsg = 0;
const long intervalo = 6400; // Envio de dados a cada 6,4 segundos

/* === FUNÇÕES === */
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

String encryptAES(String data) {
  Serial.print("Encrypting: ");
  Serial.println(data);

  // Prepare input with Pkcs7 padding
  byte input[16] = {0};
  byte output[16];
  int input_len = data.length();
  int copy_len = input_len < 16 ? input_len : 15;
  strncpy((char*)input, data.c_str(), copy_len);
  int padding = 16 - copy_len;
  for (int i = copy_len; i < 16; i++) {
    input[i] = padding;
  }

  // Initialize AES context
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, aes_key, 128);

  // Encrypt with CBC mode
  byte iv[16];
  memcpy(iv, aes_iv, 16);
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, 16, iv, input, output);

  // Clean up
  mbedtls_aes_free(&aes);

  // Encode to Base64
  String encrypted = base64::encode(output, 16);
  Serial.print("Encrypted (Base64): ");
  Serial.println(encrypted);
  return encrypted;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(BOMBA_PIN, OUTPUT);
  digitalWrite(BOMBA_PIN, LOW);

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
    int soloPerc = map(soloBruto, 4095, 0, 0, 100);

    if (isnan(temp) || isnan(humAr)) {
      Serial.println("Erro na leitura do sensor DHT");
      return;
    }

    // Enviar temperatura (criptografada)
    char buffer[10];
    dtostrf(temp, 4, 1, buffer);
    String encryptedTemp = encryptAES(String(buffer));
    client.publish(topic_temp, encryptedTemp.c_str());

    // Enviar humidade do ar (sem criptografia)
    dtostrf(humAr, 4, 1, buffer);
    client.publish(topic_humidade_ar, buffer);

    // Enviar humidade do solo (sem criptografia)
    itoa(soloPerc, buffer, 10);
    client.publish(topic_humidade_solo, buffer);

    // Debug
    Serial.printf("Temp: %.1f ºC | Hum Ar: %.1f %% | Solo: %d %%\n", 
                  temp, humAr, soloPerc);
  }
}