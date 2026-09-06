#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("About Pullar");
    setFixedSize(500, 420);
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
    auto *titleLabel = new QLabel("Pullar", this);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: 800;");
    auto *subtitleLabel = new QLabel("Don't just watch—pull it", this);
    subtitleLabel->setStyleSheet("font-size: 13px; color: #7cc6fe; font-weight: 700;");
    auto *versionLabel = new QLabel("Version 1.2.0 (Windows Native x64)", this);
    versionLabel->setStyleSheet("font-size: 12px; color: #945e38; font-weight: 500;");

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);
    titleLayout->addWidget(versionLabel);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    auto *descLabel = new QLabel(
        "Pullar is a high-speed, multi-connection desktop utility "
        "engineered for pulling videos, playlists, and audio from YouTube, X, TikTok, Instagram, "
        "Facebook, Vimeo, Reddit, and direct media streams with built-in media playback.", this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("line-height: 1.4; font-size: 12.5px;");
    mainLayout->addWidget(descLabel);

    auto *featuresLabel = new QLabel(
        "Features: High-speed multi-threaded downloading, YouTube playlist support, "
        "built-in video and audio player, non-blocking UI, and zero telemetry.", this);
    featuresLabel->setWordWrap(true);
    featuresLabel->setStyleSheet("font-size: 12px; color: #7cc6fe;");
    mainLayout->addWidget(featuresLabel);

    auto *contactLabel = new QLabel(
        "Technical Help, Bug Reports & Software Inquiries:<br>"
        "<a href=\"https://abdullahcs.pages.dev/\" style=\"color: #7cc6fe; text-decoration: none; font-weight: bold;\">"
        "https://abdullahcs.pages.dev/</a>", this);
    contactLabel->setOpenExternalLinks(true);
    contactLabel->setStyleSheet("font-size: 12px; margin-top: 4px;");
    mainLayout->addWidget(contactLabel);

    auto *creditsLabel = new QLabel("Developed by Abdullah Bhatti. Licensed under the MIT License.", this);
    creditsLabel->setStyleSheet("font-size: 12px; margin-top: 2px; color: #c4aca0;");
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
