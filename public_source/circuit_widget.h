#pragma once
#include <QWidget>
#include <QString>
#include <QMap>

class CircuitWidget : public QWidget {
    Q_OBJECT
public:
    explicit CircuitWidget(QWidget* parent = nullptr);

    void setCircuitType(const QString& type);
    void setValues(const QMap<QString, double>& values, const QString& resultVar,
                   double resultValue, const QString& resultUnit);
    void clear();
    void setLightMode(bool light);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString             m_circuitType;
    QMap<QString,double>m_values;
    QString             m_resultVar;
    double              m_resultValue = 0.0;
    QString             m_resultUnit;
    bool                m_lightMode   = false;

    // Colors derived from m_lightMode
    QColor wireColor()  const;
    QColor bgColor()    const;
    QColor labelColor() const;
    QColor dimColor()   const;

    void drawOhmCircuit      (QPainter& p, const QRectF& rect);
    void drawSeriesCircuit   (QPainter& p, const QRectF& rect);
    void drawParallelCircuit (QPainter& p, const QRectF& rect);
    void drawPowerCircuit    (QPainter& p, const QRectF& rect);
    void drawCapacitorCircuit(QPainter& p, const QRectF& rect);
    void drawInductorCircuit (QPainter& p, const QRectF& rect);
    void drawFrequencyDiagram(QPainter& p, const QRectF& rect);
    void drawBJTCircuit      (QPainter& p, const QRectF& rect);
    void drawOpAmpCircuit    (QPainter& p, const QRectF& rect);
    void drawGenericFormula  (QPainter& p, const QRectF& rect);

    // Drawing helpers
    void drawWire         (QPainter& p, qreal x1, qreal y1, qreal x2, qreal y2);
    void drawResistor     (QPainter& p, qreal cx, qreal cy, qreal w, qreal h,
                           const QString& label = "", bool horizontal = true);
    void drawCapacitor    (QPainter& p, qreal cx, qreal cy, qreal size,
                           const QString& label = "");
    void drawInductor     (QPainter& p, qreal cx, qreal cy, qreal w,
                           const QString& label = "");
    void drawVoltageSource(QPainter& p, qreal cx, qreal cy, qreal r,
                           const QString& label = "");
    void drawGround       (QPainter& p, qreal cx, qreal cy);
    void drawArrow        (QPainter& p, qreal x1, qreal y1, qreal x2, qreal y2,
                           const QString& label = "");
    void drawLabel        (QPainter& p, qreal cx, qreal cy, const QString& text,
                           bool bold = false);
    QString fmtVal        (double v, const QString& unit) const;
};
