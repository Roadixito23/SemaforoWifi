// Referencia portada a NodeMCU (ESP8266): rojo sin Wi-Fi, verde con Wi-Fi.
// Misma logica que ../referencia_arduino. Placa en Arduino IDE: "NodeMCU 1.0 (ESP-12E Module)".
#include <ESP8266WiFi.h>

// Copia wifi_secrets.example.h a wifi_secrets.h (esta ignorado por git) y edita tu red (2,4 GHz).
#include "wifi_secrets.h"

// NodeMCU: D5 = GPIO14, D7 = GPIO13 (evitan los pines de arranque D3, D4 y D8)
const int LED_ROJO = D5;
const int LED_VERDE = D7;

const unsigned long INTERVALO_REINTENTO = 20000;  // begin() no bloquea: damos tiempo a conectar
unsigned long ultimoIntento = 0;
bool estabaConectado = false;

void mostrarConectado(bool conectado) {
  digitalWrite(LED_VERDE, conectado ? HIGH : LOW);
  digitalWrite(LED_ROJO, conectado ? LOW : HIGH);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  mostrarConectado(false);   // rojo al iniciar

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  ultimoIntento = millis() - INTERVALO_REINTENTO;  // primer intento inmediato
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    mostrarConectado(true);    // verde
    if (!estabaConectado) {
      estabaConectado = true;
      Serial.print("Conectado, IP: ");
      Serial.println(WiFi.localIP());
    }
  } else {
    mostrarConectado(false);   // rojo
    estabaConectado = false;

    if (millis() - ultimoIntento >= INTERVALO_REINTENTO) {
      ultimoIntento = millis();
      Serial.print("Conectando a ");
      Serial.println(WIFI_SSID);
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
  }
}
