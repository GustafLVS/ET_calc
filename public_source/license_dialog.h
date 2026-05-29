#pragma once
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class LicenseDialog : public QDialog {
    Q_OBJECT
public:
    explicit LicenseDialog(QWidget* parent = nullptr);

private slots:
    void onActivate();

private:
    void updateStatusLabel(const QString& html);

    QLabel*      m_statusLabel;
    QLineEdit*   m_keyEdit;
    QPushButton* m_activateBtn;
};
