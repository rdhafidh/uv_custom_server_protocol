#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <iostream>
class MybufferIo;

class Processor
{
public:
    Processor();
    ~Processor();
    static std::string render(std::string *in,MybufferIo *io);
};

#endif // PROCESSOR_H