#include "flowwidget.h"
#include "iostreambridge.h"
#include <QPainter>
#include <QXmlSimpleReader>
#include <QtSvg/QSvgGenerator>

// Static configuration //
const int MaxBlockWidth = 200;
const int BlockPadding = 15;

// Initializers //

flowwidget::flowwidget(QWidget *parent) : QWidget(parent)
{
    XMLHandlr = new SketchXMLHandler();
    ScaleFactor = 1.0;
    PanX = 0.0;
    PanY = 0.0;
    BaseFontSize = 8;

    LineColor = Qt::black;
    TextColor = Qt::white;
}

flowwidget::~flowwidget()
{
    delete XMLHandlr;
}

//  Internals //

struct NodeSize {
    QRect TitleBoundBox;
    QRect DescBoundBox;

    int Width;
    int Height;
};

NodeSize getNodeSize(QFont font, QFont titlefont, FlowNode& node) {
    NodeSize ns;
    QFontMetrics fmetrics(font);
    QFontMetrics ftmetrics(titlefont);

    QRect titlebounding = ftmetrics.boundingRect(QString(node.Title.c_str()));
    int MaxWidth;
    if (titlebounding.width() < MaxBlockWidth)
        MaxWidth = MaxBlockWidth;
    else
        MaxWidth = titlebounding.width();
    QRect descbounding = fmetrics.boundingRect(QRect(0, 0, MaxWidth, 0), Qt::AlignHCenter | Qt::TextWordWrap, QString(node.Desc.c_str()));

    int blockwidth = BlockPadding*2;
    if (titlebounding.width() < descbounding.width())
        blockwidth += descbounding.width();
    else
        blockwidth += titlebounding.width();
    int blockheight = BlockPadding + titlebounding.height() +
            BlockPadding + descbounding.height() + BlockPadding;

    ns.DescBoundBox = descbounding;
    ns.TitleBoundBox = titlebounding;
    ns.Width = blockwidth;
    ns.Height = blockheight;
    return ns;
}

void renderFullTree(QPaintDevice *paintdev, SketchXMLHandler* XMLHandlr,
                    QColor LineColor, std::uint16_t BaseFontSize,
                    double Width, double Height, double PermaScale, double ScaleFactor, double PanX, double PanY) {
    QPainter paint(paintdev);
    paint.resetTransform();
    paint.translate(double(Width/2) + (PanX * ScaleFactor), double(Height/2) + (PanY * ScaleFactor));

    paint.scale(PermaScale * Width * ScaleFactor, PermaScale * Width * ScaleFactor);
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.setRenderHint(QPainter::TextAntialiasing, true);
    paint.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPen linepen (LineColor);
    linepen.setWidth(5);
    paint.setPen(linepen);
    std::for_each(XMLHandlr->nodelinkmap.begin(), XMLHandlr->nodelinkmap.end(),
                  [&](std::pair<std::string, std::vector<std::string>> block) {
        FlowNode fnparent = XMLHandlr->OrphanNodes[block.first];
        std::for_each(block.second.begin(), block.second.end(), [&](std::string child) {
            FlowNode fnchild = XMLHandlr->OrphanNodes[child];
            if ((fnchild.Type == NodeType::NONE) || (fnparent.Type == NodeType::NONE))
                return;
            paint.drawLine(fnparent.CenterPosX, fnparent.CenterPosY,
                           fnchild.CenterPosX, fnchild.CenterPosY);
        });
    });

    std::for_each(XMLHandlr->OrphanNodes.begin(), XMLHandlr->OrphanNodes.end(),
                  [&](std::pair<std::string, FlowNode> nodepair) {
        if (nodepair.second.Type == NodeType::NONE)
            return;
        QFont basefont = QFont("sans-serif", BaseFontSize*nodepair.second.FontSizeMult);
        QFont titlefont = QFont("sans-serif", BaseFontSize*nodepair.second.FontSizeMult*1.5);

        NodeSize ns = getNodeSize(basefont, titlefont, nodepair.second);

        int rectoriginx = nodepair.second.CenterPosX - (ns.Width/2);
        int rectoriginy = nodepair.second.CenterPosY - (ns.Height/2);

        QColor PrimColor = QColor(nodepair.second.ColorRGBA[0],
                nodepair.second.ColorRGBA[1],
                nodepair.second.ColorRGBA[2],
                nodepair.second.ColorRGBA[3]);
        paint.fillRect(rectoriginx, rectoriginy, ns.Width, ns.Height, PrimColor);

        QColor SecColor;
        if (PrimColor.toHsl().lightness() > 256)
            SecColor = PrimColor.lighter(200);
        else
            SecColor = PrimColor.darker(300);
        QPen textpen (SecColor);
        textpen.setWidth(3);
        paint.setPen(textpen);
        paint.drawRect(rectoriginx, rectoriginy, ns.Width, ns.Height);

        paint.setFont(titlefont);
        paint.drawText(rectoriginx + BlockPadding, rectoriginy + BlockPadding,
                       ns.Width - (BlockPadding*2), ns.TitleBoundBox.height(),
                       Qt::AlignCenter, QString(nodepair.second.Title.c_str()));
        paint.setFont(basefont);
        paint.drawText(nodepair.second.CenterPosX - (ns.DescBoundBox.width()/2),
                       rectoriginy + BlockPadding + ns.TitleBoundBox.height() + BlockPadding,
                       ns.DescBoundBox.width(), ns.DescBoundBox.height(),
                       Qt::AlignCenter | Qt::TextWordWrap,
                       QString(nodepair.second.Desc.c_str()));
    });
    paint.end();
}

