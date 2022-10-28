#ifndef ROUTER_H
#define ROUTER_H

#include <fstream>
#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>
#include <vector>

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

struct Interface {
    std::string name;
    std::string ip;
    std::string mask;
    bool status;
};

using namespace std;

class Router {
    public:
        Router(int router_ID, int AS, std::vector<Interface> interfaces, std::vector<int> neighbours, std::vector<Peer> routing_table);

        int get_router_ID();
        int get_router_AS();
        float get_router_trust();
        std::vector<Interface> get_router_int();
        std::vector<int> get_router_neigh();
        std::vector<Peer> get_router_rt();

    private:
        int router_ID;
        int AS;
        std::vector<Interface> interfaces;
        std::vector<int> neighbours;
        std::vector<Peer> routing_table;
};

#endif