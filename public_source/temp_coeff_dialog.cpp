#include "temp_coeff_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QLabel>
#include <QFont>

struct AlphaEntry {
    const char* material;
    const char* alpha;
    const char* application;
    const char* type;       // "PTC" / "NTC" / "Linear"
};

static const AlphaEntry ENTRIES[] = {
    { "Kupfer (Cu)",        "+0,00393",       "Leiter, Spulen, Wicklungen",       "PTC"    },
    { "Aluminium (Al)",     "+0,00390",       "Leiter, Kühlkörper",               "PTC"    },
    { "Gold (Au)",          "+0,00340",       "Kontakte, Mikroelektronik",         "PTC"    },
    { "Silber (Ag)",        "+0,00380",       "Kontakte, Präzisionsleiter",        "PTC"    },
    { "Wolfram (W)",        "+0,00450",       "Glühfaden, Temperatursensoren",     "PTC"    },
    { "Eisen (Fe)",         "+0,00650",       "Motoren, Transformatoren",          "PTC"    },
    { "Platin (Pt)",        "+0,00385",       "PT100 / PT1000 Sensoren",           "PTC"    },
    { "Nickel (Ni)",        "+0,00600",       "Temperatursensoren, Heizelemente",  "PTC"    },
    { "Chrom (Cr)",         "+0,00300",       "Legierungen, Heizelemente",         "PTC"    },
    { "Zinn (Sn)",          "+0,00440",       "Lötverbindungen",                   "PTC"    },
    { "Zink (Zn)",          "+0,00370",       "Legierungen, Beschichtungen",       "PTC"    },
    { "Nickelin (NiCr)",    "+0,00023",       "Widerstandsmaterial",               "Linear" },
    { "Manganin",           "±1 × 10⁻⁵",     "Präzisionswiderstände",             "Linear" },
    { "Konstantan",         "−3 × 10⁻⁵",     "Thermoelemente, Shunts",            "NTC"    },
    { "Kanthal (FeCrAl)",   "+0,00004",       "Heizelemente, Öfen, Laboröfen",     "Linear" },
    { "Kohle / Graphit (C)","-0,00045",       "Motorbürsten, Elektroden",          "NTC"    },
    { "Germanium (Ge)",     "ca. −0,048",     "Halbleiter (diskret)",              "NTC"    },
    { "Silizium (Si)",      "ca. −0,075",     "Halbleiter, ICs, MOSFETs",          "NTC"    },
    { "NTC-Thermistor",     "−0,02 … −0,06",  "Temperatursensoren, Kompensation",  "NTC"    },
    { "PTC-Thermistor",     "+0,1 … +0,3",    "Selbstregelnde Heizung, Schutz",   "PTC"    },
};
static const int N_ENTRIES = static_cast<int>(sizeof(ENTRIES) / sizeof(ENTRIES[0]));

