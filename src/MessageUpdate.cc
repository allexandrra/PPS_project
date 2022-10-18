#include "../include/Router.h"
#include "../include/MessageUpdate.h"

#include <fstream>
#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>
#include <vector>

MessageUpdate::MessageUpdate(uint16_t unfeasable_route_len, std::vector<int> withdraw_routes, uint16_t total_path_atr_len, std::vector<int> path_atr, std::vector<int> NLRI) 
: MessageHeader() {
    this->unfeasable_route_len = unfeasable_route_len;
    this->withdrawn_routes = withdrawn_routes;
    this->total_path_atr_len = total_path_atr_len;
    this->path_atr = path_atr;
    this->NLRI = NLRI;
}

MessageUpdate::MessageUpdate() : MessageHeader() {
    //TODO
}


uint16_t MessageUpdate::get_unfeasable_route_len() { return unfeasable_route_len; }
std::vector<int> MessageUpdate::get_withdrawn_routes() { return withdrawn_routes; }
uint16_t MessageUpdate::get_total_path_atr_len() { return total_path_atr_len; }
std::vector<int> MessageUpdate::get_path_atr() { return path_atr; }
std::vector<int> MessageUpdate::get_NLRI() { return NLRI; }

std::ostream& operator<<(std::ostream& stream, const MessageUpdate& msg) {
    //TODO
    return stream;
}

std::istream & operator>>(std::istream & stream, MessageUpdate& msg) {
    //TODO
    return stream;
}

bool MessageUpdate::check_neighbour(Router router, int req_router) {
    for (int n : router.get_router_neigh()) {
        if (n == req_router) 
            return true;
    }
    return false;
}

vector<Route> MessageUpdate::check_preferences(vector<Route> new_routes, vector<Peer> routing_table) {
    vector<Route> loc_RIB;
    for (Route r : new_routes) {
        for (Peer p : routing_table) {
            if (r.nlri.prefix.compare(p.network) == 0) {
                // compare preferences
                for (Path_atrs atr : r.path_atr) {
                    switch (atr.type) {
                    case 1: //weight
                        // compara greutatea
                        // daca mai mare treci mai departe
                        // daca mai mica inlocuieste ruta actuala cu ruta noua in tabelul de rutare
                        break;
                    case 2: //loc_pref
                        break;
                    case 3: // 
                        break;
                    default:
                        break;
                    }
                }
            } else {
                loc_RIB.push_back(r);
                break;
            }
        }
    }

    return loc_RIB;
}

void MessageUpdate::apply_policy() {

}

void MessageUpdate::add_to_RT() {

}

void MessageUpdate::add_to_RIBin() {
    
}

void MessageUpdate::add_to_RIBout() {

}

void MessageUpdate::add_to_FIB() { 

}
