#include "formula_engine.h"
#include <QRegularExpression>
#include <QStack>
#include <cmath>
#include <limits>

// ─── SI Prefix Parsing ───────────────────────────────────────────────────────

double FormulaEngine::parseSIValue(const QString& text, bool& ok) {
    QString t = text.trimmed().replace(",", ".");
    ok = false;
    if (t.isEmpty()) return 0.0;

    // SI prefix map (longest first, case-sensitive where needed)
    struct PrefixEntry { QString sym; double mult; };
    static const QVector<PrefixEntry> prefixes = {
        {"T",  1e12}, {"G",  1e9}, {"M",  1e6},
        {"k",  1e3},  {"K",  1e3},
        {"m",  1e-3},
        {"µ",  1e-6}, {"u",  1e-6},
        {"n",  1e-9}, {"p",  1e-12}
    };

    for (const auto& p : prefixes) {
        if (t.endsWith(p.sym)) {
            QString numStr = t.left(t.length() - p.sym.length());
            bool numOk;
            double v = numStr.toDouble(&numOk);
            if (numOk) { ok = true; return v * p.mult; }
        }
    }

    // Plain number (possibly with unit letter at end — strip one letter and retry)
    double v = t.toDouble(&ok);
    if (ok) return v;

    if (t.length() > 1) {
        // Try stripping last non-numeric, non-prefix character (unit letter like A, V, Ω…)
        QString noUnit = t.left(t.length() - 1);
        v = noUnit.toDouble(&ok);
        if (ok) return v;

        // Try prefix + unit: "8mA" → strip 'A', then check "8m"
        if (noUnit.length() > 0) {
            for (const auto& p : prefixes) {
                if (noUnit.endsWith(p.sym)) {
                    QString numStr = noUnit.left(noUnit.length() - p.sym.length());
                    bool numOk;
                    double val = numStr.toDouble(&numOk);
                    if (numOk) { ok = true; return val * p.mult; }
                }
            }
        }
    }

    ok = false;
    return 0.0;
}

QString FormulaEngine::formatSICompact(double value) {
    if (value == 0.0) return "0";
    double abs_val = std::abs(value);
    struct P { double f; const char* s; };
    static const P ps[] = {
        {1e12,"T"},{1e9,"G"},{1e6,"M"},{1e3,"k"},
        {1.0,""},{1e-3,"m"},{1e-6,"µ"},{1e-9,"n"},{1e-12,"p"}
    };
    for (const auto& p : ps) {
        double sc = abs_val / p.f;
        if (sc >= 1.0 && sc < 1000.0) {
            QString n = (sc >= 100) ? QString::number(sc,'f',1) :
                        (sc >= 10)  ? QString::number(sc,'f',2) :
                                      QString::number(sc,'f',3);
            while (n.contains('.') && (n.endsWith('0') || n.endsWith('.')))
                n.chop(1);
            return (value < 0 ? "-" : "") + n + p.s;
        }
    }
    return QString::number(value,'g',4);
}

QString FormulaEngine::formatSI(double value, const QString& unit) {
    QString c = formatSICompact(value);
    if (unit.isEmpty()) return c;
    // Insert space before unit (but not before prefix — already merged)
    return c + " " + unit;
}

// ─── Expression evaluator (shunting-yard) ────────────────────────────────────

static double applyOp(double a, double b, QChar op) {
    switch (op.toLatin1()) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0) ? a / b : std::numeric_limits<double>::quiet_NaN();
        case '^': return std::pow(a, b);
    }
    return 0;
}
static int precedence(QChar op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 3;
    return 0;
}

