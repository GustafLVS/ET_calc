#include "custom_formula_dialog.h"
#include "formula_library.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>
#include <QScrollArea>
#include <QDialogButtonBox>
#include <QUuid>

CustomFormulaDialog::CustomFormulaDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Neue Formel hinzufügen");
    setMinimumSize(640, 580);
    setupUi();
    applyStyle();
}

void CustomFormulaDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Scroll area for content
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    auto* content = new QWidget;
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(10);

    // ── Category ──────────────────────────────────────────────────
    auto* catGroup = new QGroupBox("Kategorie");
    auto* catLayout = new QFormLayout(catGroup);

    m_categoryCombo = new QComboBox;
    m_categoryCombo->addItem("-- Neue Kategorie --", "");
    for (const auto& cat : FormulaLibrary::instance().categories())
        m_categoryCombo->addItem(cat.name, cat.id);
    catLayout->addRow("Kategorie:", m_categoryCombo);

    m_newCategoryId   = new QLineEdit; m_newCategoryId->setPlaceholderText("z.B. my_formulas");
    m_newCategoryName = new QLineEdit; m_newCategoryName->setPlaceholderText("z.B. Meine Formeln");
    catLayout->addRow("Neue Kategorie ID:", m_newCategoryId);
    catLayout->addRow("Neue Kategorie Name:", m_newCategoryName);

    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int idx) {
                bool isNew = m_categoryCombo->itemData(idx).toString().isEmpty();
                m_newCategoryId->setEnabled(isNew);
                m_newCategoryName->setEnabled(isNew);
            });
    m_newCategoryId->setEnabled(true);
    m_newCategoryName->setEnabled(true);
    contentLayout->addWidget(catGroup);

    // ── Formula metadata ──────────────────────────────────────────
    auto* fGroup = new QGroupBox("Formel-Definition");
    auto* fLayout = new QFormLayout(fGroup);

    m_formulaId          = new QLineEdit; m_formulaId->setPlaceholderText("eindeutige_id (automatisch wenn leer)");
    m_formulaName        = new QLineEdit; m_formulaName->setPlaceholderText("z.B. Leistung P = U × I");
    m_solveFor           = new QLineEdit; m_solveFor->setPlaceholderText("z.B. P");
    m_expression         = new QLineEdit; m_expression->setPlaceholderText("z.B. U * I  oder  sqrt(P * R)");
    m_resultUnit         = new QLineEdit; m_resultUnit->setPlaceholderText("z.B. W");
    m_resultDescription  = new QLineEdit; m_resultDescription->setPlaceholderText("z.B. Leistung");

    fLayout->addRow("Formel-ID (optional):", m_formulaId);
    fLayout->addRow("Name / Anzeige:", m_formulaName);
    fLayout->addRow("Berechnet:", m_solveFor);
    fLayout->addRow("Ausdruck:", m_expression);
    fLayout->addRow("Ergebnis-Einheit:", m_resultUnit);
    fLayout->addRow("Ergebnis-Beschreibung:", m_resultDescription);

    auto* exprHelp = new QLabel(
        "<small style='color:#888;'>Unterstützte Operatoren: + − * / ^ sqrt(x) "
        "Variablennamen aus der Tabelle unten verwenden.</small>");
    exprHelp->setWordWrap(true);
    fLayout->addRow("", exprHelp);
    contentLayout->addWidget(fGroup);

    // ── Variables ─────────────────────────────────────────────────
    auto* varGroup = new QGroupBox("Variablen");
    auto* varLayout = new QVBoxLayout(varGroup);

    m_variableTable = new QTableWidget(0, 3);
    m_variableTable->setHorizontalHeaderLabels({"Name", "Einheit", "Beschreibung"});
    m_variableTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_variableTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_variableTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_variableTable->setMinimumHeight(110);

    auto* varBtnLayout = new QHBoxLayout;
    auto* addVarBtn = new QPushButton("+ Variable");
    auto* rmVarBtn  = new QPushButton("− Entfernen");
    varBtnLayout->addWidget(addVarBtn);
    varBtnLayout->addWidget(rmVarBtn);
    varBtnLayout->addStretch();

    connect(addVarBtn, &QPushButton::clicked, this, &CustomFormulaDialog::addVariable);
    connect(rmVarBtn,  &QPushButton::clicked, this, &CustomFormulaDialog::removeVariable);

    varLayout->addWidget(m_variableTable);
    varLayout->addLayout(varBtnLayout);
    contentLayout->addWidget(varGroup);

    // ── Steps ─────────────────────────────────────────────────────
    auto* stepsGroup = new QGroupBox("Rechenschritte (optional, eine Zeile pro Schritt)");
    auto* stepsLayout = new QVBoxLayout(stepsGroup);

    m_stepsEdit = new QTextEdit;
    m_stepsEdit->setPlaceholderText(
        "Formel: P = U × I\n"
        "Einsetzen: P = {U} V × {I} A\n"
        "Berechnung: P = {result} W\n\n"
        "Platzhalter: {VariableName}, {result}");
    m_stepsEdit->setMaximumHeight(100);
    stepsLayout->addWidget(m_stepsEdit);
    contentLayout->addWidget(stepsGroup);

    // ── Test ──────────────────────────────────────────────────────
    auto* testGroup = new QGroupBox("Formel testen");
    auto* testLayout = new QVBoxLayout(testGroup);

    m_testBtn = new QPushButton("Testen (Testwerte = 1 für jede Variable)");
    connect(m_testBtn, &QPushButton::clicked, this, &CustomFormulaDialog::testFormula);

    m_testOutput = new QTextEdit;
    m_testOutput->setReadOnly(true);
    m_testOutput->setMaximumHeight(80);
    m_testOutput->setPlaceholderText("Testergebnis erscheint hier...");

    testLayout->addWidget(m_testBtn);
    testLayout->addWidget(m_testOutput);
    contentLayout->addWidget(testGroup);

    scroll->setWidget(content);
    mainLayout->addWidget(scroll);

    // ── Buttons ───────────────────────────────────────────────────
    auto* btnBox = new QDialogButtonBox;
    m_okBtn = btnBox->addButton("Formel speichern", QDialogButtonBox::AcceptRole);
    btnBox->addButton("Abbrechen", QDialogButtonBox::RejectRole);
    connect(m_okBtn, &QPushButton::clicked, this, &CustomFormulaDialog::onAccept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(btnBox);

    // Pre-add two variable rows
    addVariable();
    addVariable();
}

