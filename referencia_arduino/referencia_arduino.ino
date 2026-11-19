#include "WiFiS3.h"

const char* WIFI_SSID = "NOMBRE_DE_RED";
const char* WIFI_PASS = "CONTRASENA_DE_RED";

const int LED_ROJO = 13;
const int LED_VERDE = 12;

const unsigned long INTERVALO_REINTENTO = 5000;  // ms entre intentos
unsigned long ultimoIntento = 0;

void mostrarConectado(bool conectado) {
  digitalWrite(LED_VERDE, conectado ? HIGH : LOW);
  digitalWrite(LED_ROJO, conectado ? LOW : HIGH);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  mostrarConectado(false);   // rojo al iniciar

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("No se detecta el modulo WiFi");
    while (true);
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    mostrarConectado(true);    // verde
  } else {
    mostrarConectado(false);   // rojo

    if (millis() - ultimoIntento >= INTERVALO_REINTENTO) {
      ultimoIntento = millis();
      Serial.print("Conectando a ");
      Serial.println(WIFI_SSID);
      WiFi.begin(WIFI_SSID, WIFI_PASS);   // bloquea unos segundos

      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("Conectado, IP: ");
        Serial.println(WiFi.localIP());
      } else {
        Serial.println("Sin conexion");
      }
    }
  }
}