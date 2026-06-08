#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QStringList>
#include "mainwindow.h"
#include "theme/thememanager.h"

// Returns the first name from `candidates` that is actually installed,
// or an empty string if none match (Qt will use the system default).
static QString pickFont(const QStringList &candidates)
{
    const QStringList available = QFontDatabase::families();
    for (const QString &name : candidates)
        if (available.contains(name, Qt::CaseInsensitive))
            return name;
    return {};
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);
    QApplication app(argc, argv);
    app.setApplicationName("Oreon System Manager");
    app.setOrganizationName("Oreon");
    app.setApplicationVersion("0.1.0");

    // Probe installed families so Qt never has to search through aliases.
    // Priority: Linux distro fonts first, then macOS, then Windows, then generic.
    const QString uiFamily = pickFont(
        {"Noto Sans", "Cantarell", "Ubuntu", "Roboto", "Helvetica Neue", "Segoe UI", "Arial"});

    QFont uiFont;
    if (!uiFamily.isEmpty())
        uiFont.setFamily(uiFamily);
    uiFont.setPointSize(10);
    uiFont.setStyleHint(QFont::SansSerif);
    app.setFont(uiFont);

    ThemeManager::instance().restore();

    MainWindow window;
    window.show();

    return app.exec();
}
