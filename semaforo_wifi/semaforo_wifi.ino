// Semaforo Wi-Fi: servidor HTTP que controla 3 LEDs (rojo, amarillo, verde).
// Placas soportadas (se detectan solas al compilar):
//   - Arduino UNO R4 WiFi      (libreria WiFiS3)
//   - ESP8266 / NodeMCU        (libreria ESP8266WiFi)
//   - ESP32                    (libreria WiFi; tambien sirve para simular en Wokwi)
// Rutas HTTP: /estado, /rojo, /verde  (la app Android usa estas tres).

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #define PLACA "ESP8266"
  // NodeMCU: D5 = GPIO14, D6 = GPIO12, D7 = GPIO13 (evitan los pines de arranque D3, D4 y D8)
  const int LED_ROJO = 14;
  const int LED_AMARILLO = 12;
  const int LED_VERDE = 13;
  const unsigned long INTERVALO_REINTENTO = 20000;  // begin() no bloquea: damos tiempo a conectar
#elif defined(ESP32)
  #include <WiFi.h>
  #define PLACA "ESP32"
  const int LED_ROJO = 27;
  const int LED_AMARILLO = 26;
  const int LED_VERDE = 25;
  const unsigned long INTERVALO_REINTENTO = 20000;
#else
  #include "WiFiS3.h"
  #define PLACA "UNO R4 WiFi"
  const int LED_ROJO = 13;
  const int LED_AMARILLO = 12;
  const int LED_VERDE = 11;
  const unsigned long INTERVALO_REINTENTO = 5000;  // begin() bloquea unos segundos
#endif

// Credenciales: copia wifi_secrets.example.h a wifi_secrets.h (esta ignorado por git).
// Sin ese archivo se usa la red abierta del simulador Wokwi.
#if __has_include("wifi_secrets.h")
  #include "wifi_secrets.h"
#else
  const char* WIFI_SSID = "Wokwi-GUEST";
  const char* WIFI_PASS = "";
#endif

unsigned long ultimoIntento = 0;

WiFiServer server(80);
bool servidorIniciado = false;

void mostrarVerde(bool verde) {
  digitalWrite(LED_VERDE, verde ? HIGH : LOW);
  digitalWrite(LED_ROJO, verde ? LOW : HIGH);
  digitalWrite(LED_AMARILLO, LOW);   // reservado: la app aun no lo usa
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

// Decodifica %XX y '+' de un texto recibido por URL (ej. "hola%20mundo" -> "hola mundo")
String decodificarUrl(const String& s) {
  String r = "";
  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '+') {
      r += ' ';
    } else if (c == '%' && i + 2 < s.length() && isxdigit(s[i + 1]) && isxdigit(s[i + 2])) {
      r += (char) strtol(s.substring(i + 1, i + 3).c_str(), NULL, 16);
      i += 2;
    } else {
      r += c;
    }
  }
  return r;
}

// Responde a /led?estado=1 o /led?estado=0 (enciende o apaga el LED del pin 13)
void controlarLed(WiFiClient& client, bool encender) {
  digitalWrite(LED_ROJO, encender ? HIGH : LOW);
  Serial.println(encender ? "LED encendido" : "LED apagado");
  responder(client, "200 OK", encender ? "{\"led\":1}" : "{\"led\":0}");
}

void atenderCliente() {
  WiFiClient client = server.available();
  if (!client) return;

  // Leemos la peticion completa; solo nos interesa la primera linea (ej. "GET /verde HTTP/1.1")
  String primeraLinea = "";
  String linea = "";
  bool esPrimera = true;
  unsigned long inicio = millis();

  while (client.connected() && millis() - inicio < 1000) {
    if (!client.available()) { yield(); continue; }   // yield: evita el watchdog en ESP
    char c = client.read();
    if (c == '\n') {
      if (linea.length() == 0) break;          // linea vacia = fin de cabeceras
      if (esPrimera) { primeraLinea = linea; esPrimera = false; }
      linea = "";
    } else if (c != '\r' && linea.length() < 80) {
      linea += c;
    }
  }

  Serial.println(primeraLinea);

  if (primeraLinea.startsWith("GET /mensaje")) {
    // Formato: GET /mensaje?texto=hola HTTP/1.1
    int ini = primeraLinea.indexOf("texto=");
    String texto = "";
    if (ini >= 0) {
      ini += 6;
      int fin = primeraLinea.indexOf(' ', ini);
      if (fin < 0) fin = primeraLinea.length();
      int amp = primeraLinea.indexOf('&', ini);
      if (amp >= 0 && amp < fin) fin = amp;
      texto = decodificarUrl(primeraLinea.substring(ini, fin));
    }
    Serial.print("Mensaje recibido: ");
    Serial.println(texto);
    responder(client, "200 OK", "{\"ok\":1}");
  } else if (primeraLinea.startsWith("GET /led?estado=1")) {
    controlarLed(client, true);
  } else if (primeraLinea.startsWith("GET /led?estado=0")) {
    controlarLed(client, false);
  } else if (primeraLinea.startsWith("GET /verde")) {
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
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  mostrarVerde(false);   // rojo al iniciar

  Serial.println();
  Serial.print("Placa: ");
  Serial.println(PLACA);

#if defined(ESP8266) || defined(ESP32)
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
#else
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("No se detecta el modulo WiFi");
    while (true);
  }
#endif

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
#if defined(ESP32)
      if (strcmp(WIFI_SSID, "Wokwi-GUEST") == 0) {
        WiFi.begin(WIFI_SSID, WIFI_PASS, 6);   // canal 6: en Wokwi conecta ~4 s mas rapido
      } else {
        WiFi.begin(WIFI_SSID, WIFI_PASS);
      }
#else
      WiFi.begin(WIFI_SSID, WIFI_PASS);
#endif
    }
  }
}
