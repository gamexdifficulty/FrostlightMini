#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <cstdint>
#include <vector>
#include <string>

class Serial
{
private:
    uint8_t buf[128];
    uint8_t len;
public:
    Serial();
    void update();
    std::vector<std::string> read();
    void write(const char* output);
};

#endif