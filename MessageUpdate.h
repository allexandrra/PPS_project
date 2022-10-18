#ifndef NS3_MESSAGE_UPDATE_H
#define NS3_MESSAGE_UPDATE_H

#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "MessageHeader.h"

class MessageUpdate : public MessageHeader{
    private:
        uint16_t unfeasable_route_len;
        std::vector<int> withdrawn_routes;
        uint16_t total_path_atr_len;
        std::vector<int> path_atr;
        std::vector<int> NLRI;

    public:
        MessageUpdate(uint16_t unfeasable_route_len, std::vector<int> withdraw_routes, uint16_t total_path_atr_len, std::vector<int> path_atr, std::vector<int> NLRI);
        MessageUpdate();

        uint16_t get_unfeasable_route_len();
        std::vector<int> get_withdrawn_routes();
        uint16_t get_total_path_atr_len();
        std::vector<int> get_path_atr();
        std::vector<int> get_NLRI();

        friend std::ostream& operator<<(std::ostream& stream, const MessageUpdate& msg);
        friend std::istream & operator>>(std::istream & stream, MessageUpdate& msg);
};
#endif