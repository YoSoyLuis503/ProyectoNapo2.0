#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Barrera";
const char* password = "b@rr@r@BZL67";

const char* FIRESTORE_PROJECT = "proyectonapo-4d892";
const char* API_KEY = "AIzaSyAaPeAYovM851vbHDplPGCbOXz2-mH6_HA";

// Documento fijo en Firestore
String FIRESTORE_URL =
  "https://firestore.googleapis.com/v1/projects/" +
  String(FIRESTORE_PROJECT) +
  "/databases/(default)/documents/alertas/sensor1?key=" + API_KEY;

// Pines del HC-SR04
#define TRIG D1
#define ECHO D2

void setup() {
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado!");
}

// Medir distancia
float medirDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duracion = pulseIn(ECHO, HIGH);
  float distancia = duracion * 0.034 / 2;

  return distancia;
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();

    float distancia = medirDistancia();

    Serial.print("Distancia enviada: ");
    Serial.println(distancia);

    // Datos en bruto
    String json =
      "{"
        "\"fields\": {"
          "\"distancia\": {\"doubleValue\": " + String(distancia) + "},"
          "\"lat\": {\"doubleValue\": 13.474144239341106},"
          "\"lng\": {\"doubleValue\": -88.159747069461},"
          "\"timestamp\": {\"stringValue\": \"" + String(millis()) + "\"}"
        "}"
      "}";

    http.begin(client, FIRESTORE_URL);
    http.addHeader("Content-Type", "application/json");

    int code = http.PATCH(json);  // actualizar documento
    Serial.println("Firestore code: " + String(code));

    Serial.println("Respuesta Firestore:");
    Serial.println(http.getString());

    http.end();
  }

  delay(5000);
}
