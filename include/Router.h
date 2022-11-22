#ifndef NS3_ROUTER_H
#define NS3_ROUTER_H

#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>
#include <vector>

#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/tcp-header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/node.h"

using namespace ns3;
using namespace std;
// include the namespace ns3 for it to wotk
// look at examples

namespace ns3 {
    struct Peer {
        std::string network;
        std::string next_hop;
        std::string path;
        int weight;
        int loc_pref;
        int AS_path_len;
        int MED;
        float trust;
    };

    struct Interface {
        std::string name;
        //std::string ip;
        //std::string mask;
        Ipv4AddressHelper ip_address;
        bool status;
    };

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


    class Router : public Node{
        public:
            Router(int router_ID, int AS, std::vector<Interface> interfaces, std::vector<int> neighbours, std::vector<Peer> routing_table);

            int get_router_ID();
            int get_router_AS();
            float get_router_trust();
            std::vector<Interface> get_router_int();
            std::vector<int> get_router_neigh();
            std::vector<Peer> get_router_rt();
            void push_new_route(Peer new_route);
            void update_routing_table(std::string network, std::vector<Path_atrs> atribs);

        private:
            int router_ID = Node::GetId();
            int AS;
            std::vector<Interface> interfaces;
            std::vector<int> neighbours;
            std::vector<Peer> routing_table;
    };
}
#endif