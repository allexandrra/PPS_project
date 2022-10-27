
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

struct NLRIs {
    uint8_t prefix_lenght;
    std::string prefix;
};

struct Path_atrs {
    int type;
    int lenght;
    std::string value;
    uint8_t optional;
    uint8_t transitive;
    uint8_t partial;
    uint8_t extended_lenght;
};

struct Route {
    NLRIs nlri;
    std::vector<Path_atrs> path_atr;
};

class MessageUpdate : public MessageHeader{
    private:
        uint16_t unfeasable_route_len;
        std::vector<Route> withdrawn_routes;
        uint16_t total_path_atr_len;
        std::vector<Path_atrs> path_atr;
        std::vector<NLRIs> NLRI;

    public:
        MessageUpdate(uint16_t unfeasable_route_len, std::vector<Route> withdraw_routes, uint16_t total_path_atr_len, std::vector<Path_atrs> path_atr, std::vector<NLRIs> NLRI);
        MessageUpdate();

        uint16_t get_unfeasable_route_len();
        std::vector<Route> get_withdrawn_routes();
        uint16_t get_total_path_atr_len();
        std::vector<Path_atrs> get_path_atr();
        std::vector<NLRIs> get_NLRI();

        friend std::ostream& operator<<(std::ostream& stream, const MessageUpdate& msg);
        friend std::istream & operator>>(std::istream & stream, MessageUpdate& msg);

        bool check_neighbour(Router router, int req_router);

        std::vector<Route> check_preferences(std::vector<Route> new_routes, std::vector<Peer> routing_table);

        void apply_policy();
        void add_to_RT();
        void add_to_RIBin();
        void add_to_RIBout();
        void add_to_FIB();

};
#endif