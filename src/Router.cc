#include "../include/Router.h"

Router::Router(int router_ID, int AS, std::vector<Interface> interfaces,
               std::vector<int> neighbours, std::vector<Peer> routing_table) {
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
std::vector<Peer> Router::get_router_rt() { return this->routing_table; }

void Router::push_new_route(Peer new_route) {
  this->routing_table.push_back(new_route);
}

void Router::update_routing_table(std::string network, std::vector<Path_atrs> atrib) {
  for(Peer p : this->routing_table) {
    if(p.network == network) {
      for(Path_atrs atr : atrib) {
        if(atr.type == 1) {
          p.weight = std::stoi(atr.value);
        } else if(atr.type == 2) {
          p.AS_path_len = atr.lenght;
          p.path = atr.value;
        } else if(atr.type == 3) {
          p.next_hop = atr.value;
        } else if(atr.type == 4) {
          p.MED = std::stoi(atr.value);
        } else if(atr.type == 5) {
          p.loc_pref = std::stoi(atr.value);
        }
      }
    }
  }
}

