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
    setFixedSize(500, 360);
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
    titleLabel->setStyleSheet("font-size: 20px; font-weight: 700;");
    auto *subtitleLabel = new QLabel("High-Speed Native Desktop Media Downloader", this);
    subtitleLabel->setStyleSheet("font-size: 12.5px; color: #2563eb; font-weight: 600;");
    auto *versionLabel = new QLabel("Version 1.1.0 (Windows Native x64)", this);
    versionLabel->setStyleSheet("font-size: 12px; color: #64748b;");

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);
    titleLayout->addWidget(versionLabel);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    auto *descLabel = new QLabel(
        "Simplest Video Downloader is a lightweight, high-performance desktop utility "
        "engineered for downloading video and audio from popular media platforms and direct streams "
        "with zero bloat and a modern, distraction-free interface.", this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("line-height: 1.4; font-size: 12.5px;");
    mainLayout->addWidget(descLabel);

    auto *companyLabel = new QLabel(
        "Parent Company: <b>Ophira Labs</b><br>"
        "Website: <a href=\"https://ophiralabs.pages.dev/\" style=\"color: #2563eb;\">https://ophiralabs.pages.dev/</a>", this);
    companyLabel->setOpenExternalLinks(true);
    companyLabel->setStyleSheet("font-size: 12.5px; margin-top: 4px;");
    mainLayout->addWidget(companyLabel);

    auto *creditsLabel = new QLabel("Developed by <b>Abdullah Bhatti</b> • Licensed under the MIT License.", this);
    creditsLabel->setStyleSheet("color: #64748b; font-size: 12px; margin-top: 2px;");
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
