#pragma once
#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>

class TempCoeffDialog : public QDialog {
    Q_OBJECT
public:
    explicit TempCoeffDialog(QWidget* parent = nullptr);

private slots:
    void onFilterChanged(const QString& text);

private:
    void populateTable();
    void applyFilter(const QString& text);

    QTableWidget* m_table;
    QLineEdit*    m_filterEdit;
};
