#include "iostreambridge.h"

IOStreamBridge::IOStreamBridge(std::istream &stream)
{
    internalistream = &stream;
}

IOStreamBridge::IOStreamBridge(std::ostream &stream)
{
    internalostream = &stream;
}

IOStreamBridge::~IOStreamBridge()
{

}

qint64 IOStreamBridge::readData(char* data, qint64 maxSize) {
    internalistream->read(data, maxSize);
    return internalistream->gcount();
}

qint64 IOStreamBridge::readLineData(char* data, qint64 maxSize) {
    return readData(data, maxSize);
}

qint64 IOStreamBridge::writeData(const char* data, qint64 maxSize) {
    internalostream->write(data, maxSize);
    return maxSize;
}
