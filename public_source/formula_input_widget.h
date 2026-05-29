#pragma once
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVector>
#include "formula_engine.h"

// Renders a formula inline with embedded QLineEdit blanks.
// Parses display_template (e.g. "U = {I} × {R}"), creates labels + inputs,
// auto-calculates on every keystroke, emits resultReady() with the full result.
class FormulaInputWidget : public QWidget {
    Q_OBJECT
public:
    explicit FormulaInputWidget(QWidget* parent = nullptr);

    void setFormula(const FormulaDefinition& formula);
    void clear();

    bool allFieldsValid() const;
    QMap<QString, double> currentValues() const;
    const CalculationResult& lastCalcResult() const { return m_lastResult; }

signals:
    void resultReady(const CalculationResult& result);
    void calculateRequested();   // Enter pressed in any field

private slots:
    void onFieldChanged();

private:
    struct FieldEntry { QString varName; QLineEdit* edit; };

    void rebuild(const FormulaDefinition& formula);
    void clearWidgets();
    static QString fieldStyle(const QString& state); // "default", "valid", "invalid"

    QHBoxLayout*        m_hLayout;
    QVector<FieldEntry> m_fields;
    QLabel*             m_resultLabel     = nullptr;
    QLabel*             m_resultUnitLabel = nullptr;

    FormulaDefinition   m_formula;
    CalculationResult   m_lastResult;
};
