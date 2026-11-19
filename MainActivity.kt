package com.mauricio.semaforowifi

import android.content.Intent
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.ui.Modifier

class MainActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MaterialTheme {
                Surface(Modifier.fillMaxSize()) {
                    ConnectScreen(
                        onTest = { ip -> ArduinoApi.request(ip, "/estado") },
                        // La pantalla NO abre la Activity: solo avisa. La Activity decide a dónde ir.
                        onConnected = { ip ->
                            startActivity(
                                Intent(this, ControlActivity::class.java)
                                    .putExtra(ControlActivity.EXTRA_IP, ip)
                            )
                        }
                    )
                }
            }
        }
    }
}
