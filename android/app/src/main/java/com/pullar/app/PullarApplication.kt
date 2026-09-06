package com.pullar.app

import android.app.Application
import com.pullar.app.core.YoutubeDLEngine

class PullarApplication : Application() {

    override fun onCreate() {
        super.onCreate()
        YoutubeDLEngine.init(this)
    }
}
