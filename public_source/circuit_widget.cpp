#include "circuit_widget.h"
#include "formula_engine.h"
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QFont>
#include <cmath>

CircuitWidget::CircuitWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(300, 220);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#141414"));
    setPalette(pal);
}

void CircuitWidget::setLightMode(bool light)
{
    m_lightMode = light;
    QPalette pal = palette();
    pal.setColor(QPalette::Window, light ? QColor("#f0f0f0") : QColor("#141414"));
    setPalette(pal);
    update();
}

QColor CircuitWidget::wireColor()  const { return m_lightMode ? QColor("#005599") : QColor("#00aaff"); }
QColor CircuitWidget::bgColor()    const { return m_lightMode ? QColor("#f0f0f0") : QColor("#141414"); }
QColor CircuitWidget::labelColor() const { return m_lightMode ? QColor("#111111") : QColor("#e0e0e0"); }
QColor CircuitWidget::dimColor()   const { return m_lightMode ? QColor("#555555") : QColor("#888888"); }

QSize CircuitWidget::sizeHint() const { return QSize(480, 280); }
QSize CircuitWidget::minimumSizeHint() const { return QSize(300, 200); }

void CircuitWidget::setCircuitType(const QString& type) {
    m_circuitType = type;
    update();
}

void CircuitWidget::setValues(const QMap<QString, double>& values,
                               const QString& resultVar, double resultValue,
                               const QString& resultUnit)
{
    m_values = values;
    m_resultVar = resultVar;
    m_resultValue = resultValue;
    m_resultUnit = resultUnit;
    update();
}

void CircuitWidget::clear() {
    m_values.clear();
    m_resultVar.clear();
    m_resultValue = 0.0;
    m_resultUnit.clear();
    update();
}

QString CircuitWidget::fmtVal(double v, const QString& unit) const {
    return FormulaEngine::formatSICompact(v) + unit;
}

// ─── Drawing helpers ──────────────────────────────────────────────────────────
void CircuitWidget::drawWire(QPainter& p, qreal x1, qreal y1, qreal x2, qreal y2) {
    p.setPen(QPen(wireColor(), 2.0));
    p.drawLine(QPointF(x1, y1), QPointF(x2, y2));
}

void CircuitWidget::drawResistor(QPainter& p, qreal cx, qreal cy,
                                  qreal w, qreal h,
                                  const QString& label, bool horizontal)
{
    p.save();
    p.translate(cx, cy);
    if (!horizontal) p.rotate(90);

    QPen pen(QColor("#ffd700"), 2.0);
    p.setPen(pen);
    p.setBrush(QColor(40, 40, 70));

    qreal hw = w / 2, hh = h / 2;
    // Zig-zag resistor (IEC style rectangle)
    p.drawRect(QRectF(-hw, -hh, w, h));

    // Leads
    p.setPen(QPen(QColor("#00d4ff"), 2.0));
    p.drawLine(QPointF(-hw - 15, 0), QPointF(-hw, 0));
    p.drawLine(QPointF(hw, 0), QPointF(hw + 15, 0));

    // Label
    if (!label.isEmpty()) {
        p.setPen(QPen(QColor("#ffffff"), 1));
        QFont f = p.font();
        f.setPointSize(8);
        f.setBold(true);
        p.setFont(f);
        p.drawText(QRectF(-hw, -hh - 18, w, 16), Qt::AlignCenter, label);
    }
    p.restore();
}

