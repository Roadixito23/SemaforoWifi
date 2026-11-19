# SemaforoWifi

Aplicación Android en Kotlin y Jetpack Compose para controlar un semáforo conectado por Wi-Fi. El firmware está en `semaforo_wifi/`.

## Configuración

1. Abre la raíz del proyecto en Android Studio y sincroniza Gradle.
2. En `semaforo_wifi/`, copia `wifi_secrets.example.h` a `wifi_secrets.h` y escribe el nombre y la contraseña de tu red Wi-Fi.
3. Abre `semaforo_wifi.ino` en Arduino IDE, selecciona una placa compatible con `WiFiS3` y carga el firmware.

## Placa ESP8266 (NodeMCU)

Para compilar con una NodeMCU hay que añadir el soporte ESP8266 en Arduino IDE:

1. Archivo → Preferencias → **Gestor de URLs adicionales de tarjetas**, pega:
   `https://arduino.esp8266.com/stable/package_esp8266com_index.json`
2. Herramientas → Placa → Gestor de tarjetas → instala **esp8266**.
3. Selecciona la placa **NodeMCU 1.0 (ESP-12E Module)**.

Cableado y sketch de referencia: `esp8266_nodemcu/`.
