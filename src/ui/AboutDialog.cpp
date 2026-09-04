#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("About Simplest Video Downloader");
    setFixedSize(480, 320);
    setupUi();
}

void AboutDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 20);
    mainLayout->setSpacing(14);

    auto *headerLayout = new QHBoxLayout();
    auto *iconLabel = new QLabel(this);
    QPixmap icon(":/app.png");
    if (!icon.isNull()) {
        iconLabel->setPixmap(icon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    headerLayout->addWidget(iconLabel);

    auto *titleLayout = new QVBoxLayout();
    auto *titleLabel = new QLabel("Simplest Video Downloader", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: 700; color: #0f172a;");
    auto *subtitleLabel = new QLabel("High-Speed Native Desktop Media Downloader", this);
    subtitleLabel->setStyleSheet("font-size: 12.5px; color: #2563eb; font-weight: 600;");
    auto *versionLabel = new QLabel("Version 1.0.0 (Windows Native x64)", this);
    versionLabel->setStyleSheet("font-size: 12px; color: #64748b;");

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);
    titleLayout->addWidget(versionLabel);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    auto *descLabel = new QLabel(
        "Simplest Video Downloader is a native desktop utility designed to download videos and "
        "audio from YouTube, X, TikTok, Instagram, Facebook, and direct media streams with "
        "zero bloat and maximum performance.", this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: #334155; line-height: 1.4; font-size: 12.5px;");
    mainLayout->addWidget(descLabel);

    auto *creditsLabel = new QLabel("Crafted by <b>Abdullah Bhatti</b><br>Licensed under the MIT License.", this);
    creditsLabel->setStyleSheet("color: #64748b; font-size: 12px;");
    mainLayout->addWidget(creditsLabel);

    mainLayout->addStretch();

    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    auto *okBtn = new QPushButton("Close", this);
    okBtn->setFixedWidth(100);
    btnLayout->addWidget(okBtn);
    mainLayout->addLayout(btnLayout);

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
}
