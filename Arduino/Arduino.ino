#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Servo.h>

// -----------------------------
// CONFIG WIFI
// -----------------------------
const char* ssid = "RedLuis";
const char* password = "holahola";

// -----------------------------
// FIRESTORE CONFIG
// -----------------------------
const char* FIRESTORE_PROJECT = "proyectonapo-4d892";
const char* API_KEY = "AIzaSyAaPeAYovM851vbHDplPGCbOXz2-mH6_HA";

String FIRESTORE_URL =
  "https://firestore.googleapis.com/v1/projects/" +
  String(FIRESTORE_PROJECT) +
  "/databases/(default)/documents/alertas/sensor1?key=" + API_KEY;

// -----------------------------
// SENSOR HC-SR04
// -----------------------------
#define TRIG D1
#define ECHO D2

// -----------------------------
// SERVOS + LEDS
// -----------------------------
Servo servo1;
Servo servo2;

#define SERVO1_PIN D5
#define SERVO2_PIN D6  // ← segundo servo

#define LED_VERDE    D7
#define LED_AMARILLO D8
#define LED_ROJO     D4   // usa otro pin libre si querés

// -----------------------------
String ultimoEstado = ""; 
// -----------------------------


void setup() {
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // LEDs
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);

  // Servos
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  // Estado inicial (abierto)
  servo1.write(0);
  servo2.write(0);
  digitalWrite(LED_VERDE, HIGH);

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado!");
}


// -----------------------------
// Medir distancia
// -----------------------------
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



// -----------------------------
// LOOP
// -----------------------------
void loop() {

  float distancia = medirDistancia();
  Serial.print("Distancia medida: ");
  Serial.println(distancia);

  String estado;

  // -----------------------------
  // DETERMINAR EL ESTADO
  // -----------------------------
  if (distancia <= 4) {
    estado = "Peligro";
  } 
  else if (distancia <= 6) {
    estado = "Alerta";
  } 
  else {
    estado = "Normal";
  }

  // -----------------------------
  // ACTUAR SOLO SI EL ESTADO CAMBIÓ
  // -----------------------------
  if (estado != ultimoEstado) {
    ultimoEstado = estado;

    if (estado == "Normal") {
      servo1.write(0);
      servo2.write(0);
      digitalWrite(LED_VERDE, HIGH);
      digitalWrite(LED_AMARILLO, LOW);
      digitalWrite(LED_ROJO, LOW);
    }

    else if (estado == "Alerta") {
      servo1.write(45);
      servo2.write(45);
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_AMARILLO, HIGH);
      digitalWrite(LED_ROJO, LOW);
    }

    else if (estado == "Peligro") {
      servo1.write(90);
      servo2.write(90);
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_AMARILLO, LOW);
      digitalWrite(LED_ROJO, HIGH);
    }

    Serial.println("Nuevo estado → " + estado);
  }

  // -----------------------------
  // SUBIR A FIRESTORE
  // -----------------------------
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();

    String json =
      "{"
        "\"fields\": {"
          "\"distancia\": {\"doubleValue\": " + String(distancia) + "},"
          "\"estadoLocal\": {\"stringValue\": \"" + estado + "\"},"
          "\"lat\": {\"doubleValue\": 13.474144239341106},"
          "\"lng\": {\"doubleValue\": -88.159747069461},"
          "\"timestamp\": {\"stringValue\": \"" + String(millis()) + "\"}"
        "}"
      "}";

    http.begin(client, FIRESTORE_URL);
    http.addHeader("Content-Type", "application/json");

    int code = http.PATCH(json);
    Serial.println("Firestore code: " + String(code));
    Serial.println(http.getString());

    http.end();
  }

  delay(3000);
}
