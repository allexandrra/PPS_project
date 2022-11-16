#ifndef ROUTER_H
#define ROUTER_H

#include <fstream>
#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>
#include <vector>
#include "ns3/node-container.h"
#include "ns3/ipv4-address.h"

//#include "Interface.h"
//#include "Peer.h"

struct Peer {
    std::string network;
    std::string next_hop;
    std::string path;
    int weight;
    int loc_pref;
    int AS_path_len;
    char origin_code;
    int MED;
    bool e_vs_i;
    float trust;
};

using namespace ns3;

    namespace ns3 {
        struct Interface {
        std::string name;
        Ipv4Address local_ip;
        Ipv4Address remote_ip;
        Ipv4Address mask;
        bool status;

        Interface(std::string name, Ipv4Address local_ip, Ipv4Address remote_ip, Ipv4Address mask) : status(true) {
            this->name = name;
            this->local_ip = local_ip;
            this->remote_ip = remote_ip;
            this->mask = mask;
        }

    };

    class Router {
        public:
            Router(int router_ID, int AS, std::vector<Interface> interfaces, std::vector<int> neighbours, std::vector<Peer> routing_table, NodeContainer node, Ipv4Address ASip, Ipv4Mask ASmask);
            Router(int router_ID, int AS, NodeContainer node, Ipv4Address ASip, Ipv4Mask ASmask);

            int get_router_ID();
            int get_router_AS();
            float get_router_trust();
            std::vector<Interface> get_router_int();
            std::vector<int> get_router_neigh();
            std::vector<Peer> get_router_rt();
            NodeContainer get_router_node();
            Ipv4Address get_router_ASip();
            Ipv4Mask get_router_ASmask();
            void add_interface(Interface interface);
            void add_neighbour(int neighbour);

        private:
            int router_ID;
            int AS;
            std::vector<Interface> interfaces;
            std::vector<int> neighbours;
            std::vector<Peer> routing_table;
            NodeContainer node;
            Ipv4Address ASip;
            Ipv4Mask ASmask;
        
    };

} // namespace ns3

#endif