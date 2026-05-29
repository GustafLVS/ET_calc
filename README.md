# ⚡ ET-Rechner — Elektrotechnik Formelrechner

Ein moderner Formelrechner für Elektrotechnik-Studenten und Ingenieure, gebaut mit Qt6 und C++17.

![Screenshot](screenshot.png)

---

## Funktionen

- **Ohmsches Gesetz** — Spannung, Strom, Widerstand
- **Elektrische Leistung** — P, U, I, R Varianten
- **Reihen- & Parallelschaltungen** — Widerstände und Kondensatoren
- **Kondensatoren & Spulen** — Ladung, Energie, Zeitkonstante
- **Frequenz & Signale** — Periode, Kreisfrequenz, Wellenlänge
- **BJT Transistor** — Verstärkung, Arbeitspunkt
- **Operationsverstärker** — Invertierend, Nicht-invertierend, Differenzverstärker
- **Spezifischer Widerstand & Temperaturkoeffizient** — inkl. Referenztabelle für 20 Materialien
- **SI-Präfix Eingabe** — p n µ m k M G T direkt in Eingabefeldern
- **Schaltungsdiagramme** — visuelle Darstellung zur aktiven Formel
- **Eigene Formeln** — benutzerdefinierte Formeln hinzufügen und speichern
- **Dunkles Design** — augenschonendes Dark-Theme

---

## Voraussetzungen

- **Qt 6.x** (empfohlen: 6.5 oder neuer)
- **C++17** Compiler (MSVC 2019/2022 oder MinGW 64-bit)
- Windows 10 / 11

---

## Bauen

### Option A — Qt Creator (einfachste Methode)

1. Qt Creator öffnen
2. `ET_calc.pro` öffnen
3. Kit auswählen (Qt 6.x + MSVC oder MinGW)
4. Build-Modus auf **Release** stellen
5. ▶ Play-Button drücken

### Option B — Kommandozeile (MSYS2/MinGW)

```bash
qmake ET_calc.pro CONFIG+=release
mingw32-make -j4 release
```

### Option C — Batch-Skript

```
build.bat
```

---

## Lizenz / Testversion

Dieses Programm verwendet ein **5-Tage-Testlizenzsystem**.  
Für einen Product Key bitte den Entwickler kontaktieren.

---

## Technologien

| Komponente | Version |
|------------|---------|
| Qt         | 6.x     |
| C++        | 17      |
| Build      | qmake   |
| UI         | Qt Widgets |

---

## Projektstruktur

```
ET_calc/
├── main.cpp                  # Einstiegspunkt, Lizenzprüfung
├── mainwindow.cpp/h          # Hauptfenster
├── formula_engine.cpp/h      # Berechnungslogik
├── formula_library.cpp/h     # Formeln laden & verwalten
├── formula_input_widget.cpp/h# Dynamische Eingabefelder
├── circuit_widget.cpp/h      # Schaltungsdiagramm-Zeichnung
├── custom_formula_dialog.cpp/h # Eigene Formeln
├── temp_coeff_dialog.cpp/h   # Temperaturkoeffizient-Tabelle
├── license_manager.cpp/h     # Lizenzverwaltung
├── license_dialog.cpp/h      # Aktivierungsdialog
├── formulas.json             # Alle eingebauten Formeln
└── resources.qrc             # Qt Ressourcen
```

---

## Formeln hinzufügen

Eigene Formeln können über **Formeln → Neue Formel hinzufügen** im Programm erstellt werden. Sie werden in `%AppData%\ET-Calc\custom_formulas.json` gespeichert.

---

## Lizenz

Dieses Projekt ist für nicht-kommerzielle und Bildungszwecke gedacht.
