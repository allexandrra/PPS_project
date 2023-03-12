#include "../include/Router.h"
#include "../include/MessageUpdate.h"

#include <fstream>
#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>
#include <vector>
#include <bits/stdc++.h> 

MessageUpdate::MessageUpdate(uint16_t unfeasable_route_len, std::vector<NLRIs> withdraw_routes, uint16_t total_path_atr_len, std::vector<Path_atrs> path_atr, std::vector<NLRIs> NLRI) 
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
std::vector<NLRIs> MessageUpdate::get_withdrawn_routes() { return withdrawn_routes; }
uint16_t MessageUpdate::get_total_path_atr_len() { return total_path_atr_len; }
std::vector<Path_atrs> MessageUpdate::get_path_atr() { return path_atr; }
std::vector<NLRIs> MessageUpdate::get_NLRI() { return NLRI; }

std::ostream& operator<<(std::ostream& stream, const MessageUpdate& msg) {
    std::stringstream strStreamMarker;
    std::copy(msg.marker.begin(), msg.marker.end(), std::ostream_iterator<int8_t>(strStreamMarker));

    std::stringstream strStreamData;
    
    stream << std::bitset<16>(msg.unfeasable_route_len).to_string() << " ";

    if(msg.unfeasable_route_len > 0) {
        for(NLRIs n : msg.withdrawn_routes) {
            stream << std::bitset<8>(n.prefix_lenght).to_string() << " ";
            size_t pos = 0;
            std::string token;
            std::string prefix = n.prefix;
            while((pos = prefix.find(".")) != std::string::npos) {
                token = prefix.substr(0, pos);
                stream << token;
                prefix.erase(0, pos + 1);
            }
            stream << " ";
        }
    }

    stream << std::bitset<16>(msg.total_path_atr_len).to_string() << " ";

    if(msg.total_path_atr_len > 0) {
        for(Path_atrs p : msg.path_atr) {
            std::bitset<8> bit_flags;
            if(p.optional == 1) {
                bit_flags[0] = '1';
            } else {
                bit_flags[0] = '0';
            }
            if(p.transitive == 1) {
                bit_flags[1] = '1';
            } else {
                bit_flags[1] = '0';
            }
            if(p.partial == 1) {
                bit_flags[2] = '1';
            } else {
                bit_flags[2] = '0';
            }
            if(p.extended_lenght == 1) {
                bit_flags[3] = '1';
            } else {
                bit_flags[3] = '0';
            }
            bit_flags[4] = '0';
            bit_flags[5] = '0';
            bit_flags[6] = '0';
            bit_flags[7] = '0';

            stream << bit_flags << " " << std::bitset<8> (p.type).to_string() << " "
                << std::bitset<8> (p.lenght).to_string() << " " << p.value << " ";
        }

        for(NLRIs n : msg.NLRI) {
            stream << std::bitset<8> (n.prefix_lenght).to_string() << " ";
            size_t pos = 0;
            std::string token;
            std::string prefix = n.prefix;
            while((pos = prefix.find(".")) != std::string::npos) {
                token = prefix.substr(0, pos);
                stream << token;
                prefix.erase(0, pos + 1);
            }
            stream << " ";
        }
    }    

    return stream;
}

