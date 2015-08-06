#ifndef FLOWNODE
#define FLOWNODE

#include <vector>
#include <string>
#include <cstdint>

enum NodeType {
    NONE = 0,
    BASIC_RECT = 2,
    CONTAINER = 3
};

struct FlowNode {
    std::string GUID;
    std::string Title;
    std::string Desc;

    std::uint8_t ColorRGBA[4];
    double FontSizeMult;
    NodeType Type;
    double CenterPosX;
    double CenterPosY;
};

#endif // FLOWNODE

