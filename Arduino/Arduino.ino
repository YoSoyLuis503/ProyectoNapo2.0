#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Servo.h>

// WiFi
const char* ssid = "RedLuis";
const char* password = "holahola";

// Firestore
const char* FIRESTORE_PROJECT = "proyectonapo-4d892";
const char* API_KEY = "AIzaSyAaPeAYovM851vbHDplPGCbOXz2-mH6_HA";

String URL_GET =
  "https://firestore.googleapis.com/v1/projects/" +
  String(FIRESTORE_PROJECT) +
  "/databases/(default)/documents/estadoGeneral/river?key=" + API_KEY;

// Servos
Servo servo1;
Servo servo2;

#define S1 12  // D6
#define S2 13  // D7


void setup() {
  Serial.begin(115200);

  // Servos
  servo1.attach(S1);
  servo2.attach(S2);
  servo1.write(0);
  servo2.write(0);

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado!");
}


// mover servos
void moverServos(String estado) {
  if (estado == "Peligro") {
    servo1.write(90);
    servo2.write(90);
  } 
  else if (estado == "alerta") {
    servo1.write(45);
    servo2.write(45);
  } 
  else {
    servo1.write(0);
    servo2.write(0);
  }
}


String extraerCampo(String payload, String key) {
  int pos = payload.indexOf(key);
  if (pos == -1) return "";

  int start = payload.indexOf("stringValue", pos);
  if (start == -1) return "";

  start = payload.indexOf(":", start) + 2;
  int end = payload.indexOf("\"", start);

  return payload.substring(start, end);
}


void loop() {
  if (WiFi.status() == WL_CONNECTED) {

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    http.begin(client, URL_GET);
    int code = http.GET();

    if (code == 200) {
      String payload = http.getString();
      Serial.println("\nFirestore:");
      Serial.println(payload);

      // extraer "estado"
      String estado = extraerCampo(payload, "estado");
      Serial.println("Estado leido: " + estado);

      moverServos(estado);
    }
    else {
      Serial.println("Error GET: " + String(code));
    }

    http.end();
  }

  delay(5000);  // leer cada 5s
}
