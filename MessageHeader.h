#ifndef NS3_MESSAGE_HEADER_H
#define NS3_MESSAGE_HEADER_H

#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>


class MessageHeader {
    protected:
        std::vector<uint8_t> marker;
        uint16_t lenght;
        uint16_t type;

    public:
        MessageHeader(std::vector<uint8_t> marker, uint16_t lenght, uint16_t type);
        MessageHeader(uint16_t lenght, uint16_t type);
        MessageHeader(uint16_t type);
        MessageHeader();

        std::vector<uint8_t> get_marker();
        uint16_t get_lenght();
        uint16_t get_type();
        void set_market(std::vector<uint8_t> marker);
        void set_lenght(uint16_t lenght);
        void set_type(uint16_t type);

        friend std::ostream& operator<<(std::ostream& stream, const MessageHeader& msg);
        friend std::istream& operator>>(std::istream & stream, MessageHeader& msg);
};

#endif