#ifndef NS3_MESSAGE_UPDATE_H
#define NS3_MESSAGE_UPDATE_H

#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "MessageHeader.h"
#include "Router.h"

#define MIN_MESSAGE_UPDATE_LEN 16

class MessageUpdate : public MessageHeader{
    private:
        uint16_t unfeasable_route_len;
        std::vector<NLRIs> withdrawn_routes;
        uint16_t total_path_atr_len;
        std::vector<Path_atrs> path_atr;
        std::vector<NLRIs> NLRI;

    public:
        MessageUpdate(uint16_t unfeasable_route_len, std::vector<NLRIs> withdraw_routes, uint16_t total_path_atr_len, std::vector<Path_atrs> path_atr, std::vector<NLRIs> NLRI);
        MessageUpdate(uint16_t total_path_atr_len, std::vector<Path_atrs> path_atr, std::vector<NLRIs> NLRI);
        MessageUpdate(uint16_t unfeasable_route_len, std::vector<NLRIs> withdraw_routes);
        MessageUpdate();

        uint16_t get_unfeasable_route_len();
        std::vector<NLRIs> get_withdrawn_routes();
        uint16_t get_total_path_atr_len();
        std::vector<Path_atrs> get_path_atr();
        std::vector<NLRIs> get_NLRI();

        friend std::ostream& operator<<(std::ostream& stream, const MessageUpdate& msg);
        friend std::istream& operator>>(std::istream & stream, MessageUpdate& msg);

        bool check_neighbour(Router router, int req_router);

        std::vector<Route> check_preferences(std::vector<Route> new_routes, std::vector<Peer> routing_table);
        std::vector<Route> add_to_RIBin(std::vector<Path_atrs> path_atr, std::vector<NLRIs> nlri);
        void add_to_RIBout();
        void add_to_FIB();

};
#endif