void CircuitWidget::drawCapacitor(QPainter& p, qreal cx, qreal cy,
                                   qreal size, const QString& label)
{
    p.save();
    p.translate(cx, cy);
    qreal gap = 5;
    qreal plateH = size;
    QPen pen(QColor("#4fc3f7"), 2.5);
    p.setPen(pen);

    // Two plates
    p.drawLine(QPointF(-gap, -plateH/2), QPointF(-gap, plateH/2));
    p.drawLine(QPointF(gap, -plateH/2), QPointF(gap, plateH/2));

    // Leads
    p.setPen(QPen(QColor("#00d4ff"), 2.0));
    p.drawLine(QPointF(-gap - 20, 0), QPointF(-gap, 0));
    p.drawLine(QPointF(gap, 0), QPointF(gap + 20, 0));

    if (!label.isEmpty()) {
        p.setPen(Qt::white);
        QFont f = p.font(); f.setPointSize(8); p.setFont(f);
        p.drawText(QRectF(-30, -plateH/2 - 18, 60, 16), Qt::AlignCenter, label);
    }
    p.restore();
}

void CircuitWidget::drawInductor(QPainter& p, qreal cx, qreal cy,
                                  qreal w, const QString& label)
{
    p.save();
    p.translate(cx, cy);
    QPen pen(QColor("#80cbc4"), 2.5);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    int bumps = 4;
    qreal bumpW = w / bumps;
    qreal r = bumpW / 2;
    qreal startX = -w / 2;

    for (int i = 0; i < bumps; ++i) {
        qreal bx = startX + i * bumpW + r;
        p.drawArc(QRectF(bx - r, -r, bumpW, bumpW), 0, 180 * 16);
    }

    // Leads
    p.setPen(QPen(QColor("#00d4ff"), 2.0));
    p.drawLine(QPointF(-w/2 - 15, 0), QPointF(-w/2, 0));
    p.drawLine(QPointF(w/2, 0), QPointF(w/2 + 15, 0));

    if (!label.isEmpty()) {
        p.setPen(Qt::white);
        QFont f = p.font(); f.setPointSize(8); p.setFont(f);
        p.drawText(QRectF(-w/2, -r - 18, w, 16), Qt::AlignCenter, label);
    }
    p.restore();
}

void CircuitWidget::drawVoltageSource(QPainter& p, qreal cx, qreal cy,
                                       qreal r, const QString& label)
{
    p.save();
    p.translate(cx, cy);
    p.setPen(QPen(QColor("#ff7043"), 2.5));
    p.setBrush(QColor(50, 20, 20));
    p.drawEllipse(QPointF(0, 0), r, r);

    // + and - signs
    p.setPen(QPen(Qt::white, 1.5));
    p.drawLine(QPointF(-r*0.3f, -r*0.5f), QPointF(r*0.3f, -r*0.5f));
    p.drawLine(QPointF(0, -r*0.7f), QPointF(0, -r*0.3f));
    p.drawLine(QPointF(-r*0.3f, r*0.5f), QPointF(r*0.3f, r*0.5f));

    // Leads
    p.setPen(QPen(QColor("#00d4ff"), 2.0));
    p.drawLine(QPointF(0, -r), QPointF(0, -r - 20));
    p.drawLine(QPointF(0, r), QPointF(0, r + 20));

    if (!label.isEmpty()) {
        p.setPen(Qt::white);
        QFont f = p.font(); f.setPointSize(8); p.setFont(f);
        p.drawText(QRectF(-30, r + 22, 60, 16), Qt::AlignCenter, label);
    }
    p.restore();
}

void CircuitWidget::drawGround(QPainter& p, qreal cx, qreal cy) {
    p.save();
    p.translate(cx, cy);
    p.setPen(QPen(QColor("#aaaaaa"), 2.0));
    p.drawLine(QPointF(0, 0), QPointF(0, 8));
    p.drawLine(QPointF(-10, 8), QPointF(10, 8));
    p.drawLine(QPointF(-6, 12), QPointF(6, 12));
    p.drawLine(QPointF(-2, 16), QPointF(2, 16));
    p.restore();
}