void CustomFormulaDialog::applyStyle() {
    setStyleSheet(R"(
        QDialog { background: #1a1a2e; color: #e0e0e0; }
        QGroupBox {
            border: 1px solid #3a3a5c;
            border-radius: 6px;
            margin-top: 8px;
            padding: 8px;
            color: #00d4ff;
            font-weight: bold;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 8px; }
        QLineEdit, QComboBox, QTextEdit, QTableWidget {
            background: #0f0f23;
            border: 1px solid #3a3a5c;
            border-radius: 4px;
            color: #e0e0e0;
            padding: 4px;
        }
        QLineEdit:focus, QComboBox:focus, QTextEdit:focus {
            border: 1px solid #00d4ff;
        }
        QHeaderView::section {
            background: #16213e;
            color: #00d4ff;
            border: none;
            padding: 4px;
        }
        QPushButton {
            background: #16213e;
            border: 1px solid #3a3a5c;
            border-radius: 4px;
            color: #e0e0e0;
            padding: 6px 12px;
        }
        QPushButton:hover { background: #0e3460; border-color: #00d4ff; }
        QScrollArea { border: none; }
        QLabel { color: #e0e0e0; }
    )");
}

void CustomFormulaDialog::addVariable() {
    int row = m_variableTable->rowCount();
    m_variableTable->insertRow(row);
    m_variableTable->setItem(row, 0, new QTableWidgetItem(""));
    m_variableTable->setItem(row, 1, new QTableWidgetItem(""));
    m_variableTable->setItem(row, 2, new QTableWidgetItem(""));
}

void CustomFormulaDialog::removeVariable() {
    int row = m_variableTable->currentRow();
    if (row >= 0) m_variableTable->removeRow(row);
}

void CustomFormulaDialog::testFormula() {
    FormulaDefinition f = getFormula();
    if (f.expression.isEmpty()) {
        m_testOutput->setPlainText("Kein Ausdruck definiert.");
        return;
    }

    QMap<QString, double> testVals;
    for (const auto& v : f.variables) testVals[v.name] = 1.0;

    auto res = FormulaEngine::calculate(f, testVals);
    if (res.success)
        m_testOutput->setPlainText(QString("✓ Test OK: %1 = %2 %3")
            .arg(f.solve_for).arg(res.value).arg(f.result_unit));
    else
        m_testOutput->setPlainText("✗ Fehler: " + res.error);
}

void CustomFormulaDialog::onAccept() {
    if (m_formulaName->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Fehler", "Bitte einen Formelnamen eingeben.");
        return;
    }
    if (m_expression->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Fehler", "Bitte einen Ausdruck eingeben.");
        return;
    }
    if (m_solveFor->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Fehler", "Bitte 'Berechnet' ausfüllen.");
        return;
    }
    if (m_variableTable->rowCount() == 0) {
        QMessageBox::warning(this, "Fehler", "Mindestens eine Variable erforderlich.");
        return;
    }
    accept();
}

FormulaDefinition CustomFormulaDialog::getFormula() const {
    FormulaDefinition f;
    f.id = m_formulaId->text().trimmed();
    if (f.id.isEmpty())
        f.id = "custom_" + QUuid::createUuid().toString(QUuid::Id128).left(8);
    f.name = m_formulaName->text().trimmed();
    f.solve_for = m_solveFor->text().trimmed();
    f.expression = m_expression->text().trimmed();
    f.result_unit = m_resultUnit->text().trimmed();
    f.result_description = m_resultDescription->text().trimmed();

    for (int r = 0; r < m_variableTable->rowCount(); ++r) {
        auto* nameItem = m_variableTable->item(r, 0);
        auto* unitItem = m_variableTable->item(r, 1);
        auto* descItem = m_variableTable->item(r, 2);
        QString vname = nameItem ? nameItem->text().trimmed() : "";
        if (vname.isEmpty()) continue;
        FormulaVariable var;
        var.name = vname;
        var.unit = unitItem ? unitItem->text().trimmed() : "";
        var.description = descItem ? descItem->text().trimmed() : "";
        f.variables.append(var);
    }

    QString stepsText = m_stepsEdit->toPlainText().trimmed();
    if (!stepsText.isEmpty())
        f.steps = stepsText.split('\n');

    return f;
}

QString CustomFormulaDialog::getCategoryId() const {
    QString id = m_categoryCombo->currentData().toString();
    if (id.isEmpty()) return m_newCategoryId->text().trimmed();
    return id;
}

QString CustomFormulaDialog::getCategoryName() const {
    if (!m_categoryCombo->currentData().toString().isEmpty())
        return m_categoryCombo->currentText();
    return m_newCategoryName->text().trimmed();
}