// Node XML loaders //

void flowwidget::LoadXML(std::istream &data) {
    QXmlSimpleReader qxsr;
    qxsr.setContentHandler(XMLHandlr);
    qxsr.setErrorHandler(XMLHandlr);

    QIODevice* iodev = new IOStreamBridge(data);
    if (qxsr.parse(QXmlInputSource(iodev))) {
        std::cerr << "[FlowWidget] Recentering nodes and adjusting scale." << std::endl;
        double GenMinimumX = std::numeric_limits<double>::max();
        double GenExtremeX = std::numeric_limits<double>::min();
        double GenMinimumY = std::numeric_limits<double>::max();
        double GenExtremeY = std::numeric_limits<double>::min();
        std::for_each(XMLHandlr->OrphanNodes.begin(), XMLHandlr->OrphanNodes.end(),
                      [&](std::pair<std::string, FlowNode> nodepair) {
            NodeSize ns = getNodeSize(QFont("sans-serif", BaseFontSize*nodepair.second.FontSizeMult), QFont("sans-serif", BaseFontSize*nodepair.second.FontSizeMult*1.5), nodepair.second);
            if (nodepair.second.CenterPosX - double(ns.Width/2) < GenMinimumX)
                GenMinimumX = nodepair.second.CenterPosX - double(ns.Width/2);
            if (nodepair.second.CenterPosX + double(ns.Width/2) > GenExtremeX)
                GenExtremeX = nodepair.second.CenterPosX + double(ns.Width/2);
            if (nodepair.second.CenterPosY - double(ns.Height/2) < GenMinimumY)
                GenMinimumY = nodepair.second.CenterPosY - double(ns.Height/2);
            if (nodepair.second.CenterPosY + double(ns.Height/2) > GenExtremeY)
                GenExtremeY = nodepair.second.CenterPosY + double(ns.Height/2);
        });

        QLineF XLine(QPointF(GenMinimumX, 0), QPointF(GenExtremeX, 0));
        QLineF YLine(QPointF(GenMinimumY, 0), QPointF(GenExtremeY, 0));
        if (XLine.length() > YLine.length()) {
            PermaScale = 1.0/XLine.length();
        } else {
            PermaScale = 1.0/YLine.length();
        }

        double NewCenterX = (GenMinimumX + GenExtremeX) / 2.0;
        double NewCenterY = (GenMinimumY + GenExtremeY) / 2.0;
        std::cerr << "[FlowWidget] Nodes are off by X " << -NewCenterX << ", Y " << -NewCenterY <<
                     ", deducted from XMin " << GenMinimumX << ", XMax " << GenExtremeX <<
                     ", YMin " << GenMinimumY << ", YMax" << GenExtremeY << std::endl;
        for (auto itr = XMLHandlr->OrphanNodes.begin(); itr != XMLHandlr->OrphanNodes.end(); itr++) {
            itr->second.CenterPosX -= NewCenterX;
            itr->second.CenterPosY -= NewCenterY;
        }
        std::cerr << "[FlowWidget] Recentered nodes. Ready!" << std::endl;
    } else {
        std::cerr << "[FlowWidget] Error loading XML." << std::endl;
    }
}

