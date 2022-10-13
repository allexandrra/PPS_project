#ifndef ROUTER_H
#define ROUTER_H

#include <fstream>
#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>
#include <vector>

#include "Interface.h"
#include "Peer.h"

using namespace std;

class Router {
    public:
        Router(int router_ID, int AS, std::vector<Interface> interfaces, std::vector<int> neighbours, std::vector<Peer> routing_table);

        int get_router_ID();
        int get_router_AD();
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