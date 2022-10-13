#ifndef NS3_MESSAGE_BGP_H
#define NS3_MESSAGE_BGP_H

#include <stdint.h>
#include <vector>

class MessageBGP {
    //private:
    protected:
        int8_t* marker;
        int16_t lenght;
        int8_t type;

    public:
        MessageBGP(int8_t* marker, int16_t lenght, int8_t type);
        MessageBGP();
        int8_t* get_marker();
        int16_t get_lenght();
        int8_t get_type();
}; 
#endif