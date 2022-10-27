#include "../include/Router.h"
#include "../include/MessageUpdate.h"

#include <fstream>
#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>
#include <vector>

MessageUpdate::MessageUpdate(uint16_t unfeasable_route_len, std::vector<Route> withdraw_routes, uint16_t total_path_atr_len, std::vector<Path_atrs> path_atr, std::vector<NLRIs> NLRI) 
: MessageHeader(2) {
    this->unfeasable_route_len = unfeasable_route_len;
    this->withdrawn_routes.assign(withdraw_routes.begin(), withdraw_routes.end());
    this->total_path_atr_len = total_path_atr_len;
    this->path_atr.assign(path_atr.begin(), path_atr.end());
    this->NLRI.assign(NLRI.begin(), NLRI.end());
}

MessageUpdate::MessageUpdate() : MessageHeader(2) {
    //TODO
    this->unfeasable_route_len = 0;
    this->withdrawn_routes.resize(0);
    this->total_path_atr_len = 0;
    this->path_atr.resize(0);
    this->NLRI.resize(0);
}


uint16_t MessageUpdate::get_unfeasable_route_len() { return unfeasable_route_len; }
std::vector<Route> MessageUpdate::get_withdrawn_routes() { return withdrawn_routes; }
uint16_t MessageUpdate::get_total_path_atr_len() { return total_path_atr_len; }
std::vector<Path_atrs> MessageUpdate::get_path_atr() { return path_atr; }
std::vector<NLRIs> MessageUpdate::get_NLRI() { return NLRI; }

std::ostream& operator<<(std::ostream& stream, const MessageUpdate& msg) {
    //TODO
    std::stringstream strStreamData;
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
                        if (std::stoi(atr.value) < p.weight) {
                            loc_RIB.push_back(r);
                            break;
                        }
                    case 2: //loc_pref
                        if (std::stoi(atr.value) < p.loc_pref) {
                            loc_RIB.push_back(r);
                            break;
                        }
                    case 3: // MED
                        if (std::stoi(atr.value) < p.MED) {
                            loc_RIB.push_back(r);
                            break;
                        }
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
