#ifndef IOSTREAMBRIDGE_H
#define IOSTREAMBRIDGE_H
#include <iostream>
#include <QIODevice>

class IOStreamBridge : public QIODevice
{
public:
    IOStreamBridge(std::istream &stream);
    IOStreamBridge(std::ostream &stream);
    ~IOStreamBridge();
protected:
    virtual qint64 readData(char* data, qint64 maxSize) override;
    virtual qint64 readLineData(char* data, qint64 maxSize) override;
    virtual qint64 writeData(const char* data, qint64 maxSize) override;
    std::istream* internalistream;
    std::ostream* internalostream;
};

#endif // IOSTREAMBRIDGE_H
