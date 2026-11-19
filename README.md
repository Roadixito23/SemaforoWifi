# SemaforoWifi

Aplicación Android en Kotlin y Jetpack Compose para controlar un semáforo conectado por Wi-Fi. El firmware está en `semaforo_wifi/`.

## Configuración

1. Abre la raíz del proyecto en Android Studio y sincroniza Gradle.
2. En `semaforo_wifi/`, copia `wifi_secrets.example.h` a `wifi_secrets.h` y escribe el nombre y la contraseña de tu red Wi-Fi.
3. Abre `semaforo_wifi.ino` en Arduino IDE, selecciona una placa compatible con `WiFiS3` y carga el firmware.