double FormulaEngine::evaluateExpression(const QString& expr,
                                          const QMap<QString, double>& vars,
                                          bool& ok)
{
    ok = false;
    QString processed = expr;

    // Replace sqrt(...)
    QRegularExpression sqrtRe(R"(sqrt\(([^()]+)\))");
    int maxPasses = 20;
    while (processed.contains("sqrt(") && maxPasses-- > 0) {
        auto m = sqrtRe.match(processed);
        if (!m.hasMatch()) break;
        bool innerOk;
        double inner = evaluateExpression(m.captured(1), vars, innerOk);
        if (!innerOk || inner < 0) return std::numeric_limits<double>::quiet_NaN();
        processed.replace(m.capturedStart(), m.capturedLength(),
                          QString::number(std::sqrt(inner), 'g', 15));
    }

    // Replace variable names (longest first)
    QStringList varNames = vars.keys();
    std::sort(varNames.begin(), varNames.end(), [](const QString& a, const QString& b){
        return a.length() > b.length();
    });
    for (const QString& vname : varNames) {
        processed.replace(
            QRegularExpression(R"(\b)" + QRegularExpression::escape(vname) + R"(\b)"),
            QString::number(vars[vname], 'g', 15));
    }
    processed.replace("3.14159265", QString::number(M_PI,'g',15));

    // Shunting-yard
    QStack<double> output;
    QStack<QChar> operators;

    auto applyTop = [&]() -> bool {
        if (output.size() < 2) return false;
        double b = output.pop(), a = output.pop();
        QChar op = operators.pop();
        double res = applyOp(a, b, op);
        if (std::isnan(res)) return false;
        output.push(res);
        return true;
    };

    int i = 0;
    bool expectUnary = true;

    while (i < processed.length()) {
        QChar c = processed[i];
        if (c.isSpace()) { ++i; continue; }

        if (c.isDigit() || c == '.' || (c == '-' && expectUnary)) {
            int start = i;
            if (c == '-') ++i;
            while (i < processed.length() &&
                   (processed[i].isDigit() || processed[i] == '.' ||
                    processed[i] == 'e' || processed[i] == 'E' ||
                    ((processed[i]=='+' || processed[i]=='-') && i > 0 &&
                     (processed[i-1]=='e' || processed[i-1]=='E')))) ++i;
            bool convOk;
            double val = processed.mid(start, i - start).toDouble(&convOk);
            if (!convOk) return std::numeric_limits<double>::quiet_NaN();
            output.push(val);
            expectUnary = false;
            continue;
        }

        if (c == '(') { operators.push(c); expectUnary = true; ++i; continue; }

        if (c == ')') {
            while (!operators.isEmpty() && operators.top() != '(')
                if (!applyTop()) return std::numeric_limits<double>::quiet_NaN();
            if (!operators.isEmpty()) operators.pop();
            expectUnary = false; ++i; continue;
        }

        if (c=='+'||c=='-'||c=='*'||c=='/'||c=='^') {
            while (!operators.isEmpty() && operators.top() != '(' &&
                   precedence(operators.top()) >= precedence(c) && c != '^')
                if (!applyTop()) return std::numeric_limits<double>::quiet_NaN();
            operators.push(c);
            expectUnary = true; ++i; continue;
        }
        return std::numeric_limits<double>::quiet_NaN();
    }

    while (!operators.isEmpty())
        if (!applyTop()) return std::numeric_limits<double>::quiet_NaN();

    if (output.size() != 1) return std::numeric_limits<double>::quiet_NaN();
    ok = true;
    return output.top();
}

// ─── Chain line builder ───────────────────────────────────────────────────────

QString FormulaEngine::buildChainLine(const FormulaDefinition& formula,
                                       const QMap<QString, double>& values,
                                       double result)
{
    // e.g.  "U = I × R = 8 mA × 3 kΩ = 24.0 V"
    // Use chain_formula if available, else use formula name
    QString base = formula.chain_formula.isEmpty() ? formula.name : formula.chain_formula;

    // Build substituted version
    QString substituted;
    if (!formula.display_template.isEmpty()) {
        substituted = formula.display_template;
        for (const auto& var : formula.variables) {
            if (values.contains(var.name)) {
                QString valStr = formatSI(values[var.name], var.unit);
                substituted.replace("{" + var.name + "}", valStr);
            }
        }
    }

    QString resultStr = formatSI(result, formula.result_unit);

    if (!substituted.isEmpty() && substituted != base)
        return base + " = " + substituted + " = " + resultStr;
    else
        return base + " = " + resultStr;
}

