package com.pullar.app.data.database

import android.content.Context
import androidx.room.Database
import androidx.room.Room
import androidx.room.RoomDatabase
import com.pullar.app.data.dao.DownloadDao
import com.pullar.app.data.model.DownloadEntity

@Database(entities = [DownloadEntity::class], version = 2, exportSchema = false)
abstract class PullarDatabase : RoomDatabase() {

    abstract fun downloadDao(): DownloadDao

    companion object {
        @Volatile
        private var INSTANCE: PullarDatabase? = null

        fun getDatabase(context: Context): PullarDatabase {
            return INSTANCE ?: synchronized(this) {
                val instance = Room.databaseBuilder(
                    context.applicationContext,
                    PullarDatabase::class.java,
                    "pullar_database"
                )
                    .fallbackToDestructiveMigration()
                    .build()
                INSTANCE = instance
                instance
            }
        }
    }
}
