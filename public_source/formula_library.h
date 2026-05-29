#pragma once
#include "formula_engine.h"
#include <QVector>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>

class FormulaLibrary {
public:
    static FormulaLibrary& instance();

    bool loadFromJson(const QString& jsonPath);
    bool loadFromResource();
    bool saveToJson(const QString& jsonPath);

    bool addCustomFormula(const CategoryDefinition& category,
                          const FormulaDefinition& formula);
    bool addCustomCategory(const CategoryDefinition& category);

    QVector<CategoryDefinition>& categories();
    const QVector<CategoryDefinition>& categories() const;

    const FormulaDefinition* findFormula(const QString& id) const;
    const CategoryDefinition* findCategory(const QString& id) const;

    QString customFormulasPath() const;

private:
    FormulaLibrary() = default;
    QVector<CategoryDefinition> m_categories;
    QString m_customPath;

    CategoryDefinition parseCategory(const QJsonObject& obj);
    FormulaDefinition  parseFormula(const QJsonObject& obj);
    QJsonObject categoryToJson(const CategoryDefinition& cat) const;
    QJsonObject formulaToJson(const FormulaDefinition& f) const;
};