void CircuitWidget::drawArrow(QPainter& p, qreal x1, qreal y1, qreal x2, qreal y2,
                               const QString& label)
{
    p.setPen(QPen(QColor("#76ff03"), 2.0));
    p.drawLine(QPointF(x1, y1), QPointF(x2, y2));

    // Arrowhead
    double angle = std::atan2(y2 - y1, x2 - x1);
    double arrowLen = 10;
    double arrowAngle = 0.4;
    QPointF p1(x2 - arrowLen * std::cos(angle - arrowAngle),
               y2 - arrowLen * std::sin(angle - arrowAngle));
    QPointF p2(x2 - arrowLen * std::cos(angle + arrowAngle),
               y2 - arrowLen * std::sin(angle + arrowAngle));
    p.drawLine(QPointF(x2, y2), p1);
    p.drawLine(QPointF(x2, y2), p2);

    if (!label.isEmpty()) {
        qreal mx = (x1 + x2) / 2;
        qreal my = (y1 + y2) / 2;
        p.setPen(Qt::white);
        QFont f = p.font(); f.setPointSize(8); p.setFont(f);
        p.drawText(QRectF(mx + 4, my - 10, 80, 20), label);
    }
}

void CircuitWidget::drawLabel(QPainter& p, qreal cx, qreal cy,
                               const QString& text, bool bold)
{
    p.setPen(Qt::white);
    QFont f = p.font();
    f.setPointSize(bold ? 10 : 8);
    f.setBold(bold);
    p.setFont(f);
    QFontMetrics fm(f);
    QRectF r(cx - fm.horizontalAdvance(text)/2.0,
             cy - fm.height()/2.0,
             fm.horizontalAdvance(text),
             fm.height());
    p.drawText(r, Qt::AlignCenter, text);
}

// ─── Circuit drawings ─────────────────────────────────────────────────────────
void CircuitWidget::drawOhmCircuit(QPainter& p, const QRectF& rect) {
    qreal cx = rect.center().x();
    qreal cy = rect.center().y();
    qreal w = rect.width();
    qreal h = rect.height();

    QString uLabel = m_values.contains("U") ? fmtVal(m_values["U"], "V") :
                     (m_resultVar == "U" ? fmtVal(m_resultValue, "V") : "U");
    QString iLabel = m_values.contains("I") ? fmtVal(m_values["I"], "A") :
                     (m_resultVar == "I" ? fmtVal(m_resultValue, "A") : "I");
    QString rLabel = m_values.contains("R") ? fmtVal(m_values["R"], "Ω") :
                     (m_resultVar == "R" ? fmtVal(m_resultValue, "Ω") : "R");

    qreal left = cx - w * 0.35;
    qreal right = cx + w * 0.35;
    qreal top = cy - h * 0.28;
    qreal bot = cy + h * 0.28;

    // Outer rectangle
    drawWire(p, left, top, right, top);
    drawWire(p, right, top, right, bot);
    drawWire(p, right, bot, left, bot);
    drawWire(p, left, bot, left, top);

    // Voltage source on left
    drawVoltageSource(p, left, cy, 22, uLabel);

    // Resistor on top
    drawResistor(p, cx, top, 70, 20, rLabel, true);

    // Current arrow
    drawArrow(p, cx + 30, top - 22, cx + 70, top - 22, iLabel);

    // Dot nodes
    p.setPen(QPen(QColor("#ffcc02"), 1));
    p.setBrush(QColor("#ffcc02"));
    p.drawEllipse(QPointF(left, top), 4, 4);
    p.drawEllipse(QPointF(right, top), 4, 4);
    p.drawEllipse(QPointF(left, bot), 4, 4);
    p.drawEllipse(QPointF(right, bot), 4, 4);

    // Title
    drawLabel(p, cx, rect.top() + 14, "Ohmsches Gesetz", true);
}

