#include "formula_library.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>

FormulaLibrary& FormulaLibrary::instance() {
    static FormulaLibrary inst;
    return inst;
}

QString FormulaLibrary::customFormulasPath() const {
    return m_customPath;
}

QVector<CategoryDefinition>& FormulaLibrary::categories() {
    return m_categories;
}
const QVector<CategoryDefinition>& FormulaLibrary::categories() const {
    return m_categories;
}

FormulaDefinition FormulaLibrary::parseFormula(const QJsonObject& obj) {
    FormulaDefinition f;
    f.id = obj["id"].toString();
    f.name = obj["name"].toString();
    f.solve_for = obj["solve_for"].toString();
    f.expression = obj["expression"].toString();
    f.result_unit = obj["result_unit"].toString();
    f.result_description = obj["result_description"].toString();

    for (const QJsonValue& v : obj["variables"].toArray()) {
        QJsonObject vo = v.toObject();
        FormulaVariable var;
        var.name = vo["name"].toString();
        var.unit = vo["unit"].toString();
        var.description = vo["description"].toString();
        f.variables.append(var);
    }

    for (const QJsonValue& s : obj["steps"].toArray())
        f.steps.append(s.toString());

    f.display_template = obj["display_template"].toString();
    f.chain_formula    = obj["chain_formula"].toString();

    return f;
}

CategoryDefinition FormulaLibrary::parseCategory(const QJsonObject& obj) {
    CategoryDefinition cat;
    cat.name = obj["name"].toString();
    cat.id = obj["id"].toString();
    cat.circuit_type = obj["circuit_type"].toString();

    for (const QJsonValue& fv : obj["formulas"].toArray())
        cat.formulas.append(parseFormula(fv.toObject()));

    return cat;
}

bool FormulaLibrary::loadFromJson(const QString& jsonPath) {
    QFile f(jsonPath);
    if (!f.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) return false;

    QJsonArray cats = doc.object()["categories"].toArray();
    for (const QJsonValue& cv : cats)
        m_categories.append(parseCategory(cv.toObject()));

    return true;
}

bool FormulaLibrary::loadFromResource() {
    QFile f(":/formulas.json");
    if (!f.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) return false;

    QJsonArray cats = doc.object()["categories"].toArray();
    for (const QJsonValue& cv : cats)
        m_categories.append(parseCategory(cv.toObject()));

    // Also load custom formulas
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appData);
    m_customPath = appData + "/custom_formulas.json";

    QFile cf(m_customPath);
    if (cf.open(QIODevice::ReadOnly)) {
        QJsonDocument cdoc = QJsonDocument::fromJson(cf.readAll());
        if (cdoc.isObject()) {
            for (const QJsonValue& cv : cdoc.object()["categories"].toArray()) {
                CategoryDefinition cat = parseCategory(cv.toObject());
                // Merge into existing category or add new
                bool found = false;
                for (auto& existing : m_categories) {
                    if (existing.id == cat.id) {
                        for (const auto& ff : cat.formulas)
                            existing.formulas.append(ff);
                        found = true;
                        break;
                    }
                }
                if (!found) m_categories.append(cat);
            }
        }
    }

    return true;
}

QJsonObject FormulaLibrary::formulaToJson(const FormulaDefinition& f) const {
    QJsonObject obj;
    obj["id"] = f.id;
    obj["name"] = f.name;
    obj["solve_for"] = f.solve_for;
    obj["expression"] = f.expression;
    obj["result_unit"] = f.result_unit;
    obj["result_description"] = f.result_description;

    QJsonArray vars;
    for (const auto& v : f.variables) {
        QJsonObject vo;
        vo["name"] = v.name;
        vo["unit"] = v.unit;
        vo["description"] = v.description;
        vars.append(vo);
    }
    obj["variables"] = vars;

    QJsonArray steps;
    for (const auto& s : f.steps) steps.append(s);
    obj["steps"] = steps;

    return obj;
}

QJsonObject FormulaLibrary::categoryToJson(const CategoryDefinition& cat) const {
    QJsonObject obj;
    obj["name"] = cat.name;
    obj["id"] = cat.id;
    obj["circuit_type"] = cat.circuit_type;

    QJsonArray formulas;
    for (const auto& f : cat.formulas) formulas.append(formulaToJson(f));
    obj["formulas"] = formulas;

    return obj;
}

bool FormulaLibrary::saveToJson(const QString& jsonPath) {
    QJsonArray cats;
    for (const auto& c : m_categories) cats.append(categoryToJson(c));

    QJsonObject root;
    root["categories"] = cats;

    QFile f(jsonPath);
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool FormulaLibrary::addCustomFormula(const CategoryDefinition& category,
                                       const FormulaDefinition& formula) {
    for (auto& cat : m_categories) {
        if (cat.id == category.id) {
            cat.formulas.append(formula);
            return true;
        }
    }
    // Category not found, add new one with just this formula
    CategoryDefinition newCat = category;
    newCat.formulas.clear();
    newCat.formulas.append(formula);
    m_categories.append(newCat);
    return true;
}

bool FormulaLibrary::addCustomCategory(const CategoryDefinition& category) {
    for (const auto& cat : m_categories)
        if (cat.id == category.id) return false;
    m_categories.append(category);
    return true;
}

const FormulaDefinition* FormulaLibrary::findFormula(const QString& id) const {
    for (const auto& cat : m_categories)
        for (const auto& f : cat.formulas)
            if (f.id == id) return &f;
    return nullptr;
}

const CategoryDefinition* FormulaLibrary::findCategory(const QString& id) const {
    for (const auto& cat : m_categories)
        if (cat.id == id) return &cat;
    return nullptr;
}
