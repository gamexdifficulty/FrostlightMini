#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <cstdint>

class Serial
{
private:
    uint8_t buf[128];
    uint8_t len;
public:
    Serial();
    void update();
    bool read(char* command);
    void write(char* output);
};

#endif