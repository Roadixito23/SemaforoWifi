package com.mauricio.semaforowifi

import android.util.Patterns
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.Button
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.launch

@Composable
fun ConnectScreen(
    onTest: suspend (String) -> Result<String>,
    onConnected: (String) -> Unit          // lambda: la navegación la decide la Activity
) {
    var ip by rememberSaveable { mutableStateOf("") }        // sobrevive a rotaciones
    var connecting by remember { mutableStateOf(false) }     // remember: conserva el valor entre recomposiciones
    var error by remember { mutableStateOf<String?>(null) }
    val scope = rememberCoroutineScope()

    Column(Modifier.fillMaxSize().padding(24.dp)) {
        Text("SemaforoWifi", style = MaterialTheme.typography.headlineMedium)
        Spacer(Modifier.height(8.dp))
        Text("Conecta el teléfono a la misma red WiFi que el Arduino e ingresa la IP que muestra el Monitor Serie.")
        Spacer(Modifier.height(16.dp))

        OutlinedTextField(
            value = ip,
            onValueChange = { ip = it.trim() },
            label = { Text("IP del Arduino (ej. 192.168.1.50)") },
            singleLine = true,
            keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
            modifier = Modifier.fillMaxWidth()
        )
        Spacer(Modifier.height(12.dp))

        Button(
            enabled = !connecting,
            onClick = {
                if (!Patterns.IP_ADDRESS.matcher(ip).matches()) {
                    error = "La IP no es válida."
                    return@Button
                }
                connecting = true
                error = null
                scope.launch {
                    onTest(ip)
                        .onSuccess { onConnected(ip) }
                        .onFailure { error = "No se pudo conectar: ${it.message}" }
                    connecting = false
                }
            }
        ) { Text("Conectar") }

        if (connecting) {
            Spacer(Modifier.height(16.dp))
            Text("Conectando…")
            LinearProgressIndicator(Modifier.fillMaxWidth())
        }
        error?.let {
            Spacer(Modifier.height(16.dp))
            Text(it, color = MaterialTheme.colorScheme.error)
        }
    }
}
