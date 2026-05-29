#include "mainwindow.h"
#include "formula_library.h"
#include "formula_engine.h"
#include "custom_formula_dialog.h"
#include "temp_coeff_dialog.h"
#include "license_manager.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QSplitter>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>
#include <QFileDialog>
#include <QClipboard>
#include <QDate>
#include <QFont>
#include <QPalette>
#include <QFrame>
#include <QSizePolicy>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("ET-Rechner â€” Elektrotechnik Formelrechner");
    setMinimumSize(1100, 720);
    resize(1360, 860);

    setupUi();
    applyGlobalStyle();

    FormulaLibrary::instance().loadFromResource();
    populateCategories();

    // Show license expiry info in status bar
    const int daysLeft = LicenseManager::instance().daysRemaining();
    if (daysLeft <= 2) {
        m_statusLabel->setText(
            QString("âš  Testversion: noch %1 Tag(e) gÃ¼ltig").arg(daysLeft));
        m_statusLabel->setStyleSheet("color:#ff8040;");
    } else {
        m_statusLabel->setText(
            QString("Testversion â€” noch %1 Tag(e) gÃ¼ltig").arg(daysLeft));
        m_statusLabel->setStyleSheet("color:#00aa66;");
    }
}

// â”€â”€â”€ Dark-grey theme â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void MainWindow::applyGlobalStyle()
{
    qApp->setStyle("Fusion");

    QPalette dark;
    dark.setColor(QPalette::Window,          QColor("#1a1a1a"));
    dark.setColor(QPalette::WindowText,      QColor("#e0e0e0"));
    dark.setColor(QPalette::Base,            QColor("#242424"));
    dark.setColor(QPalette::AlternateBase,   QColor("#2e2e2e"));
    dark.setColor(QPalette::Text,            QColor("#e0e0e0"));
    dark.setColor(QPalette::Button,          QColor("#2e2e2e"));
    dark.setColor(QPalette::ButtonText,      QColor("#e0e0e0"));
    dark.setColor(QPalette::Highlight,       QColor("#1a4a7a"));
    dark.setColor(QPalette::HighlightedText, QColor("#00aaff"));
    dark.setColor(QPalette::ToolTipBase,     QColor("#2e2e2e"));
    dark.setColor(QPalette::ToolTipText,     QColor("#e0e0e0"));
    qApp->setPalette(dark);

    setStyleSheet(R"(
        QMainWindow  { background:#1a1a1a; }
        QMenuBar     { background:#141414; color:#c8c8c8; border-bottom:1px solid #333; }
        QMenuBar::item:selected { background:#2e2e2e; color:#00aaff; }
        QMenu        { background:#242424; color:#e0e0e0; border:1px solid #3a3a3a; }
        QMenu::item:selected { background:#1a4a7a; color:#00aaff; }

        QToolBar {
            background:#141414; border-bottom:1px solid #333; spacing:4px; padding:3px;
        }
        QToolButton {
            background:transparent; border:1px solid transparent; border-radius:4px;
            color:#b0b0b0; padding:4px 12px; font-size:13px;
        }
        QToolButton:hover  { background:#2e2e2e; border-color:#00aaff; color:#00aaff; }
        QToolButton:pressed{ background:#1a4a7a; }

        QGroupBox {
            border:1px solid #3a3a3a; border-radius:8px;
            margin-top:10px; padding:10px 8px 8px 8px;
            font-size:12px; font-weight:bold; color:#00aaff;
        }
        QGroupBox::title { subcontrol-origin:margin; left:10px; padding:0 4px; }

        QComboBox {
            background:#242424; border:1px solid #3a3a3a; border-radius:5px;
            color:#e0e0e0; padding:6px 10px; font-size:12px; min-height:28px;
        }
        QComboBox:hover  { border-color:#00aaff; }
        QComboBox:focus  { border-color:#00aaff; }
        QComboBox::drop-down { border:none; width:20px; }
        QComboBox QAbstractItemView {
            background:#242424; border:1px solid #3a3a3a; color:#e0e0e0;
            selection-background-color:#1a4a7a; selection-color:#00aaff;
        }

        QLineEdit {
            background:#242424; border:1px solid #3a3a3a; border-radius:5px;
            color:#fff; padding:6px 10px; font-size:13px; min-height:28px;
        }
        QLineEdit:hover { border-color:#555; }
        QLineEdit:focus { border-color:#00aaff; background:#2a2a2a; }

        QPushButton#calcBtn {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #0077cc,stop:1 #0055aa);
            border:none; border-radius:6px; color:#fff;
            font-size:14px; font-weight:bold; padding:10px 20px; min-height:36px;
        }
        QPushButton#calcBtn:hover  { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #0099ee,stop:1 #0066bb); }
        QPushButton#calcBtn:pressed{ background:#004488; }

        QPushButton#clearBtn {
            background:#2e1a1a; border:1px solid #5a2a2a; border-radius:6px;
            color:#ff7070; font-size:13px; padding:8px 16px; min-height:34px;
        }
        QPushButton#clearBtn:hover { background:#3a1a1a; border-color:#ff7070; }

        QPushButton {
            background:#2e2e2e; border:1px solid #3a3a3a; border-radius:5px;
            color:#d0d0d0; font-size:12px; padding:6px 14px;
        }
        QPushButton:hover  { background:#3a3a3a; border-color:#00aaff; color:#00aaff; }
        QPushButton:checked{ background:#1a4a7a; border-color:#00aaff; color:#00aaff; }

        QTextBrowser {
            background:#141414; border:1px solid #333; border-radius:6px;
            color:#e0e0e0; font-family:'Consolas','Courier New',monospace;
            font-size:12px; padding:8px;
        }

        QListWidget {
            background:#141414; border:none; border-right:1px solid #2a2a2a;
            color:#b0b0b0; font-size:12px; outline:none;
        }
        QListWidget::item { padding:8px 12px; border-bottom:1px solid #222; }
        QListWidget::item:hover   { background:#2a2a2a; color:#e0e0e0; }
        QListWidget::item:selected {
            background:#1a4a7a; color:#00aaff;
            font-weight:bold; border-left:3px solid #00aaff;
        }

        QSplitter::handle { background:#2a2a2a; }
        QSplitter::handle:horizontal { width:3px; }
        QSplitter::handle:vertical   { height:3px; }

        QScrollBar:vertical { background:#1a1a1a; width:8px; border-radius:4px; }
        QScrollBar::handle:vertical {
            background:#3a3a3a; border-radius:4px; min-height:20px;
        }
        QScrollBar::handle:vertical:hover { background:#00aaff; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0px; }

        QScrollBar:horizontal { background:#1a1a1a; height:8px; border-radius:4px; }
        QScrollBar::handle:horizontal {
            background:#3a3a3a; border-radius:4px; min-width:20px;
        }
        QScrollBar::handle:horizontal:hover { background:#00aaff; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width:0px; }

        QLabel  { color:#b0b0b0; font-size:12px; }
        QStatusBar { background:#141414; color:#666; border-top:1px solid #2a2a2a; }
    )");
}

// â”€â”€â”€ UI construction â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void MainWindow::setupUi()
{
    // â”€â”€ Menu â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    auto* fileMenu  = menuBar()->addMenu("&Datei");
    auto* saveAct   = fileMenu->addAction("Eigene Formeln speichern");
    auto* exportAct = fileMenu->addAction("Ergebnis exportieren...");
    fileMenu->addSeparator();
    fileMenu->addAction("Beenden", qApp, &QApplication::quit);
    connect(saveAct,   &QAction::triggered, this, &MainWindow::onSaveCustomFormulas);
    connect(exportAct, &QAction::triggered, this, &MainWindow::onExportResult);

    auto* fMenu  = menuBar()->addMenu("&Formeln");
    auto* addAct = fMenu->addAction("Neue Formel hinzufÃ¼gen...");
    connect(addAct, &QAction::triggered, this, &MainWindow::onAddFormula);

    auto* refMenu  = menuBar()->addMenu("&Referenz");
    refMenu->addAction("Temperaturkoeffizient Î± (20 Materialien)...",
                       this, &MainWindow::onShowTempTable);

    auto* helpMenu = menuBar()->addMenu("&Hilfe");
    helpMenu->addAction("Ãœber ET-Rechner", this, &MainWindow::onAbout);
    helpMenu->addSeparator();
    helpMenu->addAction("Lizenzinformationen", this, &MainWindow::onLicenseInfo);

    // â”€â”€ Toolbar â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    auto* tb = new QToolBar("Werkzeuge", this);
    tb->setMovable(false);
    addToolBar(tb);
    tb->addAction("âš¡ Berechnen",        this, &MainWindow::onCalculate);
    tb->addAction("âœ– LÃ¶schen",           this, &MainWindow::onClear);
    tb->addSeparator();
    tb->addAction("ï¼‹ Formel hinzufÃ¼gen", this, &MainWindow::onAddFormula);
    tb->addSeparator();
    tb->addAction("ðŸ“‹ Kopieren",          this, &MainWindow::onCopyResult);

    // â”€â”€ Status bar â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    m_statusBar   = statusBar();
    m_statusLabel = new QLabel("Bereit");
    m_statusBar->addWidget(m_statusLabel);

    // â”€â”€ Central area â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    auto* central     = new QWidget;
    setCentralWidget(central);
    auto* outerLayout = new QHBoxLayout(central);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // Left sidebar
    {
        auto* hdr = new QLabel("  Kategorien");
        hdr->setFixedHeight(32);
        hdr->setStyleSheet("background:#141414; color:#00aaff; font-weight:bold;"
                           "font-size:13px; border-bottom:1px solid #2a2a2a;");

        m_categoryList = new QListWidget;
        m_categoryList->setFixedWidth(190);

        auto* side = new QWidget;
        side->setFixedWidth(190);
        side->setStyleSheet("background:#141414;");
        auto* sl = new QVBoxLayout(side);
        sl->setContentsMargins(0, 0, 0, 0);
        sl->setSpacing(0);
        sl->addWidget(hdr);
        sl->addWidget(m_categoryList);
        outerLayout->addWidget(side);
    }

    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::VLine);
    sep->setStyleSheet("background:#2a2a2a; border:none; max-width:1px;");
    outerLayout->addWidget(sep);

    // Main area (right of sidebar)
    auto* mainArea   = new QWidget;
    auto* mainLayout = new QVBoxLayout(mainArea);
    mainLayout->setContentsMargins(12, 10, 12, 8);
    mainLayout->setSpacing(8);

    // Formula selector row
    {
        auto* grp = new QGroupBox("Formelauswahl");
        auto* gl  = new QHBoxLayout(grp);
        gl->setSpacing(10);

        auto* catLbl = new QLabel("Kategorie:");
        catLbl->setStyleSheet("color:#777; font-size:11px;");
        m_categoryCombo = new QComboBox;
        m_categoryCombo->setMinimumWidth(220);

        auto* fLbl = new QLabel("Formel:");
        fLbl->setStyleSheet("color:#777; font-size:11px;");
        m_formulaCombo = new QComboBox;
        m_formulaCombo->setMinimumWidth(320);

        gl->addWidget(catLbl);
        gl->addWidget(m_categoryCombo);
        gl->addSpacing(16);
        gl->addWidget(fLbl);
        gl->addWidget(m_formulaCombo, 1);
        mainLayout->addWidget(grp);
    }

    // â”€â”€ Horizontal splitter: left=calc area, right=circuit â”€â”€â”€â”€
    auto* hSplit = new QSplitter(Qt::Horizontal);
    hSplit->setHandleWidth(4);

    // â”€â”€ Left panel â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    auto* leftPanel  = new QWidget;
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(8);

    // â”€â”€ Inline formula widget (auto-height, horizontal scroll) â”€â”€
    {
        auto* grp    = new QGroupBox("Formel");
        auto* grpLay = new QVBoxLayout(grp);
        grpLay->setContentsMargins(6, 6, 6, 6);

        m_formulaScroll = new QScrollArea;
        m_formulaScroll->setWidgetResizable(true);
        m_formulaScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_formulaScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_formulaScroll->setFrameShape(QFrame::NoFrame);
        m_formulaScroll->setStyleSheet("background:transparent;");
        m_formulaScroll->setMinimumHeight(48);
        m_formulaScroll->setMaximumHeight(80);

        m_formulaWidget = new FormulaInputWidget;
        m_formulaScroll->setWidget(m_formulaWidget);

        grpLay->addWidget(m_formulaScroll);
        leftLayout->addWidget(grp);
    }

    // â”€â”€ Result box â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    {
        m_resultBox = new QFrame;
        m_resultBox->setFrameShape(QFrame::StyledPanel);
        m_resultBox->setStyleSheet(
            "QFrame { background:#1a2a1a; border:1px solid #2d5a2d;"
            "border-radius:8px; }");

        auto* rlay = new QVBoxLayout(m_resultBox);
        rlay->setContentsMargins(14, 8, 14, 8);
        rlay->setSpacing(2);

        m_bigResult = new QLabel("â€”");
        m_bigResult->setAlignment(Qt::AlignCenter);
        m_bigResult->setStyleSheet("color:#00ee88; font-size:32px; font-weight:bold;");
        rlay->addWidget(m_bigResult);

        m_chainLine = new QLabel;
        m_chainLine->setAlignment(Qt::AlignCenter);
        m_chainLine->setStyleSheet("color:#ffd700; font-size:13px; font-family:Consolas,monospace;");
        m_chainLine->setWordWrap(true);
        rlay->addWidget(m_chainLine);

        m_resultDesc = new QLabel;
        m_resultDesc->setAlignment(Qt::AlignCenter);
        m_resultDesc->setStyleSheet("color:#5a8a5a; font-size:10px;");
        rlay->addWidget(m_resultDesc);

        leftLayout->addWidget(m_resultBox);
    }

    // â”€â”€ Step-by-step (BELOW result, ABOVE buttons) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    {
        auto* grp    = new QGroupBox("Rechenweg");
        auto* grpLay = new QVBoxLayout(grp);
        grpLay->setContentsMargins(6, 6, 6, 6);
        grpLay->setSpacing(4);

        m_stepsView = new QTextBrowser;
        m_stepsView->setOpenLinks(false);
        m_stepsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_stepsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        grpLay->addWidget(m_stepsView, 1);
        leftLayout->addWidget(grp, 1);
        grp->setObjectName("stepsGroup");
    }

    // â”€â”€ Action buttons â€” ALWAYS at the very bottom â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    {
        auto* btnRow = new QHBoxLayout;
        btnRow->setSpacing(6);

        m_calcBtn = new QPushButton("âš¡  Berechnen");
        m_calcBtn->setObjectName("calcBtn");
        m_calcBtn->setMinimumHeight(38);

        m_clearBtn = new QPushButton("âœ–  LÃ¶schen");
        m_clearBtn->setObjectName("clearBtn");
        m_clearBtn->setMinimumHeight(38);

        auto* copyBtn = new QPushButton("ðŸ“‹ Kopieren");
        copyBtn->setFixedHeight(34);
        connect(copyBtn, &QPushButton::clicked, this, &MainWindow::onCopyResult);

        m_toggleStepsBtn = new QPushButton("â–²  Schritte");
        m_toggleStepsBtn->setCheckable(true);
        m_toggleStepsBtn->setChecked(true);
        m_toggleStepsBtn->setFixedHeight(34);

        m_compactBtn = new QPushButton("âŠ¡  Kompakt");
        m_compactBtn->setCheckable(true);
        m_compactBtn->setFixedHeight(34);

        btnRow->addWidget(m_calcBtn,  3);
        btnRow->addWidget(m_clearBtn, 2);
        btnRow->addWidget(copyBtn,    2);
        btnRow->addWidget(m_toggleStepsBtn, 2);
        btnRow->addWidget(m_compactBtn,     1);
        leftLayout->addLayout(btnRow);
    }

    hSplit->addWidget(leftPanel);

    // â”€â”€ Right panel (circuit) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    {
        m_circuitContainer = new QWidget;
        auto* ccl = new QVBoxLayout(m_circuitContainer);
        ccl->setContentsMargins(0, 0, 0, 0);
        ccl->setSpacing(4);

        auto* grp    = new QGroupBox("Schaltungsdarstellung");
        auto* grpLay = new QVBoxLayout(grp);

        m_circuitView = new CircuitWidget;
        m_circuitView->setMinimumSize(320, 260);
        grpLay->addWidget(m_circuitView);

        m_lightCircuitBtn = new QPushButton("â˜€  Hell / Dunkel");
        m_lightCircuitBtn->setCheckable(true);
        m_lightCircuitBtn->setFixedHeight(28);
        grpLay->addWidget(m_lightCircuitBtn);

        ccl->addWidget(grp, 1);
        hSplit->addWidget(m_circuitContainer);
    }

    hSplit->setStretchFactor(0, 3);
    hSplit->setStretchFactor(1, 2);
    mainLayout->addWidget(hSplit, 1);
    outerLayout->addWidget(mainArea, 1);

    // â”€â”€ Signal connections â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    connect(m_categoryList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0) m_categoryCombo->setCurrentIndex(row);
    });
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onCategoryChanged);
    connect(m_formulaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFormulaChanged);
    connect(m_calcBtn,  &QPushButton::clicked, this, &MainWindow::onCalculate);
    connect(m_clearBtn, &QPushButton::clicked, this, &MainWindow::onClear);
    connect(m_toggleStepsBtn, &QPushButton::toggled, this, &MainWindow::onToggleSteps);
    connect(m_compactBtn,     &QPushButton::toggled, this, &MainWindow::onToggleCompact);
    connect(m_lightCircuitBtn,&QPushButton::toggled, this, &MainWindow::onToggleLightCircuit);
    connect(m_formulaWidget,  &FormulaInputWidget::resultReady,
            this, &MainWindow::onResultReady);
    connect(m_formulaWidget,  &FormulaInputWidget::calculateRequested,
            this, &MainWindow::onCalculate);
}

// â”€â”€â”€ Population â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void MainWindow::populateCategories()
{
    m_categoryList->blockSignals(true);
    m_categoryCombo->blockSignals(true);
    m_categoryList->clear();
    m_categoryCombo->clear();

    for (const auto& cat : FormulaLibrary::instance().categories()) {
        QString icon = "âš¡";
        if      (cat.id == "ohm")       icon = "Î©";
        else if (cat.id == "power")     icon = "P";
        else if (cat.id == "series")    icon = "âŠž";
        else if (cat.id == "parallel")  icon = "âŠŸ";
        else if (cat.id == "capacitor") icon = "C";
        else if (cat.id == "inductor")  icon = "L";
        else if (cat.id == "frequency") icon = "f";
        else if (cat.id == "bjt")       icon = "T";
        else if (cat.id == "opamp")     icon = "â–·";
        else if (cat.id == "basics")    icon = "I";
        else if (cat.id == "rho")       icon = "Ï";
        else if (cat.id == "rtemp")     icon = "Î±";

        m_categoryList->addItem(icon + "  " + cat.name);
        m_categoryCombo->addItem(cat.name, cat.id);
    }

    m_categoryList->blockSignals(false);
    m_categoryCombo->blockSignals(false);

    if (m_categoryCombo->count() > 0) {
        m_categoryList->setCurrentRow(0);
        m_categoryCombo->setCurrentIndex(0);
        onCategoryChanged(0);
    }
}

void MainWindow::populateFormulas(const QString& categoryId)
{
    m_formulaCombo->blockSignals(true);
    m_formulaCombo->clear();
    const auto* cat = FormulaLibrary::instance().findCategory(categoryId);
    if (cat) {
        for (const auto& f : cat->formulas)
            m_formulaCombo->addItem(f.name, f.id);
    }
    m_formulaCombo->blockSignals(false);

    if (m_formulaCombo->count() > 0)
        onFormulaChanged(0);
}

void MainWindow::onCategoryChanged(int idx)
{
    if (idx < 0) return;
    QString catId = m_categoryCombo->itemData(idx).toString();
    m_currentCategoryId = catId;

    m_categoryList->blockSignals(true);
    m_categoryList->setCurrentRow(idx);
    m_categoryList->blockSignals(false);

    const auto* cat = FormulaLibrary::instance().findCategory(catId);
    if (cat) m_circuitView->setCircuitType(cat->circuit_type);

    populateFormulas(catId);
}

void MainWindow::onFormulaChanged(int idx)
{
    if (idx < 0) return;
    m_currentFormulaId = m_formulaCombo->itemData(idx).toString();

    const auto* formula = FormulaLibrary::instance().findFormula(m_currentFormulaId);
    if (formula) {
        m_formulaWidget->setFormula(*formula);
        // Auto-fit the scroll area height to the formula widget's natural height
        int h = qBound(48, m_formulaWidget->sizeHint().height() + 10, 80);
        m_formulaScroll->setFixedHeight(h);
    }

    // Reset result display
    m_bigResult->setText("â€”");
    m_bigResult->setStyleSheet("color:#555; font-size:34px; font-weight:bold;");
    m_chainLine->clear();
    m_resultDesc->clear();
    m_stepsView->clear();
    m_circuitView->clear();
    m_lastResult = CalculationResult{};

    m_statusLabel->setText("Formel gewÃ¤hlt: " + m_formulaCombo->currentText());
}

// â”€â”€â”€ Auto-result slot (triggered by FormulaInputWidget) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void MainWindow::onResultReady(const CalculationResult& res)
{
    m_lastResult = res;

    if (!res.success) {
        m_bigResult->setText("â€”");
        m_bigResult->setStyleSheet("color:#555; font-size:34px; font-weight:bold;");
        m_chainLine->clear();
        m_resultDesc->clear();
        return;
    }

    // Big result
    QString valStr = FormulaEngine::formatSICompact(res.value);
    m_bigResult->setText(valStr + (res.result_unit.isEmpty() ? "" : " " + res.result_unit));
    m_bigResult->setStyleSheet("color:#00ee88; font-size:34px; font-weight:bold;");

    // Chain line
    m_chainLine->setText(res.chain_line);

    // Description
    m_resultDesc->setText(res.result_description.isEmpty()
                          ? "" : "âž¤ " + res.result_description);

    // Update circuit automatically
    const auto* formula = FormulaLibrary::instance().findFormula(m_currentFormulaId);
    if (formula) {
        m_circuitView->setValues(m_formulaWidget->currentValues(),
                                 formula->solve_for, res.value, res.result_unit);
    }

    m_statusLabel->setText(
        QString("âœ“  %1  â€”  %2")
            .arg(res.chain_line.isEmpty() ? valStr : res.chain_line)
            .arg(QDate::currentDate().toString("dd.MM.yyyy")));
}

// â”€â”€â”€ Explicit calculate (shows steps) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void MainWindow::onCalculate()
{
    if (!m_formulaWidget->allFieldsValid()) {
        // Trigger a re-evaluate to get error feedback on fields
        const auto* formula = FormulaLibrary::instance().findFormula(m_currentFormulaId);
        if (!formula) {
            showError("Keine Formel ausgewÃ¤hlt.");
            return;
        }
        showError("Bitte alle Felder mit gÃ¼ltigen Werten fÃ¼llen.\n"
                  "SI-PrÃ¤fixe erlaubt: z. B. 8mA, 3kÎ©, 470ÂµF");
        return;
    }

    const CalculationResult& res = m_formulaWidget->lastCalcResult();
    if (!res.success) {
        showError(res.error.isEmpty() ? "Berechnungsfehler" : res.error);
        return;
    }

    showResult(res);

    // Make steps visible
    if (!m_stepsVisible) {
        m_toggleStepsBtn->setChecked(true);
        // onToggleSteps() will be called via signal
    }
}

// â”€â”€â”€ Result rendering â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void MainWindow::showResult(const CalculationResult& res)
{
    QString html = "<html><body style='font-family:Consolas,monospace; font-size:12px; color:#e0e0e0; margin:4px;'>";

    for (const QString& line : res.steps) {
        QString esc = line.toHtmlEscaped();

        if (line.startsWith("â”")) {
            html += "<span style='color:#333;'>" + esc + "</span><br>";
        } else if (line.startsWith("â”€")) {
            html += "<span style='color:#2a2a2a;'>" + esc + "</span><br>";
        } else if (line.contains("Formel:") || line.contains("Aufstell.:")) {
            html += "<span style='color:#00aaff; font-weight:bold;'>" + esc + "</span><br>";
        } else if (line.contains("Eingabewerte:") || line.contains("Umformung")) {
            html += "<span style='color:#777;'>" + esc + "</span><br>";
        } else if (line.trimmed().isEmpty()) {
            html += "<br>";
        } else if (res.steps.indexOf(line) == res.steps.size() - 2) {
            // Second-to-last: the result line
            html += "<span style='color:#00ee88; font-size:13px; font-weight:bold;'>" + esc + "</span><br>";
        } else {
            html += "<span style='color:#cccccc;'>" + esc + "</span><br>";
        }
    }

    html += "</body></html>";
    m_stepsView->setHtml(html);
}

void MainWindow::showError(const QString& msg)
{
    m_stepsView->setHtml(
        QString("<html><body style='font-family:Consolas; font-size:12px;'>"
                "<span style='color:#ff5050; font-weight:bold;'>âœ— %1</span>"
                "</body></html>").arg(msg.toHtmlEscaped()));
    m_statusLabel->setText("Fehler: " + msg);
}

// â”€â”€â”€ Clear â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void MainWindow::onClear()
{
    m_formulaWidget->clear();
    m_bigResult->setText("â€”");
    m_bigResult->setStyleSheet("color:#555; font-size:34px; font-weight:bold;");
    m_chainLine->clear();
    m_resultDesc->clear();
    m_stepsView->clear();
    m_circuitView->clear();
    m_lastResult = CalculationResult{};
    m_statusLabel->setText("Eingaben gelÃ¶scht");
}

// â”€â”€â”€ Toggle steps â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void MainWindow::onToggleSteps()
{
    m_stepsVisible = m_toggleStepsBtn->isChecked();
    // Find the stepsGroup widget
    QWidget* stepsGroup = findChild<QGroupBox*>("stepsGroup");
    if (stepsGroup) stepsGroup->setVisible(m_stepsVisible);
    m_toggleStepsBtn->setText(m_stepsVisible ? "â–²  Schritte verbergen"
                                              : "â–¼  Schritte anzeigen");
}

// â”€â”€â”€ Compact mode â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void MainWindow::onToggleCompact()
{
    m_compact = m_compactBtn->isChecked();

    // Hide/show circuit panel and steps group
    m_circuitContainer->setVisible(!m_compact);
    QWidget* stepsGroup = findChild<QGroupBox*>("stepsGroup");
    if (stepsGroup) stepsGroup->setVisible(!m_compact && m_stepsVisible);

    if (m_compact) {
        resize(700, height());
        m_compactBtn->setText("âŠ   Normal");
    } else {
        resize(1360, height());
        m_compactBtn->setText("âŠ¡  Kompakt");
    }
}

// â”€â”€â”€ Circuit light/dark toggle â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void MainWindow::onToggleLightCircuit()
{
    m_circuitLight = m_lightCircuitBtn->isChecked();
    m_circuitView->setLightMode(m_circuitLight);
}

// â”€â”€â”€ Add custom formula â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void MainWindow::onAddFormula()
{
    CustomFormulaDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    FormulaDefinition f = dlg.getFormula();
    QString catId       = dlg.getCategoryId();
    QString catName     = dlg.getCategoryName();

    if (catId.isEmpty()) {
        QMessageBox::warning(this, "Fehler", "Bitte Kategorie-ID angeben.");
        return;
    }

    CategoryDefinition cat;
    cat.id           = catId;
    cat.name         = catName.isEmpty() ? catId : catName;
    cat.circuit_type = catId;
    FormulaLibrary::instance().addCustomFormula(cat, f);

    populateCategories();

    // Navigate to the new formula
    for (int i = 0; i < m_categoryCombo->count(); ++i) {
        if (m_categoryCombo->itemData(i).toString() == catId) {
            m_categoryCombo->setCurrentIndex(i);
            for (int j = 0; j < m_formulaCombo->count(); ++j) {
                if (m_formulaCombo->itemData(j).toString() == f.id) {
                    m_formulaCombo->setCurrentIndex(j);
                    break;
                }
            }
            break;
        }
    }

    QMessageBox::information(this, "Formel gespeichert",
        QString("Formel '%1' wurde hinzugefÃ¼gt.\n"
                "Datei â†’ Eigene Formeln speichern zum dauerhaften Speichern.")
        .arg(f.name));
}

void MainWindow::onSaveCustomFormulas()
{
    QString path = FormulaLibrary::instance().customFormulasPath();
    if (path.isEmpty())
        path = QFileDialog::getSaveFileName(this, "Formeln speichern", "", "JSON (*.json)");
    if (path.isEmpty()) return;

    if (FormulaLibrary::instance().saveToJson(path))
        QMessageBox::information(this, "Gespeichert", "Gespeichert:\n" + path);
    else
        QMessageBox::critical(this, "Fehler", "Speichern fehlgeschlagen.");
}

void MainWindow::onCopyResult()
{
    QString text = m_stepsView->toPlainText();
    if (text.isEmpty() && m_lastResult.success)
        text = m_lastResult.chain_line;
    if (!text.isEmpty()) {
        qApp->clipboard()->setText(text);
        m_statusLabel->setText("In Zwischenablage kopiert");
    }
}

void MainWindow::onExportResult()
{
    QString path = QFileDialog::getSaveFileName(this, "Ergebnis exportieren", "",
                                                "Textdatei (*.txt)");
    if (path.isEmpty()) return;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&f);
        out.setEncoding(QStringConverter::Utf8);
        out << m_stepsView->toPlainText();
        m_statusLabel->setText("Exportiert: " + path);
    }
}

void MainWindow::onShowTempTable()
{
    auto* dlg = new TempCoeffDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "Ãœber ET-Rechner",
        "<h2>ET-Rechner</h2>"
        "<p>Elektrotechnik Formelrechner v2.0</p>"
        "<p>Entwickelt mit Qt Widgets (C++17)</p>"
        "<hr>"
        "<p><b>Funktionen:</b></p>"
        "<ul>"
        "<li>Ohmsches Gesetz &amp; GrundgrÃ¶ÃŸen</li>"
        "<li>Elektrische Leistung</li>"
        "<li>Reihen- und Parallelschaltungen</li>"
        "<li>Kondensatoren &amp; Spulen</li>"
        "<li>Frequenz- &amp; Signalberechnungen</li>"
        "<li>BJT Transistor</li>"
        "<li>OperationsverstÃ¤rker</li>"
        "<li>Spezifischer Widerstand &amp; Temperatur</li>"
        "<li>SI-PrÃ¤fix-Eingabe: p n Âµ m k M G T</li>"
        "<li>Inline-Formel-Eingabefelder</li>"
        "<li>Eigene Formeln hinzufÃ¼gen</li>"
        "</ul>"
        "<p><i>Formeln sind in JSON gespeichert und erweiterbar.</i></p>");
}

void MainWindow::onLicenseInfo()
{
    auto& lic = LicenseManager::instance();
    const int days  = lic.daysRemaining();
    const QDate iss = lic.issueDate();

    QString msg;
    if (days >= 0) {
        msg = QString(
            "<h3>Lizenzinformationen</h3>"
            "<p><b>Status:</b> <span style='color:green;'>Aktiv</span></p>"
            "<p><b>Ausstellungsdatum:</b> %1</p>"
            "<p><b>Verbleibende Tage:</b> %2</p>"
            "<hr><p style='color:gray; font-size:10px;'>Testversion â€” nach Ablauf neuen Key anfordern.</p>")
            .arg(iss.toString("dd.MM.yyyy"))
            .arg(days);
    } else {
        msg = "<h3>Lizenzinformationen</h3>"
              "<p><b>Status:</b> <span style='color:red;'>Abgelaufen</span></p>"
              "<p>Bitte einen neuen Product Key beim Entwickler anfordern.</p>";
    }
    QMessageBox::information(this, "Lizenz", msg);
}