void CircuitWidget::drawSeriesCircuit(QPainter& p, const QRectF& rect) {
    qreal cx = rect.center().x();
    qreal cy = rect.center().y();
    qreal w = rect.width();
    qreal h = rect.height();

    qreal left = cx - w * 0.38;
    qreal right = cx + w * 0.38;
    qreal top = cy - h * 0.25;
    qreal bot = cy + h * 0.25;

    QString r1 = m_values.contains("R1") ? fmtVal(m_values["R1"], "Ω") : "R1";
    QString r2 = m_values.contains("R2") ? fmtVal(m_values["R2"], "Ω") : "R2";
    QString rg = m_resultVar == "R_ges" ? fmtVal(m_resultValue, "Ω") : "R_ges";

    drawWire(p, left, top, right, top);
    drawWire(p, right, top, right, bot);
    drawWire(p, right, bot, left, bot);
    drawWire(p, left, bot, left, top);

    drawVoltageSource(p, left, cy, 22);
    drawResistor(p, cx - 40, top, 55, 20, r1, true);
    drawResistor(p, cx + 40, top, 55, 20, r2, true);

    // Result label
    p.setPen(QPen(QColor("#76ff03"), 1.5));
    QFont f = p.font(); f.setPointSize(9); f.setBold(true); p.setFont(f);
    p.drawText(QRectF(cx - 60, bot + 8, 120, 20), Qt::AlignCenter,
               QString("R_ges = %1").arg(rg));

    drawLabel(p, cx, rect.top() + 14, "Reihenschaltung", true);
}

void CircuitWidget::drawParallelCircuit(QPainter& p, const QRectF& rect) {
    qreal cx = rect.center().x();
    qreal cy = rect.center().y();
    qreal w = rect.width();
    qreal h = rect.height();

    qreal left = cx - w * 0.36;
    qreal right = cx + w * 0.36;
    qreal mid1 = cx - 15;
    qreal mid2 = cx + 15;
    qreal top1 = cy - h * 0.20;
    qreal top2 = cy + h * 0.20;

    QString r1 = m_values.contains("R1") ? fmtVal(m_values["R1"], "Ω") : "R1";
    QString r2 = m_values.contains("R2") ? fmtVal(m_values["R2"], "Ω") : "R2";
    QString rg = m_resultVar == "R_ges" ? fmtVal(m_resultValue, "Ω") : "R_ges";

    // Left rail
    drawWire(p, left, top1, left, top2);
    // Right rail
    drawWire(p, right, top1, right, top2);

    // Top and bottom horizontal
    drawWire(p, left, top1, right, top1);
    drawWire(p, left, top2, right, top2);

    // R1 branch (upper)
    drawResistor(p, cx, top1, 70, 22, r1, true);

    // R2 branch (lower)
    drawResistor(p, cx, top2, 70, 22, r2, true);

    // Voltage source
    drawVoltageSource(p, left - 30, cy, 22);
    drawWire(p, left - 30, cy - 22, left, top1);
    drawWire(p, left - 30, cy + 22, left, top2);

    // Result
    p.setPen(QPen(QColor("#76ff03"), 1.5));
    QFont f = p.font(); f.setPointSize(9); f.setBold(true); p.setFont(f);
    p.drawText(QRectF(cx - 60, top2 + 30, 120, 20), Qt::AlignCenter,
               QString("R_ges = %1").arg(rg));

    drawLabel(p, cx, rect.top() + 14, "Parallelschaltung", true);
}

void CircuitWidget::drawPowerCircuit(QPainter& p, const QRectF& rect) {
    qreal cx = rect.center().x();
    qreal cy = rect.center().y();
    qreal w = rect.width();
    qreal h = rect.height();

    qreal left = cx - w * 0.35;
    qreal right = cx + w * 0.35;
    qreal top = cy - h * 0.28;
    qreal bot = cy + h * 0.28;

    drawWire(p, left, top, right, top);
    drawWire(p, right, top, right, bot);
    drawWire(p, right, bot, left, bot);
    drawWire(p, left, bot, left, top);

    QString uLabel = m_values.contains("U") ? fmtVal(m_values["U"], "V") : "U";
    QString rLabel = m_values.contains("R") ? fmtVal(m_values["R"], "Ω") : "R";
    QString pLabel = m_resultVar == "P" ? fmtVal(m_resultValue, "W") :
                     m_values.contains("P") ? fmtVal(m_values["P"], "W") : "P";

    drawVoltageSource(p, left, cy, 22, uLabel);
    drawResistor(p, cx, top, 70, 20, rLabel);

    // Power symbol
    p.setPen(QPen(QColor("#ff9800"), 2));
    p.setBrush(QColor(80, 40, 0));
    p.drawEllipse(QPointF(cx, cy + 5), 25, 25);
    p.setPen(QPen(Qt::white, 1));
    QFont f = p.font(); f.setPointSize(8); f.setBold(true); p.setFont(f);
    p.drawText(QRectF(cx - 25, cy - 8, 50, 26), Qt::AlignCenter, "P\n" + pLabel);

    drawLabel(p, cx, rect.top() + 14, "Elektrische Leistung", true);
}

