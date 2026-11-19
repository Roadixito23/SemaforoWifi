#include "WiFiS3.h"

#include "wifi_secrets.h"

const int LED_ROJO = 13;
const int LED_VERDE = 12;

const unsigned long INTERVALO_REINTENTO = 5000;  // ms entre intentos
unsigned long ultimoIntento = 0;

WiFiServer server(80);
bool servidorIniciado = false;

void mostrarVerde(bool verde) {
  digitalWrite(LED_VERDE, verde ? HIGH : LOW);
  digitalWrite(LED_ROJO, verde ? LOW : HIGH);
}

const char* estadoActual() {
  return digitalRead(LED_VERDE) == HIGH ? "ESTADO:VERDE" : "ESTADO:ROJO";
}

void responder(WiFiClient& client, const char* codigo, const char* cuerpo) {
  client.print("HTTP/1.1 ");
  client.println(codigo);
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();
  client.println(cuerpo);
}

void atenderCliente() {
  WiFiClient client = server.available();
  if (!client) return;

  // Leemos la petición completa; solo nos interesa la primera línea (ej. "GET /verde HTTP/1.1")
  String primeraLinea = "";
  String linea = "";
  bool esPrimera = true;
  unsigned long inicio = millis();

  while (client.connected() && millis() - inicio < 1000) {
    if (!client.available()) continue;
    char c = client.read();
    if (c == '\n') {
      if (linea.length() == 0) break;          // línea vacía = fin de cabeceras
      if (esPrimera) { primeraLinea = linea; esPrimera = false; }
      linea = "";
    } else if (c != '\r' && linea.length() < 80) {
      linea += c;
    }
  }

  Serial.println(primeraLinea);

  if (primeraLinea.startsWith("GET /verde")) {
    mostrarVerde(true);
    responder(client, "200 OK", estadoActual());
  } else if (primeraLinea.startsWith("GET /rojo")) {
    mostrarVerde(false);
    responder(client, "200 OK", estadoActual());
  } else if (primeraLinea.startsWith("GET /estado")) {
    responder(client, "200 OK", estadoActual());
  } else {
    responder(client, "404 Not Found", "ERROR:RUTA");
  }

  delay(1);
  client.stop();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  mostrarVerde(false);   // rojo al iniciar

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("No se detecta el modulo WiFi");
    while (true);
  }

  ultimoIntento = millis() - INTERVALO_REINTENTO;  // primer intento inmediato
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!servidorIniciado) {
      server.begin();
      servidorIniciado = true;
      mostrarVerde(true);    // verde al conectar (como el sketch original)
      Serial.print("Conectado, IP: ");
      Serial.println(WiFi.localIP());
    }
    atenderCliente();
  } else {
    servidorIniciado = false;
    mostrarVerde(false);     // rojo sin WiFi

    if (millis() - ultimoIntento >= INTERVALO_REINTENTO) {
      ultimoIntento = millis();
      Serial.print("Conectando a ");
      Serial.println(WIFI_SSID);
      WiFi.begin(WIFI_SSID, WIFI_PASS);   // bloquea unos segundos
    }
  }
}
