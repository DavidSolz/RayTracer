#ifndef ILOG_H
#define ILOG_H

enum MessageType{
    INFO,
    WARNING,
    ISSUE,
};

class ILog{
public:
    virtual void Write(MessageType _type, const char * _data) = 0;
};


#endif