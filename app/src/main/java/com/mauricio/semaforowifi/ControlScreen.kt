package com.mauricio.semaforowifi

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import java.net.URLEncoder

private val Rojo = Color(0xFFD32F2F)
private val Verde = Color(0xFF2E7D32)

@Composable
fun ControlScreen(ip: String, onDisconnect: () -> Unit) {
    var connected by remember { mutableStateOf(true) }
    var verde by remember { mutableStateOf(false) }
    var aviso by remember { mutableStateOf<String?>(null) }
    var fallos by remember { mutableIntStateOf(0) }
    var mensaje by remember { mutableStateOf("") }
    var resultado by remember { mutableStateOf<String?>(null) }
    val scope = rememberCoroutineScope()

    // Envía una ruta que responde JSON ({"ok":1} o {"led":0/1}) y muestra el resultado
    fun enviarSimple(ruta: String, exito: String) {
        scope.launch {
            ArduinoApi.request(ip, ruta)
                .onSuccess { resultado = "$exito ($it)"; aviso = null }
                .onFailure { resultado = null; aviso = "No se pudo enviar: ${it.message}" }
        }
    }

    fun procesar(respuesta: String) {
        when (respuesta) {
            "ESTADO:VERDE" -> { verde = true; aviso = null }
            "ESTADO:ROJO" -> { verde = false; aviso = null }
            else -> aviso = "Respuesta no reconocida: ${respuesta.take(40)}"
        }
    }

    fun enviar(ruta: String) {
        scope.launch {
            ArduinoApi.request(ip, ruta)
                .onSuccess { procesar(it) }
                .onFailure { aviso = "No se pudo enviar: ${it.message}" }
            }
        }

    LaunchedEffect(ip) {
        while (true) {
            ArduinoApi.request(ip, "/estado")
                .onSuccess { fallos = 0; connected = true; procesar(it) }
                .onFailure { fallos++; if (fallos >= 3) connected = false }
            delay(2000)
        }
    }

    Column(
        modifier = Modifier.fillMaxSize().padding(24.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        Text(
            if (connected) "Conectado a $ip" else "Sin conexión con $ip",
            style = MaterialTheme.typography.titleLarge,
            color = if (connected) Verde else Rojo
        )

        Lamp(color = Rojo, on = !connected || !verde)
        Lamp(color = Verde, on = connected && verde)

        Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
            Button(enabled = connected, onClick = { enviar("/rojo") }) { Text("Poner rojo") }
            Button(enabled = connected, onClick = { enviar("/verde") }) { Text("Poner verde") }
        }

        OutlinedTextField(
            value = mensaje,
            onValueChange = { mensaje = it },
            label = { Text("Mensaje") },
            singleLine = true,
            modifier = Modifier.fillMaxWidth()
        )
        Button(
            enabled = connected && mensaje.isNotBlank(),
            onClick = {
                val texto = URLEncoder.encode(mensaje.trim(), "UTF-8")
                enviarSimple("/mensaje?texto=$texto", "Mensaje enviado")
            }
        ) { Text("Enviar") }

        Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
            Button(enabled = connected, onClick = { enviarSimple("/led?estado=1", "LED encendido") }) { Text("Encender") }
            Button(enabled = connected, onClick = { enviarSimple("/led?estado=0", "LED apagado") }) { Text("Apagar") }
        }

        resultado?.let { Text(it, color = Verde) }

        aviso?.let { Text(it, color = MaterialTheme.colorScheme.error) }

        OutlinedButton(onClick = onDisconnect) { Text("Volver") }
    }
}

@Composable
private fun Lamp(color: Color, on: Boolean) {
    Box(
        Modifier
            .size(96.dp)
            .background(if (on) color else color.copy(alpha = 0.2f), CircleShape)
    )
}