std::istream & operator>>(std::istream & stream, MessageUpdate& msg) {
    //TODO
    // message header
    std::bitset<128> bit_marker;
    std::bitset<16> bit_length;
    std::bitset<8> bit_type;
    // update message fields
    std::bitset<16> bit_unfeasable_route_len;
    std::bitset<16> bit_total_path_atr_len;

    std::vector<uint8_t> marker(128);
    for (int i = 0; i < (int)marker.size(); i++) {
        marker[i] = (bit_marker[i] == 1) ? '1' : '0';
    }

    stream >> bit_marker >> bit_length >> bit_type >> bit_unfeasable_route_len;

    msg.unfeasable_route_len = (uint16_t)bit_unfeasable_route_len.to_ulong();

    if (msg.unfeasable_route_len > 0) {
        for(int i = 0; i < (int)msg.unfeasable_route_len; i++) {
            NLRIs new_withdrawn_route;

            std::bitset<8> length;
            std::bitset<8> prefix;

            stream >> length >> prefix;

            new_withdrawn_route.prefix_lenght = (uint8_t)length.to_ulong();

            for (int j = 0; j < 3; j++) {
                stream >> prefix;
                new_withdrawn_route.prefix.append(std::to_string((uint8_t)prefix.to_ulong()));
                new_withdrawn_route.prefix.append(".");
            }
            stream >> prefix;
            new_withdrawn_route.prefix.append(std::to_string((uint8_t)prefix.to_ulong()));

            msg.withdrawn_routes.push_back(new_withdrawn_route);
        }
    }

    stream >> bit_total_path_atr_len;

    msg.total_path_atr_len = (uint16_t)bit_total_path_atr_len.to_ulong();

    if (msg.total_path_atr_len > 0) {
        for (int i = 0; i < (int)msg.total_path_atr_len; i++) {
            Path_atrs new_path_atr;

            std::bitset<8> bit_atr_flags;
            std::bitset<8> bit_atr_type_code;
            std::bitset<8> bit_atr_len;

            stream >> bit_atr_flags >> bit_atr_type_code >> bit_length;

            new_path_atr.optional = (uint8_t)bit_atr_flags[0];
            new_path_atr.transitive = (uint8_t)bit_atr_flags[1];
            new_path_atr.partial = (uint8_t)bit_atr_flags[2];
            new_path_atr.extended_lenght = (uint8_t)bit_atr_flags[3];
            new_path_atr.lenght = (uint8_t)bit_atr_len.to_ulong();

            if(new_path_atr.extended_lenght == 0) {
                std::bitset<8> bit_atr_value;
                stream >> bit_atr_value;
                new_path_atr.value = bit_atr_value.to_string();
            } else {
                std::bitset<16> bit_atr_value;
                stream >> bit_atr_value;
                new_path_atr.value = bit_atr_value.to_string();
            }

            msg.path_atr.push_back(new_path_atr);
        }

        while (!stream.eofbit) {
            NLRIs new_nlri;

            std::bitset<8> length;
            std::bitset<8> prefix;

            stream >> length >> prefix;

            new_nlri.prefix_lenght = (uint8_t)length.to_ulong();

            for (int j = 0; j < 3; j++) {
                stream >> prefix;
                new_nlri.prefix.append(std::to_string((uint8_t)prefix.to_ulong()));
                new_nlri.prefix.append(".");
            }
            stream >> prefix;
            new_nlri.prefix.append(std::to_string((uint8_t)prefix.to_ulong()));

            msg.NLRI.push_back(new_nlri);
        }
    }

    return stream;
}

bool MessageUpdate::check_neighbour(Router router, int req_router) {
    for (int n : router.get_router_neigh()) {
        if (n == req_router) 
            return true;
    }
    return false;
}

vector<Route> MessageUpdate::check_preferences(std::vector<Route> new_routes, std::vector<Peer> routing_table) {
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
                    case 3: //originate/next hop
                        if (atr.value.compare("0.0.0.0") == 0) {
                            loc_RIB.push_back(r);
                            break;
                        }
                    case 4: //AS path length
                        if (std::stoi(atr.value) < p.AS_path_len) {
                            loc_RIB.push_back(r);
                            break;
                        }
                    case 5: // MED
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

void MessageUpdate::apply_policy(Router router, Route update_route) {
    router.update_routing_table(update_route.nlri.prefix, update_route.path_atr);
}

void MessageUpdate::add_to_RT(Router router, std::vector<Route> loc_rib) {

    for(Route r : loc_rib) {
        Peer new_peer;
        new_peer.network = r.nlri.prefix;
        new_peer.weight = 0;
        for(Path_atrs p : r.path_atr) {
            if (p.type == 2) {
                new_peer.AS_path_len = p.lenght;
                new_peer.path = p.value;
            } else if (p.type == 3) {
                new_peer.next_hop = p.value;
            } else if (p.type == 4) {
                new_peer.MED = std::stoi(p.value);
            } else if (p.type == 5) {
                new_peer.loc_pref = std::stoi(p.value);
            }
        }  
        router.push_new_route(new_peer);
    }
}

std::vector<Route> MessageUpdate::add_to_RIBin(std::vector<Path_atrs> path_atr, std::vector<NLRIs> nlri) {
    std::vector<Route> rib_in;

    for (NLRIs n : nlri) {
        Route new_route;
        new_route.nlri = n;
        for(Path_atrs p : path_atr) {
            new_route.path_atr.push_back(p);
        }
        //std::copy(path_atr.begin(), path_atr.end(), new_route.path_atr);    
        rib_in.push_back(new_route);
    }

    return rib_in;
}

void MessageUpdate::add_to_RIBout() {

}

void MessageUpdate::add_to_FIB() { 

}
