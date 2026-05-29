#pragma once
#include <QString>
#include <QMap>
#include <QStringList>
#include <QVector>
#include <cmath>

struct FormulaVariable {
    QString name;
    QString unit;
    QString description;
};

struct FormulaDefinition {
    QString id;
    QString name;
    QString solve_for;
    QString expression;
    QVector<FormulaVariable> variables;
    QString result_unit;
    QString result_description;
    QStringList steps;
    QString display_template;  // e.g. "U = {I} × {R}"
    QString chain_formula;     // e.g. "U = I × R"
};

struct CategoryDefinition {
    QString name;
    QString id;
    QString circuit_type;
    QVector<FormulaDefinition> formulas;
};

struct CalculationResult {
    bool    success    = false;
    double  value      = 0.0;
    QString result_unit;
    QString result_description;
    QStringList steps;
    QString chain_line;
    QString error;
};

class FormulaEngine {
public:
    // Main calculation
    static CalculationResult calculate(const FormulaDefinition& formula,
                                       const QMap<QString, double>& values);

    // SI prefix aware parsing/formatting
    static double parseSIValue(const QString& text, bool& ok);
    static QString formatSI(double value, const QString& unit = "");
    static QString formatSICompact(double value);   // no space, e.g. "3k"

    // Build the inline chain string
    static QString buildChainLine(const FormulaDefinition& formula,
                                  const QMap<QString, double>& values,
                                  double result);

private:
    static double evaluateExpression(const QString& expr,
                                     const QMap<QString, double>& vars,
                                     bool& ok);
    static QStringList buildSteps(const FormulaDefinition& formula,
                                  const QMap<QString, double>& values,
                                  double result);
};
