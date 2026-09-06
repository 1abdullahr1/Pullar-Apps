package com.pullar.app.ui.screens.downloads

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.pullar.app.data.database.PullarDatabase
import com.pullar.app.data.model.DownloadEntity
import com.pullar.app.data.model.DownloadStatus
import com.pullar.app.data.repository.DownloadRepository
import com.pullar.app.service.DownloadForegroundService
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch

class DownloadsViewModel(application: Application) : AndroidViewModel(application) {

    private val repository: DownloadRepository

    val activeDownloads: StateFlow<List<DownloadEntity>>

    init {
        val db = PullarDatabase.getDatabase(application)
        repository = DownloadRepository(db.downloadDao())
        activeDownloads = repository.activeDownloads.stateIn(
            viewModelScope,
            SharingStarted.WhileSubscribed(5000),
            emptyList()
        )
    }

    fun cancelDownload(task: DownloadEntity) {
        viewModelScope.launch {
            DownloadForegroundService.cancelDownload(getApplication(), task.id)
            repository.updateStatus(task.id, DownloadStatus.CANCELLED)
        }
    }

    fun removeTask(taskId: String) {
        viewModelScope.launch {
            repository.deleteById(taskId)
        }
    }

    fun retryDownload(task: DownloadEntity) {
        viewModelScope.launch {
            repository.updateStatus(task.id, DownloadStatus.QUEUED)
            DownloadForegroundService.startDownload(getApplication(), task.id)
        }
    }
}
