#pragma once
#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QTextBrowser>
#include <QLabel>
#include <QVector>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QListWidget>
#include <QFrame>
#include <QToolBar>
#include <QStatusBar>
#include "formula_engine.h"
#include "circuit_widget.h"
#include "formula_input_widget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onCategoryChanged(int idx);
    void onFormulaChanged(int idx);
    void onCalculate();
    void onClear();
    void onAddFormula();
    void onSaveCustomFormulas();
    void onAbout();
    void onCopyResult();
    void onExportResult();
    void onToggleSteps();
    void onToggleCompact();
    void onToggleLightCircuit();
    void onResultReady(const CalculationResult& res);
    void onShowTempTable();
    void onLicenseInfo();

private:
    void setupUi();
    void applyGlobalStyle();
    void populateCategories();
    void populateFormulas(const QString& categoryId);
    void showResult(const CalculationResult& res);
    void showError(const QString& msg);

    QListWidget*        m_categoryList;
    QComboBox*          m_categoryCombo;
    QComboBox*          m_formulaCombo;
    FormulaInputWidget* m_formulaWidget;
    QScrollArea*        m_formulaScroll;
    QLabel*             m_bigResult;
    QLabel*             m_chainLine;
    QLabel*             m_resultDesc;
    QFrame*             m_resultBox;
    QTextBrowser*       m_stepsView;
    QPushButton*        m_toggleStepsBtn;
    bool                m_stepsVisible = true;
    CircuitWidget*      m_circuitView;
    QWidget*            m_circuitContainer;
    QPushButton*        m_lightCircuitBtn;
    bool                m_circuitLight = false;
    QPushButton*        m_calcBtn;
    QPushButton*        m_clearBtn;
    QPushButton*        m_compactBtn;
    bool                m_compact = false;
    QStatusBar*         m_statusBar;
    QLabel*             m_statusLabel;
    QString             m_currentCategoryId;
    QString             m_currentFormulaId;
    CalculationResult   m_lastResult;
};
