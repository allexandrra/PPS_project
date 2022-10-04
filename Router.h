#ifndef ROUTER_H
#define ROUTER_H

#include <string.h>
#include <stdint.h>
#include <vector.h>
#include "Interface.h"
#include "Peer.h"

using namespace Interface;
using namespace Peer;

namespace Router {
    class Router {
        public:
            Router(int router_ID, int AS, vector<Interface::Interface> interfaces(), vector<int> neighbours(), vector<Peer::Peer> routing_table());

            int get_router_ID();
            int get_router_AD();
            vector<Interface::Interface> get_router_int();
            vector<int> get_router_neigh();
            vector<Peer::Peer> get_router_rt();

        private:
            int router_ID;
            int AS;
            vector<Interface::Interface> interfaces();
            vector<int> neighbours();
            vector<Peer::Peer> routing_table();
    };
}

#endif