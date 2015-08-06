#ifndef FLOWWIDGET_H
#define FLOWWIDGET_H

#include <QWidget>
#include <QColor>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPointF>
#include <iostream>
#include "flownode.h"
#include "sketchxml.h"

class flowwidget : public QWidget
{
    Q_OBJECT

public:
    explicit flowwidget(QWidget *parent = 0);
    ~flowwidget();

    void LoadXML(std::istream &data);
    void SaveXML(std::ostream &data);
    void SaveSVG(std::ostream &data);

    SketchXMLHandler* XMLHandlr;
    double ScaleFactor;
    double PermaScale;
    double PanX;
    double PanY;
    std::uint16_t BaseFontSize;

    QColor LineColor;
    QColor TextColor;

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

    QPointF previousMouseMove;
};

#endif // FLOWWIDGET_H