void flowwidget::SaveXML(std::ostream &data) {
    //TODO: Generate sketch XML, see file in Google Drive
}

void flowwidget::SaveSVG(std::ostream &data) {
    std::cerr << "[FlowWidget] Request to export recieved, enumerating nodes for size" << std::endl;
    int GenMinimum = INT_MAX;
    int GenExtreme = INT_MIN;
    std::for_each(XMLHandlr->OrphanNodes.begin(), XMLHandlr->OrphanNodes.end(),
                  [&](std::pair<std::string, FlowNode> nodepair) {
        NodeSize ns = getNodeSize(QFont("sans-serif", BaseFontSize*nodepair.second.FontSizeMult), QFont("sans-serif", BaseFontSize*nodepair.second.FontSizeMult*1.5), nodepair.second);

        if (nodepair.second.CenterPosX - (ns.Width/2) < GenMinimum)
            GenMinimum = nodepair.second.CenterPosX - (ns.Width/2);
        if (nodepair.second.CenterPosY - (ns.Height/2) < GenMinimum)
            GenMinimum = nodepair.second.CenterPosY - (ns.Height/2);
        if (nodepair.second.CenterPosX + (ns.Width/2) > GenExtreme)
            GenExtreme = nodepair.second.CenterPosX + (ns.Width/2);
        if (nodepair.second.CenterPosY + (ns.Height/2) > GenExtreme)
            GenExtreme = nodepair.second.CenterPosY + (ns.Height/2);
    });
    int SVGSize = GenExtreme-GenMinimum;
    std::cerr << "[FlowWidget] Calculated sheet size of " << SVGSize << std::endl;

    QSvgGenerator svgen;
    QIODevice* iodev = new IOStreamBridge(data);
    svgen.setOutputDevice(iodev);
    svgen.setSize(QSize(SVGSize, SVGSize));
    renderFullTree(&svgen, XMLHandlr,
                   LineColor, BaseFontSize,
                   SVGSize, SVGSize, PermaScale, 1, 0, 0);
    data.flush();
    std::cerr << "[FlowWidget] Finished export!" << std::endl;
}

//  Events  //

void flowwidget::paintEvent(QPaintEvent* event) {
    QPainter paint;
    paint.begin(this);
    paint.fillRect(1, 1, this->size().width()-2, this->size().height()-2, Qt::white);
    paint.end();
    renderFullTree(this, XMLHandlr,
                   LineColor, BaseFontSize,
                   this->size().width(), this->size().height(), PermaScale, ScaleFactor, PanX, PanY);
    paint.begin(this);
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.setPen(Qt::black);
    paint.drawRect(0, 0, this->size().width(), this->size().height());
}

void flowwidget::mousePressEvent(QMouseEvent* event) {
    previousMouseMove = event->localPos();
    this->update();
}

void flowwidget::mouseReleaseEvent(QMouseEvent* event) {
    this->update();
}

void flowwidget::mouseMoveEvent(QMouseEvent* event) {
    PanX += (event->localPos().x() - previousMouseMove.x()) / ScaleFactor;
    PanY += (event->localPos().y() - previousMouseMove.y()) / ScaleFactor;
    previousMouseMove = event->localPos();
    event->accept();
    this->update();
}

void flowwidget::wheelEvent(QWheelEvent* event) {
    double DeltaMult = double(event->angleDelta().y()) / double(120);
    ScaleFactor = ScaleFactor * (double(1) + (DeltaMult * 0.100));
    this->update();
}