// ─── Step builder ─────────────────────────────────────────────────────────────

QStringList FormulaEngine::buildSteps(const FormulaDefinition& formula,
                                       const QMap<QString, double>& values,
                                       double result)
{
    QStringList steps;
    steps << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";
    steps << "  Formel:  " + formula.name;
    if (!formula.chain_formula.isEmpty())
        steps << "  Aufstell.: " + formula.chain_formula;
    steps << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";
    steps << "";
    steps << "Eingabewerte:";
    for (const auto& var : formula.variables) {
        if (values.contains(var.name)) {
            steps << QString("  %1 = %2  [%3]")
                        .arg(var.name, -6)
                        .arg(formatSI(values[var.name], var.unit), -14)
                        .arg(var.description);
        }
    }
    steps << "";
    steps << "Umformung & Rechenweg:";
    steps << "──────────────────────────────────────";

    for (int s = 0; s < formula.steps.size(); ++s) {
        QString step = formula.steps[s];
        for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
            step.replace("{" + it.key() + "}", QString::number(it.value(),'g',6));
        }
        step.replace("{result}", QString::number(result,'g',6));

        // Common intermediate values
        auto rep = [&](const QString& key, double val) {
            step.replace("{"+key+"}", QString::number(val,'g',6));
        };
        if (values.contains("I"))  rep("I_sq", values["I"]*values["I"]);
        if (values.contains("U"))  rep("U_sq", values["U"]*values["U"]);
        if (values.contains("P") && values.contains("R")) rep("P_times_R", values["P"]*values["R"]);
        if (values.contains("R1")) rep("inv_R1", 1.0/values["R1"]);
        if (values.contains("R2")) rep("inv_R2", 1.0/values["R2"]);
        if (values.contains("R1") && values.contains("R2"))
            rep("inv_sum", 1.0/values["R1"]+1.0/values["R2"]);
        if (values.contains("C1")) rep("inv_C1", 1.0/values["C1"]);
        if (values.contains("C2")) rep("inv_C2", 1.0/values["C2"]);
        if (values.contains("C1") && values.contains("C2"))
            rep("inv_sum", 1.0/values["C1"]+1.0/values["C2"]);
        if (values.contains("L") && values.contains("C")) {
            double lc = values["L"]*values["C"];
            rep("LC_product", lc);
            rep("sqrt_LC", std::sqrt(lc));
        }
        if (values.contains("Rf") && values.contains("R1"))
            rep("ratio", values["Rf"]/values["R1"]);
        if (values.contains("UB") && values.contains("UBE"))
            rep("U_diff", values["UB"]-values["UBE"]);
        if (values.contains("alpha") && values.contains("dT"))
            rep("alpha_dT", values["alpha"]*values["dT"]);

        steps << QString("  %1. %2").arg(s+1).arg(step);
    }

    steps << "";
    steps << "──────────────────────────────────────";
    steps << QString("  %1 = %2")
                .arg(formula.solve_for)
                .arg(formatSI(result, formula.result_unit));
    steps << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";

    return steps;
}

// ─── Main calculate ───────────────────────────────────────────────────────────

CalculationResult FormulaEngine::calculate(const FormulaDefinition& formula,
                                             const QMap<QString, double>& values)
{
    CalculationResult res;
    res.success = false;

    for (const auto& var : formula.variables) {
        if (!values.contains(var.name)) {
            res.error = "Fehlender Wert: " + var.name;
            return res;
        }
    }

    bool ok;
    double result = evaluateExpression(formula.expression, values, ok);

    if (!ok || std::isnan(result) || std::isinf(result)) {
        res.error = "Berechnungsfehler (Division durch 0 oder ungültige Werte)";
        return res;
    }

    res.success = true;
    res.value = result;
    res.result_unit = formula.result_unit;
    res.result_description = formula.result_description;
    res.steps = buildSteps(formula, values, result);
    res.chain_line = buildChainLine(formula, values, result);
    return res;
}