void CircuitWidget::drawCapacitorCircuit(QPainter& p, const QRectF& rect) {
    qreal cx = rect.center().x();
    qreal cy = rect.center().y();
    qreal w = rect.width();
    qreal h = rect.height();

    qreal left = cx - w * 0.35;
    qreal right = cx + w * 0.35;
    qreal top = cy - h * 0.28;
    qreal bot = cy + h * 0.28;

    drawWire(p, left, top, cx - 30, top);
    drawWire(p, cx + 30, top, right, top);
    drawWire(p, right, top, right, bot);
    drawWire(p, right, bot, left, bot);
    drawWire(p, left, bot, left, top);

    QString cLabel = m_values.contains("C") ? fmtVal(m_values["C"], "F") : "C";
    QString uLabel = m_values.contains("U") ? fmtVal(m_values["U"], "V") : "U";

    drawCapacitor(p, cx, top, 30, cLabel);
    drawVoltageSource(p, left, cy, 22, uLabel);

    // Charge label
    if (m_resultVar == "Q" || m_resultVar == "W" || m_resultVar == "tau") {
        p.setPen(QPen(QColor("#76ff03"), 1.5));
        QFont f = p.font(); f.setPointSize(9); f.setBold(true); p.setFont(f);
        p.drawText(QRectF(cx - 60, bot + 8, 120, 20), Qt::AlignCenter,
                   QString("%1 = %2 %3").arg(m_resultVar).arg(
                       FormulaEngine::formatSICompact(m_resultValue)).arg(m_resultUnit));
    }

    drawLabel(p, cx, rect.top() + 14, "Kondensator-Schaltung", true);
}

void CircuitWidget::drawInductorCircuit(QPainter& p, const QRectF& rect) {
    qreal cx = rect.center().x();
    qreal cy = rect.center().y();
    qreal w = rect.width();
    qreal h = rect.height();

    qreal left = cx - w * 0.35;
    qreal right = cx + w * 0.35;
    qreal top = cy - h * 0.28;
    qreal bot = cy + h * 0.28;

    drawWire(p, left, top, cx - 45, top);
    drawWire(p, cx + 45, top, right, top);
    drawWire(p, right, top, right, bot);
    drawWire(p, right, bot, left, bot);
    drawWire(p, left, bot, left, top);

    QString lLabel = m_values.contains("L") ? fmtVal(m_values["L"], "H") : "L";
    drawInductor(p, cx, top, 70, lLabel);
    drawVoltageSource(p, left, cy, 22);

    if (!m_resultVar.isEmpty()) {
        p.setPen(QPen(QColor("#76ff03"), 1.5));
        QFont f = p.font(); f.setPointSize(9); f.setBold(true); p.setFont(f);
        p.drawText(QRectF(cx - 70, bot + 8, 140, 20), Qt::AlignCenter,
                   QString("%1 = %2 %3").arg(m_resultVar).arg(
                       FormulaEngine::formatSICompact(m_resultValue)).arg(m_resultUnit));
    }

    drawLabel(p, cx, rect.top() + 14, "Spulen-Schaltung", true);
}

