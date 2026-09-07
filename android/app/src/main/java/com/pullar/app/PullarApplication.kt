package com.pullar.app

import android.app.Application
import com.pullar.app.core.YoutubeDLEngine
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch

class PullarApplication : Application() {

    private val applicationScope = CoroutineScope(SupervisorJob() + Dispatchers.IO)

    override fun onCreate() {
        super.onCreate()
        applicationScope.launch {
            YoutubeDLEngine.init(this@PullarApplication)
        }
    }
}