TempCoeffDialog::TempCoeffDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Temperaturkoeffizient α — Referenztabelle");
    setMinimumSize(820, 520);
    resize(920, 580);

    setStyleSheet(R"(
        QDialog      { background:#1a1a1a; color:#e0e0e0; }
        QLabel       { color:#c0c0c0; font-size:12px; }
        QLineEdit    { background:#242424; border:1px solid #3a3a3a; border-radius:4px;
                       color:#fff; padding:5px 10px; font-size:12px; }
        QLineEdit:focus { border-color:#00aaff; }
        QTableWidget { background:#141414; color:#e0e0e0; gridline-color:#2a2a2a;
                       font-size:12px; border:1px solid #2a2a2a; }
        QTableWidget::item { padding:4px 8px; }
        QTableWidget::item:selected { background:#1a4a7a; color:#00aaff; }
        QHeaderView::section {
            background:#242424; color:#00aaff; font-weight:bold;
            padding:6px 8px; border:none; border-bottom:1px solid #3a3a3a;
        }
        QPushButton {
            background:#2e2e2e; border:1px solid #3a3a3a; border-radius:4px;
            color:#d0d0d0; padding:6px 16px; font-size:12px;
        }
        QPushButton:hover { background:#3a3a3a; color:#00aaff; border-color:#00aaff; }
    )");

    auto* mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(12, 10, 12, 10);
    mainLay->setSpacing(8);

    // Header
    auto* headerLbl = new QLabel(
        "<b>Temperaturkoeffizient α</b> der elektrischen Leitfähigkeit — Top 20 Materialien");
    headerLbl->setStyleSheet("color:#00aaff; font-size:14px;");
    mainLay->addWidget(headerLbl);

    // PTC/NTC explanation
    auto* noteLbl = new QLabel(
        "<span style='color:#ff8040;'>PTC</span> (Positive Temp. Coefficient) = Widerstand steigt mit Temperatur  |  "
        "<span style='color:#40aaff;'>NTC</span> (Negative Temp. Coefficient) = Widerstand sinkt mit Temperatur  |  "
        "<span style='color:#aaaaaa;'>Linear</span> = nahezu temperaturunabhängig");
    noteLbl->setWordWrap(true);
    noteLbl->setStyleSheet("font-size:11px; padding:2px 0;");
    mainLay->addWidget(noteLbl);

    // Filter row
    auto* filterRow = new QHBoxLayout;
    filterRow->addWidget(new QLabel("🔍 Filter:"));
    m_filterEdit = new QLineEdit;
    m_filterEdit->setPlaceholderText("Material, Typ (PTC / NTC / Linear) oder Anwendung suchen …");
    m_filterEdit->setClearButtonEnabled(true);
    filterRow->addWidget(m_filterEdit, 1);
    mainLay->addLayout(filterRow);

    // Table
    m_table = new QTableWidget;
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"Material", "α  (K⁻¹)", "Anwendung", "Typ"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->setVisible(false);
    m_table->setColumnWidth(0, 200);
    m_table->setStyleSheet(
        "QTableWidget { alternate-background-color:#1e1e1e; }"
        "QTableWidget::item[type=PTC]  { color:#ff8040; }"
        "QTableWidget::item[type=NTC]  { color:#40aaff; }"
    );
    mainLay->addWidget(m_table, 1);

    populateTable();

    // Close button
    auto* closeBtn = new QPushButton("Schließen");
    closeBtn->setFixedHeight(34);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch(1);
    btnRow->addWidget(closeBtn);
    mainLay->addLayout(btnRow);

    connect(m_filterEdit, &QLineEdit::textChanged,
            this, &TempCoeffDialog::onFilterChanged);
}

void TempCoeffDialog::populateTable()
{
    m_table->setRowCount(N_ENTRIES);
    for (int r = 0; r < N_ENTRIES; ++r) {
        const AlphaEntry& e = ENTRIES[r];

        auto makeItem = [](const char* text) {
            auto* it = new QTableWidgetItem(QString::fromUtf8(text));
            it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            return it;
        };

        m_table->setItem(r, 0, makeItem(e.material));
        m_table->setItem(r, 1, makeItem(e.alpha));

        auto* appItem = makeItem(e.application);
        appItem->setForeground(QColor("#b0b0b0"));
        m_table->setItem(r, 2, appItem);

        auto* typItem = makeItem(e.type);
        QString typ = QString::fromUtf8(e.type);
        if      (typ == "PTC")    typItem->setForeground(QColor("#ff8040"));
        else if (typ == "NTC")    typItem->setForeground(QColor("#40aaff"));
        else                      typItem->setForeground(QColor("#aaaaaa"));
        typItem->setFont(QFont("", -1, QFont::Bold));
        m_table->setItem(r, 3, typItem);
    }
}

void TempCoeffDialog::onFilterChanged(const QString& text)
{
    applyFilter(text.trimmed().toLower());
}

void TempCoeffDialog::applyFilter(const QString& filter)
{
    for (int r = 0; r < m_table->rowCount(); ++r) {
        bool show = filter.isEmpty();
        if (!show) {
            for (int c = 0; c < m_table->columnCount(); ++c) {
                auto* it = m_table->item(r, c);
                if (it && it->text().toLower().contains(filter)) { show = true; break; }
            }
        }
        m_table->setRowHidden(r, !show);
    }
}