void CircuitWidget::drawFrequencyDiagram(QPainter& p, const QRectF& rect) {
    qreal cx = rect.center().x();
    qreal ox = rect.left() + 40;
    qreal oy = rect.center().y();
    qreal aw = rect.width() - 60;
    qreal ah = rect.height() * 0.35;

    // Axes
    p.setPen(QPen(QColor("#aaaaaa"), 1.5));
    p.drawLine(QPointF(ox, oy - ah), QPointF(ox, oy + ah));
    p.drawLine(QPointF(ox, oy), QPointF(ox + aw, oy));

    // Axis labels
    QFont f = p.font(); f.setPointSize(8); p.setFont(f);
    p.setPen(QColor("#aaaaaa"));
    p.drawText(QRectF(ox + aw - 10, oy + 5, 20, 16), "t");
    p.drawText(QRectF(ox + 4, oy - ah - 16, 20, 16), "U");

    // Sine wave
    QPainterPath path;
    double freq = m_values.contains("f") ? m_values["f"] : 1.0;
    double T = 1.0 / freq;
    (void)T;
    bool first = true;
    int cycles = 3;
    for (int xi = 0; xi <= 200; ++xi) {
        double t = (double)xi / 200.0 * cycles;
        double y = std::sin(2 * M_PI * t);
        qreal px = ox + (xi / 200.0) * aw;
        qreal py = oy - y * ah * 0.8;
        if (first) { path.moveTo(px, py); first = false; }
        else path.lineTo(px, py);
    }
    p.setPen(QPen(QColor("#00d4ff"), 2.0));
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);

    // Period marker
    qreal periodPixels = aw / cycles;
    p.setPen(QPen(QColor("#ffd700"), 1.5, Qt::DashLine));
    p.drawLine(QPointF(ox, oy - ah * 0.85), QPointF(ox + periodPixels, oy - ah * 0.85));
    QFont fb = p.font(); fb.setPointSize(8); fb.setBold(true); p.setFont(fb);
    p.setPen(QColor("#ffd700"));
    p.drawText(QRectF(ox, oy - ah - 18, periodPixels, 16), Qt::AlignCenter,
               "T = " + fmtVal(1.0/freq, "s"));

    if (!m_resultVar.isEmpty()) {
        p.setPen(QPen(QColor("#76ff03"), 1.5));
        p.drawText(QRectF(cx - 80, rect.bottom() - 22, 160, 18), Qt::AlignCenter,
                   QString("%1 = %2 %3").arg(m_resultVar).arg(
                       FormulaEngine::formatSICompact(m_resultValue)).arg(m_resultUnit));
    }

    drawLabel(p, cx, rect.top() + 14, "Frequenz & Signal", true);
}

void CircuitWidget::drawBJTCircuit(QPainter& p, const QRectF& rect) {
    qreal cx = rect.center().x();
    qreal cy = rect.center().y();

    // Transistor body circle
    p.setPen(QPen(QColor("#ffd700"), 2.0));
    p.setBrush(QColor(40, 40, 10));
    p.drawEllipse(QPointF(cx, cy), 35, 35);

    // Base
    p.setPen(QPen(QColor("#00d4ff"), 2.5));
    p.drawLine(QPointF(cx - 70, cy), QPointF(cx - 35, cy));
    drawLabel(p, cx - 80, cy, "B");

    // Collector
    p.drawLine(QPointF(cx - 10, cy - 15), QPointF(cx - 10, cy - 60));
    drawLabel(p, cx - 10, cy - 72, "C");

    // Emitter with arrow
    p.drawLine(QPointF(cx - 10, cy + 15), QPointF(cx - 10, cy + 60));
    drawLabel(p, cx - 10, cy + 72, "E");

    // Base bar
    p.setPen(QPen(QColor("#ffd700"), 3.0));
    p.drawLine(QPointF(cx - 15, cy - 25), QPointF(cx - 15, cy + 25));

    // Internal lines
    p.setPen(QPen(QColor("#00d4ff"), 2.0));
    p.drawLine(QPointF(cx - 15, cy - 15), QPointF(cx - 10, cy - 20));
    p.drawLine(QPointF(cx - 15, cy + 15), QPointF(cx - 10, cy + 20));

    // NPN arrow on emitter
    QPainterPath arrow;
    arrow.moveTo(cx - 10, cy + 20);
    arrow.lineTo(cx - 4, cy + 26);
    arrow.lineTo(cx - 18, cy + 26);
    arrow.closeSubpath();
    p.fillPath(arrow, QColor("#00d4ff"));

    // Labels
    QString icLabel = m_values.contains("IC") ? fmtVal(m_values["IC"], "A") :
                      m_resultVar == "IC" ? fmtVal(m_resultValue, "A") : "IC";
    QString ibLabel = m_values.contains("IB") ? fmtVal(m_values["IB"], "A") :
                      m_resultVar == "IB" ? fmtVal(m_resultValue, "A") : "IB";
    QString betaLabel = m_values.contains("beta") ? QString("β=%1").arg(m_values["beta"]) :
                        m_resultVar == "beta" ? QString("β=%1").arg(m_resultValue, 0, 'g', 4) : "β";

    drawLabel(p, cx + 50, cy - 50, "IC=" + icLabel);
    drawLabel(p, cx + 50, cy, ibLabel);
    drawLabel(p, cx + 50, cy + 20, betaLabel);

    drawLabel(p, cx, rect.top() + 14, "BJT Transistor (NPN)", true);
}

