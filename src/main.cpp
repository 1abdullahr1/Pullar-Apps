#include <QApplication>
#include <QIcon>
#include "MainWindow.h"
#include "AppSettings.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    app.setApplicationName("Simplest Video Downloader");
    app.setApplicationVersion("1.1.0");
    app.setOrganizationName("Ophira Labs");
    app.setOrganizationDomain("ophiralabs.pages.dev");
    app.setWindowIcon(QIcon(":/app.png"));

    MainWindow window;
    window.show();

    return app.exec();
}
