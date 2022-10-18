#include "../include/Router.h"

Router::Router(int router_ID, int AS, std::vector<Interface> interfaces, std::vector<int> neighbours, std::vector<Peer> routing_table) {
    this->router_ID = router_ID;
    this->AS = AS;
    this->interfaces = interfaces;
    this->neighbours = neighbours;
    this->routing_table = routing_table;
}

int Router::get_router_ID() { return this->router_ID; }
int Router::get_router_AS() { return this->AS; }
std::vector<Interface> Router::get_router_int() { return this->interfaces; }
std::vector<int> Router::get_router_neigh() { return this->neighbours; }
std::vector<Peer> Router::get_router_rt() {return this->routing_table; }