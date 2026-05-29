#include "license_dialog.h"
#include "license_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QApplication>
#include <QTimer>

LicenseDialog::LicenseDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("ET-Rechner — Testversion aktivieren");
    setFixedSize(500, 300);
    // No close button — user must activate or quit
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    setStyleSheet(R"(
        QDialog {
            background: #1a1a1a;
            color: #e0e0e0;
        }
        QLabel {
            color: #c0c0c0;
            font-size: 12px;
        }
        QLineEdit {
            background: #242424;
            border: 1px solid #3a3a3a;
            border-radius: 5px;
            color: #ffffff;
            padding: 8px 14px;
            font-size: 16px;
            font-family: 'Consolas', 'Courier New', monospace;
            letter-spacing: 2px;
        }
        QLineEdit:focus { border-color: #00aaff; }
        QPushButton {
            background: #2e2e2e;
            border: 1px solid #3a3a3a;
            border-radius: 5px;
            color: #d0d0d0;
            padding: 8px 22px;
            font-size: 13px;
        }
        QPushButton:hover { background: #3a3a3a; color: #00aaff; border-color: #00aaff; }
        QPushButton#activateBtn {
            background: #0a3a6a;
            border: 1px solid #00aaff;
            color: #00aaff;
            font-weight: bold;
        }
        QPushButton#activateBtn:hover { background: #0e4a80; }
        QPushButton#quitBtn {
            background: #2e1a1a;
            border: 1px solid #5a2a2a;
            color: #ff7070;
        }
        QPushButton#quitBtn:hover { background: #3a1a1a; border-color: #ff4444; }
    )");

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(32, 26, 32, 22);
    lay->setSpacing(16);

    // ── Title ─────────────────────────────────────────────────────
    auto* titleRow = new QHBoxLayout;
    auto* icon  = new QLabel("⚡");
    icon->setStyleSheet("font-size:28px;");
    auto* title = new QLabel("ET-Rechner  <span style='color:#888; font-size:12px;'>Testversion</span>");
    title->setStyleSheet("font-size:18px; font-weight:bold; color:#00aaff;");
    titleRow->addWidget(icon);
    titleRow->addWidget(title);
    titleRow->addStretch(1);
    lay->addLayout(titleRow);

    // ── Separator ─────────────────────────────────────────────────
    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("border:none; border-top:1px solid #2a2a2a;");
    lay->addWidget(sep);

    // ── Status label ──────────────────────────────────────────────
    m_statusLabel = new QLabel;
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setMinimumHeight(36);

    // Show current status
    const auto st = LicenseManager::instance().checkStatus();
    if (st == LicenseManager::Expired) {
        updateStatusLabel(
            "<span style='color:#ff4444;'>⛔ Testversion abgelaufen.</span>  "
            "Dieser Key ist verbraucht. Bitte einen neuen Key anfordern.");
    } else if (st == LicenseManager::Invalid) {
        updateStatusLabel(
            "<span style='color:#ff8040;'>⚠ Ungültiger Key gespeichert.</span>  "
            "Bitte einen gültigen Product Key eingeben.");
    } else {
        updateStatusLabel(
            "<span style='color:#ffcc00;'>🔑 Bitte Product Key eingeben</span>  "
            "— gültig für <b>5 Tage</b> ab Ausstellungsdatum.");
    }
    lay->addWidget(m_statusLabel);

    // ── Key input ─────────────────────────────────────────────────
    m_keyEdit = new QLineEdit;
    m_keyEdit->setPlaceholderText("XXXXXXXX-XXXXXXXX");
    m_keyEdit->setMaxLength(17);
    m_keyEdit->setAlignment(Qt::AlignCenter);
    m_keyEdit->setInputMask(">HHHHHHHH-HHHHHHHH");
    lay->addWidget(m_keyEdit);

    // ── Buttons ───────────────────────────────────────────────────
    auto* btnRow = new QHBoxLayout;
    auto* quitBtn = new QPushButton("Beenden");
    quitBtn->setObjectName("quitBtn");
    quitBtn->setFixedWidth(110);

    m_activateBtn = new QPushButton("✔  Aktivieren");
    m_activateBtn->setObjectName("activateBtn");
    m_activateBtn->setDefault(true);
    m_activateBtn->setFixedWidth(150);

    btnRow->addWidget(quitBtn);
    btnRow->addStretch(1);
    btnRow->addWidget(m_activateBtn);
    lay->addLayout(btnRow);

    connect(m_activateBtn, &QPushButton::clicked, this, &LicenseDialog::onActivate);
    connect(quitBtn, &QPushButton::clicked, qApp, &QApplication::quit);
    connect(m_keyEdit, &QLineEdit::returnPressed, this, &LicenseDialog::onActivate);
}

void LicenseDialog::updateStatusLabel(const QString& html)
{
    m_statusLabel->setText(html);
}

void LicenseDialog::onActivate()
{
    const QString raw = m_keyEdit->text().trimmed();
    if (raw.isEmpty() || raw == "-") {
        updateStatusLabel("<span style='color:#ff8040;'>⚠ Kein Key eingegeben.</span>");
        return;
    }

    m_activateBtn->setEnabled(false);
    m_activateBtn->setText("Prüfe...");

    const auto result = LicenseManager::instance().activate(raw);

    switch (result) {
    case LicenseManager::Valid: {
        const int days = LicenseManager::instance().daysRemaining();
        updateStatusLabel(
            QString("<span style='color:#00ff88;'>✔ Aktiviert! Noch <b>%1 Tag(e)</b> verfügbar.</span>")
            .arg(days));
        // Short delay so user sees the success message, then close
        QTimer::singleShot(900, this, &QDialog::accept);
        return;
    }
    case LicenseManager::Expired:
        updateStatusLabel(
            "<span style='color:#ff4444;'>⛔ Testversion abgelaufen!</span>  "
            "Dieser Key ist verbraucht — bitte neuen Key anfordern.");
        break;
    case LicenseManager::Invalid:
        updateStatusLabel(
            "<span style='color:#ff4444;'>✖ Ungültiger Key!</span>  "
            "Bitte Key genau wie erhalten eingeben.");
        break;
    default:
        break;
    }

    m_activateBtn->setEnabled(true);
    m_activateBtn->setText("✔  Aktivieren");
}
