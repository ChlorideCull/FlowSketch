#ifndef SKETCHXML_H
#define SKETCHXML_H
#include "flownode.h"
#include <QXmlSimpleReader>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <stack>

class SketchXMLHandler : public QXmlDefaultHandler {
public:
    virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts) override;
    virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;
    virtual bool characters(const QString &ch) override;
    virtual bool error(const QXmlParseException &exception) override;
    virtual bool fatalError(const QXmlParseException &exception) override;
    std::unordered_map<std::string, FlowNode> OrphanNodes;
    std::unordered_map<std::string, std::vector<std::string>> nodelinkmap;
private:
    QString CurrentNodeID;
    std::stack<QString> InNode;
    std::vector<std::vector<std::uint8_t>> ColorMap;
};

#endif // SKETCHXML_H
