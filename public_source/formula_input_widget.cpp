#include "formula_input_widget.h"
#include <QRegularExpression>
#include <QSizePolicy>
#include <cmath>

// ─── Per-field stylesheet helpers ────────────────────────────────────────────

QString FormulaInputWidget::fieldStyle(const QString& state)
{
    const QString base =
        "QLineEdit {"
        "  border-radius:4px; font-size:14px; font-weight:bold;"
        "  padding:2px 8px; min-height:32px;"
        "} ";

    if (state == "valid")
        return base +
            "QLineEdit { background:#1a2e1a; border:2px solid #2d8c2d; color:#a8ffa8; }";

    if (state == "invalid")
        return base +
            "QLineEdit { background:#2e1a1a; border:2px solid #8c2d2d; color:#ffa8a8; }";

    // default
    return base +
        "QLineEdit { background:#1e2a3a; border:2px solid #3a5a8a; color:#dde8ff; }"
        "QLineEdit:focus { border-color:#00aaff; background:#1a2840; }";
}

// ─── Constructor ──────────────────────────────────────────────────────────────

FormulaInputWidget::FormulaInputWidget(QWidget* parent)
    : QWidget(parent)
    , m_hLayout(new QHBoxLayout(this))
{
    m_hLayout->setContentsMargins(8, 6, 8, 6);
    m_hLayout->setSpacing(4);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMinimumHeight(46);
}

// ─── Public API ──────────────────────────────────────────────────────────────

void FormulaInputWidget::setFormula(const FormulaDefinition& formula)
{
    m_formula    = formula;
    m_lastResult = CalculationResult{};
    rebuild(formula);
}

void FormulaInputWidget::clear()
{
    for (const auto& fe : m_fields) {
        fe.edit->blockSignals(true);
        fe.edit->clear();
        fe.edit->setStyleSheet(fieldStyle("default"));
        fe.edit->blockSignals(false);
    }
    if (m_resultLabel) {
        m_resultLabel->setText("—");
        m_resultLabel->setStyleSheet("color:#555; font-size:18px; font-weight:bold;");
    }
    m_lastResult = CalculationResult{};
}

bool FormulaInputWidget::allFieldsValid() const
{
    if (m_fields.isEmpty()) return false;
    for (const auto& fe : m_fields) {
        QString t = fe.edit->text().trimmed();
        if (t.isEmpty()) return false;
        bool ok;
        FormulaEngine::parseSIValue(t, ok);
        if (!ok) return false;
    }
    return true;
}

QMap<QString, double> FormulaInputWidget::currentValues() const
{
    QMap<QString, double> vals;
    for (const auto& fe : m_fields) {
        bool ok;
        double v = FormulaEngine::parseSIValue(fe.edit->text().trimmed(), ok);
        if (!ok) return {};
        vals[fe.varName] = v;
    }
    return vals;
}

// ─── Widget construction ─────────────────────────────────────────────────────

void FormulaInputWidget::clearWidgets()
{
    while (m_hLayout->count()) {
        QLayoutItem* item = m_hLayout->takeAt(0);
        if (item->widget()) { item->widget()->hide(); delete item->widget(); }
        delete item;
    }
    m_fields.clear();
    m_resultLabel     = nullptr;
    m_resultUnitLabel = nullptr;
}