void CircuitWidget::drawOpAmpCircuit(QPainter& p, const QRectF& rect) {
    qreal cx = rect.center().x();
    qreal cy = rect.center().y() + 10;

    // Triangle body
    QPainterPath tri;
    tri.moveTo(cx - 45, cy - 35);
    tri.lineTo(cx - 45, cy + 35);
    tri.lineTo(cx + 45, cy);
    tri.closeSubpath();

    p.setPen(QPen(QColor("#ffd700"), 2.0));
    p.setBrush(QColor(40, 40, 10));
    p.drawPath(tri);

    // + and - inputs
    p.setPen(QPen(Qt::white, 1.5));
    QFont f = p.font(); f.setPointSize(10); f.setBold(true); p.setFont(f);
    p.drawText(QRectF(cx - 42, cy - 30, 20, 20), Qt::AlignCenter, "+");
    p.drawText(QRectF(cx - 42, cy + 10, 20, 20), Qt::AlignCenter, "−");

    // Wires
    p.setPen(QPen(QColor("#00d4ff"), 2.0));
    // Inverting input
    p.drawLine(QPointF(cx - 80, cy + 20), QPointF(cx - 45, cy + 20));
    // Non-inverting input
    p.drawLine(QPointF(cx - 80, cy - 20), QPointF(cx - 45, cy - 20));
    // Output
    p.drawLine(QPointF(cx + 45, cy), QPointF(cx + 90, cy));

    // Feedback Rf
    p.drawLine(QPointF(cx + 70, cy), QPointF(cx + 70, cy - 55));
    p.drawLine(QPointF(cx + 70, cy - 55), QPointF(cx - 60, cy - 55));
    p.drawLine(QPointF(cx - 60, cy - 55), QPointF(cx - 60, cy + 20));
    p.drawLine(QPointF(cx - 60, cy + 20), QPointF(cx - 80, cy + 20));

    QString rfLabel = m_values.contains("Rf") ? "Rf=" + fmtVal(m_values["Rf"], "Ω") : "Rf";
    QString r1Label = m_values.contains("R1") ? "R1=" + fmtVal(m_values["R1"], "Ω") : "R1";
    drawResistor(p, cx + 5, cy - 55, 60, 16, rfLabel, true);

    // R1
    p.setPen(QPen(QColor("#00d4ff"), 2.0));
    p.drawLine(QPointF(cx - 80, cy + 20), QPointF(cx - 115, cy + 20));
    drawResistor(p, cx - 97, cy + 20, 44, 16, r1Label, true);

    // Uin label
    p.setPen(Qt::white);
    QFont fb = p.font(); fb.setPointSize(8); p.setFont(fb);
    p.drawText(QRectF(cx - 120, cy - 30, 30, 16), "Uin");

    // Vout label
    QString vLabel = m_resultVar == "V" ? QString("V=%1").arg(m_resultValue, 0, 'g', 4) :
                     m_resultVar == "Uout" ? "Uout=" + fmtVal(m_resultValue, "V") : "Uout";
    p.setPen(QPen(QColor("#76ff03"), 1.5));
    p.drawText(QRectF(cx + 60, cy - 14, 60, 16), vLabel);

    drawLabel(p, cx, rect.top() + 14, "Operationsverstärker", true);
}

