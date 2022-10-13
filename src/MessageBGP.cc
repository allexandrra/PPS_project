#include <stdint.h>
#include <vector>

#include "MessageBGP.h"


MessageBGP::MessageBGP(int8_t* marker, int16_t lenght, int8_t type) {
    this->marker = marker;
    this->lenght = lenght;
    this->type = type;
}

MessageBGP::MessageBGP() {
    this->marker = 0;
    this->lenght = 0;
    this->type = 0;
}

int8_t* MessageBGP::get_marker() { return marker; }
int16_t MessageBGP::get_lenght() { return lenght; }
int8_t MessageBGP::get_type() { return type; }