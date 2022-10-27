#ifndef NS3_MESSAGE_TRUSTRATE_H
#define NS3_MESSAGE_TRUSTRATE_H

#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

/* To add : 
- New metric in routing table : trust to take into account to choose best path

*/
class MessageTrustrate{
    private:
        uint16_t trust;
    public:
        MessageTrustrate();


};
#endif