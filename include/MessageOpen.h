
#ifndef NS3_MESSAGE_OPEN_H
#define NS3_MESSAGE_OPEN_H

#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "MessageHeader.h"

class MessageOpen : public MessageHeader{
    private:
        uint16_t version;
        uint16_t my_AS;
        uint16_t hold_time;
        uint32_t BGP_id;
        uint16_t opt_param_len;
        std::vector<uint8_t> opt_param;

    public:
        MessageOpen(uint16_t AS_number, uint16_t hold_time, uint32_t BGP_id, uint16_t opt_param_len, std::vector<uint8_t> opt_param);
        MessageOpen(uint16_t AS_number, uint16_t hold_time, uint32_t BGP_id, uint16_t opt_param_len);
        MessageOpen();

        uint16_t get_version();
        uint16_t get_AS();
        uint16_t get_hold_time();
        uint32_t get_BGP_id();
        uint16_t get_opt_param_len();
        std::vector<uint8_t> get_opt_param();

        friend std::ostream& operator<<(std::ostream& stream, const MessageOpen& msg);
        friend std::istream & operator>>(std::istream & stream, MessageOpen& msg);
};

#endif