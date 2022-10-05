#include "Router.h"

using namespace std;
using std::vector;

namespace Router {
    Router::Router(int router_ID, int AS, vector<Interface::Interface> interfaces(), vector<int> neighbours(), vector<Peer::Peer> routing_table()) {
        this->router_ID = router_ID;
        this->AS = AS;
        this->interfaces() = interfaces();
        this->neighbours() = neighbours();
        this->routing_table() = routing_table();
    }

    int Router::get_router_ID() { return this->router_ID; }
    int Router::get_router_AD() { return this->AS; }
    vector<Interface::Interface> Router::get_router_int() { return this->interfaces(); }
    vector<int> Router::get_router_neigh() { return this->neighbours(); }
    vector<Peer::Peer> Router::get_router_rt() {return this->routing_table(); }
}