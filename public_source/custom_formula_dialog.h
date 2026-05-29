#pragma once
#include "formula_engine.h"
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QPushButton>
#include <QTextEdit>

class CustomFormulaDialog : public QDialog {
    Q_OBJECT
public:
    explicit CustomFormulaDialog(QWidget* parent = nullptr);

    FormulaDefinition getFormula() const;
    QString getCategoryId() const;
    QString getCategoryName() const;

private slots:
    void addVariable();
    void removeVariable();
    void testFormula();
    void onAccept();

private:
    void setupUi();
    void applyStyle();

    QLineEdit* m_formulaId;
    QLineEdit* m_formulaName;
    QLineEdit* m_solveFor;
    QLineEdit* m_expression;
    QLineEdit* m_resultUnit;
    QLineEdit* m_resultDescription;
    QComboBox* m_categoryCombo;
    QLineEdit* m_newCategoryId;
    QLineEdit* m_newCategoryName;
    QTableWidget* m_variableTable;
    QTextEdit* m_stepsEdit;
    QTextEdit* m_testOutput;
    QPushButton* m_testBtn;
    QPushButton* m_okBtn;
};