void CircuitWidget::drawGenericFormula(QPainter& p, const QRectF& rect) {
    qreal cx = rect.center().x();
    qreal cy = rect.center().y();

    // Background grid
    p.setPen(QPen(QColor(255, 255, 255, 20), 1));
    for (int xi = (int)rect.left(); xi < rect.right(); xi += 20)
        p.drawLine(xi, (int)rect.top(), xi, (int)rect.bottom());
    for (int yi = (int)rect.top(); yi < rect.bottom(); yi += 20)
        p.drawLine((int)rect.left(), yi, (int)rect.right(), yi);

    // Show variables in nice layout
    QFont f = p.font();
    f.setPointSize(11);
    f.setBold(true);
    p.setFont(f);
    p.setPen(QColor("#ffd700"));

    int row = 0;
    for (auto it = m_values.constBegin(); it != m_values.constEnd(); ++it) {
        QString text = QString("%1 = %2").arg(it.key()).arg(fmtVal(it.value(), ""));
        p.drawText(QRectF(cx - 120, cy - 40 + row * 28, 240, 26), Qt::AlignCenter, text);
        ++row;
    }

    if (!m_resultVar.isEmpty()) {
        p.setPen(QPen(QColor("#76ff03"), 2));
        p.drawRect(QRectF(cx - 110, cy - 50 + row * 28, 220, 30));
        p.setPen(QColor("#76ff03"));
        p.drawText(QRectF(cx - 110, cy - 48 + row * 28, 220, 28), Qt::AlignCenter,
                   QString("%1 = %2 %3").arg(m_resultVar).arg(
                       FormulaEngine::formatSICompact(m_resultValue)).arg(m_resultUnit));
    }
}

void CircuitWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    // Background
    p.fillRect(rect(), QColor("#0f0f23"));

    // Subtle grid
    p.setPen(QPen(QColor(255, 255, 255, 8), 1));
    for (int x = 0; x < width(); x += 20)
        p.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += 20)
        p.drawLine(0, y, width(), y);

    QRectF drawArea = QRectF(10, 10, width() - 20, height() - 20);

    QFont base = p.font();
    base.setFamily("Segoe UI");
    p.setFont(base);

    if (m_circuitType == "ohm")         drawOhmCircuit(p, drawArea);
    else if (m_circuitType == "series") drawSeriesCircuit(p, drawArea);
    else if (m_circuitType == "parallel") drawParallelCircuit(p, drawArea);
    else if (m_circuitType == "power")  drawPowerCircuit(p, drawArea);
    else if (m_circuitType == "capacitor") drawCapacitorCircuit(p, drawArea);
    else if (m_circuitType == "inductor")  drawInductorCircuit(p, drawArea);
    else if (m_circuitType == "frequency") drawFrequencyDiagram(p, drawArea);
    else if (m_circuitType == "bjt")    drawBJTCircuit(p, drawArea);
    else if (m_circuitType == "opamp")  drawOpAmpCircuit(p, drawArea);
    else if (!m_circuitType.isEmpty())  drawGenericFormula(p, drawArea);
    else {
        // Placeholder
        p.setPen(QColor(80, 80, 120));
        QFont f = p.font(); f.setPointSize(11); p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "Wähle eine Formel\nund berechne,\num die Schaltung anzuzeigen");
    }
}
