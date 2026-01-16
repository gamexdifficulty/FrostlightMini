#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <stdint.h>

class Storage {
public:
    Storage();
    ~Storage();
    bool isReady() const;
    void logBattery(uint32_t time_ms, bool charging, float voltage);
private:
    bool mounted;
};

#endif
