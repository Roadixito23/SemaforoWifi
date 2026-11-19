# Guía de estudio: código Android de SemaforoWifi

Objetivo: poder explicar el código con tus propias palabras. Las preguntas son las de la lista de "preguntas orales", aplicadas a este proyecto.
Rutas relativas a `app/src/main/java/com/mauricio/semaforowifi/` (salvo el manifest). Los números de línea corresponden al commit actual; si editas, pueden moverse.

## 1. Mapa general

| Archivo | Qué hace |
|---|---|
| `MainActivity.kt` | Punto de entrada (launcher). Muestra `ConnectScreen` y decide a dónde ir. |
| `ConnectScreen.kt` | Pantalla para escribir la IP, validarla y probar la conexión. |
| `ControlActivity.kt` | Segunda Activity. Recibe la IP por `Intent` y muestra `ControlScreen`. |
| `ControlScreen.kt` | Lámparas roja/verde, botones y sondeo de `/estado` cada 2 s. |
| `ArduinoApi.kt` | Única clase que hace red: `GET http://<ip><ruta>` y devuelve `Result<String>`. |
| `AndroidManifest.xml` | Permiso `INTERNET`, HTTP plano permitido y registro de las dos Activities. |

Flujo:

```
MainActivity -> ConnectScreen --(IP válida + /estado responde 200)--> onConnected(ip)
     |                                                                      |
     |        startActivity(Intent a ControlActivity + EXTRA_IP)  <---------+
     v
ControlActivity -> ControlScreen --(cada 2 s)--> ArduinoApi.request(ip, "/estado")
                                  --(botón)----> ArduinoApi.request(ip, "/rojo" | "/verde")
```

Protocolo con el firmware: `/estado`, `/rojo` y `/verde` devuelven `ESTADO:VERDE` o `ESTADO:ROJO` (HTTP 200).

## 2. Las preguntas aplicadas a este código

### P1. ¿Qué pasaría si quitas el `remember` de esta línea?
Líneas: `ConnectScreen.kt:33-35` y `ControlScreen.kt:36-39`.

- `remember { mutableStateOf(...) }` guarda el valor entre recomposiciones. Compose puede ejecutar la función `@Composable` muchas veces; sin `remember`, cada ejecución crea un estado nuevo con el valor inicial.
- Ejemplo con `connecting` (`ConnectScreen.kt:34`): al pulsar Conectar se pone `connecting = true`, eso provoca recomposición, y sin `remember` se vuelve a crear en `false`. Resultado: nunca se vería "Conectando…" ni la barra de progreso, y el botón no se deshabilitaría. Importante: **no deja de compilar ni se cierra**; simplemente nunca se ve ese estado.
- Diferencia con `rememberSaveable` (`ConnectScreen.kt:33`, la IP): además sobrevive a la recreación de la Activity (por ejemplo, al rotar la pantalla). Con `remember` normal, la IP escrita se perdería al rotar.

### P2. ¿Por qué esta pantalla recibe una lambda en vez de abrir la Activity directamente?
Líneas: `ConnectScreen.kt:29-31` (parámetros `onTest`, `onConnected`), `MainActivity.kt:20-26`.

- `ConnectScreen` solo dibuja y avisa ("la prueba tuvo éxito"); no sabe de `Intent`, ni de `ControlActivity`, ni de `ArduinoApi`. Quien decide a dónde navegar es la Activity.
- Ventajas: la pantalla se puede reutilizar y probar sin Android real (por ejemplo, con una lambda falsa), y cambiar la navegación no toca la UI.
- Mismo patrón en `ControlScreen(onDisconnect = { finish() })` (`ControlActivity.kt:27`): la pantalla pide "volver", la Activity lo ejecuta.
- El flujo de datos es "el estado baja, los eventos suben": la Activity pasa funciones hacia abajo y la pantalla las invoca.

### P3. ¿Dónde se registra esta Activity y qué pasa si no está?
Líneas: `app/src/main/AndroidManifest.xml:15-27`.

- Toda Activity debe declararse en el manifest dentro de `<application>`. `ControlActivity` está declarada con `exported="false"` (solo la abre la propia app) y `parentActivityName=".MainActivity"` (la flecha "arriba" vuelve a la principal).
- `MainActivity` tiene `exported="true"` y el `intent-filter` con `MAIN` + `LAUNCHER`: es lo que la hace aparecer en el lanzador del teléfono.
- Si `ControlActivity` no estuviera en el manifest, la app compila, pero al pulsar Conectar (con la prueba exitosa) `startActivity` lanzaría `ActivityNotFoundException` y la app se cerraría.

### P4. ¿Por qué `connect()` no puede ir en el hilo principal?
Líneas: `ArduinoApi.kt:12-13` (`withContext(Dispatchers.IO)`), `ArduinoApi.kt:20` (`conn.connect()`).

