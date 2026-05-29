#include "mainwindow.h"
#include "license_manager.h"
#include "license_dialog.h"
#include <QApplication>
#include <QIcon>
#include <QFont>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("ET-Rechner");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("ET-Calc");

    QFont appFont("Segoe UI", 10);
    app.setFont(appFont);

    // ── License check ─────────────────────────────────────────────
    auto& lic = LicenseManager::instance();
    if (lic.checkStatus() != LicenseManager::Valid) {
        // Show activation dialog; user must activate or the process will be
        // terminated by the Quit button (which calls qApp->quit()).
        LicenseDialog dlg;
        if (dlg.exec() != QDialog::Accepted) {
            return 0;   // user clicked Beenden or activation never succeeded
        }
        // Final check after dialog closed
        if (lic.checkStatus() != LicenseManager::Valid) {
            return 0;
        }
    }
    // ─────────────────────────────────────────────────────────────

    MainWindow window;
    window.show();

    return app.exec();
}
