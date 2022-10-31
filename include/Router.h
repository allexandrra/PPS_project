#ifndef ROUTER_H
#define ROUTER_H

#include <fstream>
#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>
#include <vector>

#include "ns3/node.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"

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

using namespace std;

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

    private:
        int router_ID = Node::GetId();
        int AS;
        std::vector<Interface> interfaces;
        std::vector<int> neighbours;
        std::vector<Peer> routing_table;
};

#endif