- Son dos ideas distintas que conviene separar:
  1. **Por qué es mala idea:** el hilo principal dibuja la interfaz. Una llamada de red puede tardar segundos (aquí hasta 3 s de timeout, líneas 16-17) y congelaría la pantalla.
  2. **Por qué Android la prohíbe:** para proteger la UI, lanza `NetworkOnMainThreadException` de inmediato (no espera al timeout) en cuanto se intenta abrir la conexión desde el hilo principal, y la app se cierra.
- Solución del proyecto: la función es `suspend` y **`withContext(Dispatchers.IO)` es lo que cambia de hilo**. Quien la llama es una corrutina que solo la *lanza*: `scope.launch` con `rememberCoroutineScope` en `ConnectScreen.kt:63` (por un clic) y `LaunchedEffect` en `ControlScreen.kt:58` (el sondeo). Nombra los dos si te preguntan: el scope/efecto lanza, `Dispatchers.IO` cambia de hilo.
- `runCatching` (línea 14) convierte cualquier excepción (timeout, sin red, HTTP distinto de 200) en un `Result.failure`, en lugar de dejar que la app se cierre. El `finally` (línea 24) cierra la conexión siempre.

### P5. (Adaptada) En el original es sobre Bluetooth; aquí es el permiso de red
Esta app **no usa Bluetooth**: se comunica por Wi-Fi con HTTP. Los equivalentes:

- `AndroidManifest.xml:4`: `INTERNET` es un permiso "normal". Se concede al instalar; no hay diálogo en tiempo de ejecución, así que el usuario no puede "rechazarlo".
- `AndroidManifest.xml:13`: `usesCleartextTraffic="true"` permite HTTP sin cifrar. Desde Android 9 (API 28) está bloqueado por defecto, y sin esta línea la petición al Arduino fallaría con "Cleartext HTTP traffic not permitted". Es necesario porque el firmware habla HTTP plano. Tiene un costo: cualquiera en la red podría leer el tráfico (aceptable para este proyecto).
- Qué hace la app si "no hay red" (Wi-Fi apagado, otra red, placa apagada): `request` devuelve `Result.failure`. En `ConnectScreen` se muestra "No se pudo conectar: …" (línea 66). En `ControlScreen`, tras 3 fallos seguidos (`fallos`, línea 62) pasa a "Sin conexión", deshabilita los botones y sigue reintentando.
- Para repasar con un permiso que sí se pide en ejecución, piensa en Bluetooth o ubicación: se solicita al usar la función y se debe manejar el rechazo. Aquí no aplica.

### P6. ¿Qué hace tu app si llega un mensaje con un formato que no espera?
Líneas: `ControlScreen.kt:42-47` (`procesar`), `ArduinoApi.kt:21`.

- `procesar` compara con `"ESTADO:VERDE"` y `"ESTADO:ROJO"`. Cualquier otra cosa cae en el `else` (línea 46): muestra "Respuesta no reconocida: …" cortada a 40 caracteres y sigue funcionando; no se cierra.
- Si el servidor responde con un HTTP distinto de 200, `ArduinoApi.kt:21` lanza `error("HTTP …")`, que `runCatching` convierte en fallo.
- Limitación a conocer: en `ConnectScreen` la prueba solo exige que `/estado` responda 200. Otro servidor (por ejemplo, un router) que responda 200 a esa ruta pasaría la prueba, y el aviso aparecería después en la pantalla de control.

## 3. Señal de alerta: cambios pequeños en vivo
Si te piden demostrar que entiendes el código, practica estos cambios:

1. **Cambiar un texto:** `ControlScreen.kt:82-83` ("Poner rojo"/"Poner verde") o `ConnectScreen.kt:70` ("Conectar").
2. **Cambiar un color:** `ControlScreen.kt:31-32` (`Rojo`, `Verde`).
3. **Cambiar la etiqueta del campo:** `ConnectScreen.kt:47`.
4. **Cambiar el intervalo de sondeo:** `ControlScreen.kt:63` (`delay(2000)`).
5. **Agregar un botón (por ejemplo, amarillo):** en `ControlScreen.kt` añade `Button(... enviar("/amarillo") ...)` y en `semaforo_wifi.ino` una ruta `/amarillo`; `procesar` tendría que aceptar un nuevo estado.
6. **Agregar un campo:** un segundo `OutlinedTextField` en `ConnectScreen.kt` (por ejemplo, puerto), con su `rememberSaveable`, y pasarlo por `onTest`/`onConnected`.

## 4. Autoevaluación rápida
- ¿Qué diferencia hay entre `remember` y `rememberSaveable`, y cuál usa la IP?
- ¿Quién abre `ControlActivity` y cómo recibe la IP? (`Intent` + `EXTRA_IP`, `ControlActivity.kt:14,19`.)
- ¿Por qué `request` es `suspend` y qué hace `Dispatchers.IO`?
- ¿Qué dos líneas del manifest hacen falta para hablar HTTP con el Arduino?
- ¿Qué ve el usuario si la placa se apaga mientras está en la pantalla de control?
- ¿Qué haría la app con una respuesta `ESTADO:AMARILLO`?
