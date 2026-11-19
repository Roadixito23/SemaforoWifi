package com.mauricio.semaforowifi

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.net.HttpURLConnection
import java.net.URL

/** Habla con el Arduino por HTTP. Rutas: /estado, /rojo, /verde. */
object ArduinoApi {

    /** Devuelve el cuerpo de la respuesta (ej. "ESTADO:VERDE") o un error. */
    suspend fun request(ip: String, path: String): Result<String> =
        withContext(Dispatchers.IO) {
            runCatching {
                val conn = URL("http://$ip$path").openConnection() as HttpURLConnection
                conn.connectTimeout = 3000
                conn.readTimeout = 3000
                conn.requestMethod = "GET"
                try {
                    conn.connect()
                    if (conn.responseCode != 200) error("HTTP ${conn.responseCode}")
                    conn.inputStream.bufferedReader().use { it.readText().trim() }
                } finally {
                    conn.disconnect()
                }
            }
        }
}
