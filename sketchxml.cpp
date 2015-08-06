#include "sketchxml.h"
#include "iostream"
#include <cstdlib>

bool SketchXMLHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts) {
    if (qName == "sketch") {
        std::cerr << "[SketchXML] XML Version: " << atts.value("version").toStdString() << std::endl;
    } else if (qName == "project") {
        std::cerr << "[SketchXML] Project Version: " << atts.value("version").toStdString() << std::endl;
    } else if (qName == "theme") {
        if (InNode.top() == "colorTheme") {
            std::string hexval = atts.value("rgb").toStdString();
            std::uint8_t* color = new std::uint8_t[4];

            std::vector<std::string> hexgroups;
            std::string tmpstr = "";
            char tmpstrcnt = 0;

            std::for_each(hexval.begin(), hexval.end(), [&](char chr){
                if (chr == '#')
                    return;
                else {
                    tmpstr += chr;
                    tmpstrcnt += 1;
                    if (tmpstrcnt == 2) {
                        hexgroups.insert(hexgroups.end(), tmpstr);
                        tmpstr = "";
                        tmpstrcnt = 0;
                    }
                }
            });

            for (int i = 0; i < 3; i++) {
                unsigned long rawval = std::strtoul(hexgroups[i].c_str(), nullptr, 16);
                color[i] = char(rawval);
            }
            color[3] = 255;

            std::uint8_t cmindex = atoi(atts.value("id").toStdString().c_str());
            ColorMap.insert(ColorMap.cend(), std::vector<std::uint8_t>());
            ColorMap[cmindex].insert(ColorMap[cmindex].cend(), color[0]);
            ColorMap[cmindex].insert(ColorMap[cmindex].cend(), color[1]);
            ColorMap[cmindex].insert(ColorMap[cmindex].cend(), color[2]);
            ColorMap[cmindex].insert(ColorMap[cmindex].cend(), color[3]);

            //QT Creator doesn't handle std::hex that well
            std::cerr << "[SXML-ColorMap] Added color " << int(cmindex) << " with RGBA " << hexval << "FF" << std::endl;
        }
    } else if (qName == "shape") {
        if (InNode.top() == "shapes") {
            FlowNode fnode;
            std::vector<std::uint8_t> tmpkeep = ColorMap[atoi(atts.value("color").toStdString().c_str())];
            fnode.ColorRGBA[0] = tmpkeep[0];
            fnode.ColorRGBA[1] = tmpkeep[1];
            fnode.ColorRGBA[2] = tmpkeep[2];
            fnode.ColorRGBA[3] = tmpkeep[3];
            fnode.Type = (NodeType)atoi(atts.value("kind").toStdString().c_str());
            CurrentNodeID = atts.value("guid");
            fnode.GUID = CurrentNodeID.toStdString();
            fnode.FontSizeMult = 0.75 + (double(atoi(atts.value("fontSize").toStdString().c_str())) * 0.25);
            OrphanNodes[CurrentNodeID.toStdString()] = fnode;
        }
    } else if (qName == "center") {
        if (InNode.top() == "shape") {
            FlowNode fnode = OrphanNodes[CurrentNodeID.toStdString()];
            fnode.CenterPosX = atts.value("x").toDouble();
            fnode.CenterPosY = atts.value("y").toDouble();
            OrphanNodes[CurrentNodeID.toStdString()] = fnode;
        }
    } else if (qName == "link") {
        if (InNode.top() == "links") {
            if (nodelinkmap.count(atts.value("parentShape").toStdString()) == 0) {
                nodelinkmap[atts.value("parentShape").toStdString()] = std::vector<std::string>();
            }
            nodelinkmap[atts.value("parentShape").toStdString()].insert(
                        nodelinkmap[atts.value("parentShape").toStdString()].cend(),
                        atts.value("childShape").toStdString());
        }
    } else if (qName == "sideLink") {
        if (InNode.top() == "sideLinks") {
            if (nodelinkmap.count(atts.value("backShape").toStdString()) == 0) {
                nodelinkmap[atts.value("backShape").toStdString()] = std::vector<std::string>();
            }
            nodelinkmap[atts.value("backShape").toStdString()].insert(
                        nodelinkmap[atts.value("backShape").toStdString()].cend(),
                        atts.value("sideShape").toStdString());
        }
    }
    InNode.push(qName);
    return true;
}

bool SketchXMLHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName) {
    if (InNode.top() != qName)
        std::cerr << "[SketchXML] Element ended in the wrong order, still popping! See node " << qName.toStdString() << std::endl;
    InNode.pop();
    return true;
}

bool SketchXMLHandler::characters(const QString &ch) {
    if (InNode.top() == "name") {
        FlowNode fnode = OrphanNodes[CurrentNodeID.toStdString()];
        fnode.Title += ch.toStdString();
        OrphanNodes[CurrentNodeID.toStdString()] = fnode;
    } else if (InNode.top() == "notes") {
        FlowNode fnode = OrphanNodes[CurrentNodeID.toStdString()];
        fnode.Desc += ch.toStdString();
        OrphanNodes[CurrentNodeID.toStdString()] = fnode;
    }
    return true;
}

bool SketchXMLHandler::error(const QXmlParseException &exception) {
    std::cerr << "[SketchXML] Parser error: " << exception.message().toStdString() << std::endl;
    std::cerr << "Line " << exception.lineNumber() << ", column " << exception.columnNumber() << std::endl;
    return true;
}

bool SketchXMLHandler::fatalError(const QXmlParseException &exception) {
    std::cerr << "[SketchXML] Fatal parser error: " << exception.message().toStdString() << std::endl;
    std::cerr << "Line " << exception.lineNumber() << ", column " << exception.columnNumber() << std::endl;
    return true;
}
