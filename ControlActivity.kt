package com.mauricio.semaforowifi

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.ui.Modifier

class ControlActivity : ComponentActivity() {

    companion object {
        const val EXTRA_IP = "ip"
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val ip = intent.getStringExtra(EXTRA_IP)
        if (ip == null) {          // abierta sin IP: no hay nada que controlar
            finish()
            return
        }
        setContent {
            MaterialTheme {
                Surface(Modifier.fillMaxSize()) {
                    ControlScreen(ip = ip, onDisconnect = { finish() })
                }
            }
        }
    }
}