void FormulaInputWidget::rebuild(const FormulaDefinition& formula)
{
    clearWidgets();

    // Use display_template; fall back to "solve_for = {v1} · {v2} ..."
    QString tmpl = formula.display_template;
    if (tmpl.isEmpty()) {
        tmpl = formula.solve_for + " = ";
        for (int i = 0; i < formula.variables.size(); ++i) {
            if (i > 0) tmpl += " · ";
            tmpl += "{" + formula.variables[i].name + "}";
        }
    }

    // Build variable info map
    QMap<QString, FormulaVariable> varMap;
    for (const auto& v : formula.variables)
        varMap[v.name] = v;

    // Parse template: split on {varname} tokens
    QRegularExpression varRe(R"(\{([^}]+)\})");
    auto it  = varRe.globalMatch(tmpl);
    int  pos = 0;

    while (it.hasNext()) {
        auto m = it.next();

        // Static text before this variable
        if (m.capturedStart() > pos) {
            QString txt = tmpl.mid(pos, m.capturedStart() - pos);
            auto* lbl = new QLabel(txt);
            lbl->setStyleSheet("color:#c0c0e0; font-size:15px;");
            lbl->setTextInteractionFlags(Qt::NoTextInteraction);
            m_hLayout->addWidget(lbl);
        }

        // Variable input field
        QString varName = m.captured(1);
        const FormulaVariable& var = varMap[varName];

        auto* le = new QLineEdit;
        le->setPlaceholderText(varName);
        le->setFixedWidth(90);
        le->setAlignment(Qt::AlignCenter);
        le->setToolTip(QString("%1\nEinheit: %2\nSI-Präfixe: p n µ m k M G T  (z. B. 8mA, 3kΩ)")
                        .arg(var.description)
                        .arg(var.unit.isEmpty() ? QString("—") : var.unit));
        le->setStyleSheet(fieldStyle("default"));

        connect(le, &QLineEdit::textChanged, this, &FormulaInputWidget::onFieldChanged);
        connect(le, &QLineEdit::returnPressed, this, &FormulaInputWidget::calculateRequested);

        m_fields.append({varName, le});
        m_hLayout->addWidget(le);

        // Unit label right after the field
        if (!var.unit.isEmpty()) {
            auto* ulbl = new QLabel(var.unit);
            ulbl->setStyleSheet("color:#ffd700; font-size:13px; font-weight:bold; margin-right:2px;");
            m_hLayout->addWidget(ulbl);
        }

        pos = m.capturedEnd();
    }

    // Remaining static text after last {var}
    if (pos < tmpl.length()) {
        auto* lbl = new QLabel(tmpl.mid(pos));
        lbl->setStyleSheet("color:#c0c0e0; font-size:15px;");
        m_hLayout->addWidget(lbl);
    }

    // " = " separator + result display
    auto* eqLbl = new QLabel("  =");
    eqLbl->setStyleSheet("color:#7a7a9a; font-size:15px; font-weight:bold;");
    m_hLayout->addWidget(eqLbl);

    m_resultLabel = new QLabel("—");
    m_resultLabel->setMinimumWidth(70);
    m_resultLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_resultLabel->setStyleSheet("color:#555; font-size:20px; font-weight:bold;");
    m_hLayout->addWidget(m_resultLabel);

    if (!formula.result_unit.isEmpty()) {
        m_resultUnitLabel = new QLabel(formula.result_unit);
        m_resultUnitLabel->setStyleSheet("color:#ffd700; font-size:13px; font-weight:bold; margin-left:2px;");
        m_hLayout->addWidget(m_resultUnitLabel);
    }

    m_hLayout->addStretch(1);
}

// ─── Auto-calculation ────────────────────────────────────────────────────────

void FormulaInputWidget::onFieldChanged()
{
    // Per-field visual feedback
    for (const auto& fe : m_fields) {
        QString t = fe.edit->text().trimmed();
        if (t.isEmpty()) {
            fe.edit->setStyleSheet(fieldStyle("default"));
        } else {
            bool ok;
            FormulaEngine::parseSIValue(t, ok);
            fe.edit->setStyleSheet(fieldStyle(ok ? "valid" : "invalid"));
        }
    }

    if (!allFieldsValid()) {
        if (m_resultLabel) {
            m_resultLabel->setText("—");
            m_resultLabel->setStyleSheet("color:#555; font-size:20px; font-weight:bold;");
        }
        CalculationResult empty;
        empty.success = false;
        m_lastResult = empty;
        emit resultReady(empty);
        return;
    }

    m_lastResult = FormulaEngine::calculate(m_formula, currentValues());

    if (m_lastResult.success && m_resultLabel) {
        m_resultLabel->setText(FormulaEngine::formatSICompact(m_lastResult.value));
        m_resultLabel->setStyleSheet("color:#00ee88; font-size:20px; font-weight:bold;");
    } else if (!m_lastResult.success && m_resultLabel) {
        m_resultLabel->setText("Err");
        m_resultLabel->setStyleSheet("color:#ff4444; font-size:20px; font-weight:bold;");
    }

    emit resultReady(m_lastResult);
}
