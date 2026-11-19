# Referencia NodeMCU (ESP8266)

Port de `../referencia_arduino`: LED rojo sin Wi-Fi, verde con Wi-Fi. El servidor HTTP para la app está en `../semaforo_wifi/` (también compatible con esta placa).

## Arduino IDE
1. Preferencias → URL de gestores adicionales: `https://arduino.esp8266.com/stable/package_esp8266com_index.json`
2. Gestor de placas → instalar **esp8266**.
3. Placa: **NodeMCU 1.0 (ESP-12E Module)**.
4. Copia `wifi_secrets.example.h` a `wifi_secrets.h` y pon tu red (solo 2,4 GHz).
5. En Linux, el usuario debe estar en el grupo `uucp` para usar `/dev/ttyUSB0`.

## Cableado (resistencia de ~220 Ω en cada LED)
| LED | Pin NodeMCU | GPIO |
|---|---|---|
| Rojo | D5 | 14 |
| Amarillo (solo en `semaforo_wifi`) | D6 | 12 |
| Verde | D7 | 13 |

Cada LED: pin → resistencia → ánodo (pata larga); cátodo → G (GND).
No uses D3, D4 ni D8: son pines de arranque.
