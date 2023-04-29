#include "../include/Router.h"
#include "../include/MessageUpdate.h"

#include <fstream>
#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>
#include <vector>
#include <bits/stdc++.h> 

/**
 * @brief Constructors for MessageUpdate with different parameters
*/
MessageUpdate::MessageUpdate(uint16_t unfeasable_route_len, std::vector<NLRIs> withdraw_routes, uint16_t total_path_atr_len, std::vector<Path_atrs> path_atr, std::vector<NLRIs> NLRI) 
: MessageHeader(MESSAGE_HEADER_LEN+MIN_MESSAGE_UPDATE_LEN, 2) {
    this->unfeasable_route_len = unfeasable_route_len;
    this->withdrawn_routes.assign(withdraw_routes.begin(), withdraw_routes.end());
    this->total_path_atr_len = total_path_atr_len;
    this->path_atr.assign(path_atr.begin(), path_atr.end());
    this->NLRI.assign(NLRI.begin(), NLRI.end());
}

MessageUpdate::MessageUpdate(uint16_t total_path_atr_len, std::vector<Path_atrs> path_atr, std::vector<NLRIs> NLRI) 
: MessageHeader(MESSAGE_HEADER_LEN+MIN_MESSAGE_UPDATE_LEN, 2) {
    this->unfeasable_route_len = 0;
    this->withdrawn_routes.resize(0);
    this->total_path_atr_len = total_path_atr_len;
    this->path_atr.assign(path_atr.begin(), path_atr.end());
    this->NLRI.assign(NLRI.begin(), NLRI.end());
}

MessageUpdate::MessageUpdate(uint16_t unfeasable_route_len, std::vector<NLRIs> withdraw_routes) 
: MessageHeader(MESSAGE_HEADER_LEN+MIN_MESSAGE_UPDATE_LEN, 2) {
    this->unfeasable_route_len = unfeasable_route_len;
    this->withdrawn_routes.assign(withdraw_routes.begin(), withdraw_routes.end());
    this->total_path_atr_len = 0;
    this->path_atr.resize(0);
    this->NLRI.resize(0);
}

MessageUpdate::MessageUpdate() : MessageHeader(2) {
    this->unfeasable_route_len = 0;
    this->withdrawn_routes.resize(0);
    this->total_path_atr_len = 0;
    this->path_atr.resize(0);
    this->NLRI.resize(0);
}

/**
 * @brief Define the getters of the various param of the class
*/
uint16_t MessageUpdate::get_unfeasable_route_len() { return unfeasable_route_len; }
std::vector<NLRIs> MessageUpdate::get_withdrawn_routes() { return withdrawn_routes; }
uint16_t MessageUpdate::get_total_path_atr_len() { return total_path_atr_len; }
std::vector<Path_atrs> MessageUpdate::get_path_atr() { return path_atr; }
std::vector<NLRIs> MessageUpdate::get_NLRI() { return NLRI; }

/**
 * @brief Overload of the << operator to print the MessageTrustrate fields into a binary bitstream
 * @param stream the output stream
 * @param msg the MessageOpen to print
 * @return the output stream
*/
std::ostream& operator<<(std::ostream& stream, const MessageUpdate& msg) {
    std::stringstream strStreamMarker;
    std::copy(msg.marker.begin(), msg.marker.end(), std::ostream_iterator<int8_t>(strStreamMarker));
    
    stream << std::bitset<128>(strStreamMarker.str()).to_string() << " " 
            << std::bitset<16>(msg.lenght).to_string() << " " 
            << std::bitset<8>(msg.type).to_string() << " ";

    stream << std::bitset<16>(msg.unfeasable_route_len).to_string() << " ";

    // read unfeasable_routes
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
            stream << prefix;
            stream << " ";
        }
    }

    stream << std::bitset<16>(msg.total_path_atr_len).to_string() << " ";

    // read path_atr
    if(msg.total_path_atr_len > 0) {
        for(Path_atrs p : msg.path_atr) {
            std::string aux = "";
            if(p.optional == 1) {
                aux.append("1");
            } else {
                aux.append("0");
            }
            if(p.transitive == 1) {
                aux.append("1");
            } else {
                aux.append("0");
            }
            if(p.partial == 1) {
                aux.append("1");
            } else {
                aux.append("0");
            }
            if(p.extended_lenght == 1) {
                aux.append("1");
            } else {
                aux.append("0");
            }
            aux.append("0000");

            std::bitset<8> bit_flags(aux);
            stream << bit_flags << " " << std::bitset<8>(p.type).to_string() << " "
                << std::bitset<8>(p.lenght).to_string() << " " << p.value << " ";
        }

        // read NLRI of the path_atr
        for(NLRIs n : msg.NLRI) {
            stream << std::bitset<8>(n.prefix_lenght).to_string() << " ";
            size_t pos = 0;
            std::string token;
            std::string prefix = n.prefix;
            while((pos = prefix.find(".")) != std::string::npos) {
                token = prefix.substr(0, pos);
                stream << token;
                prefix.erase(0, pos + 1);
            }
            stream << prefix;
            stream << " ";
        }
    }    

    return stream;
}


/**
 * @brief Overload of the >> operator to read the MessageTrustrate fields from a binary bitstream
 * @param stream the input stream
 * @param msg the MessageOpen to read
 * @return the input stream
*/
std::istream & operator>>(std::istream & stream, MessageUpdate& msg) {
    // message header
    std::bitset<128> bit_marker;
    std::bitset<16> bit_length;
    std::bitset<8> bit_type;
    // update message fields
    std::bitset<16> bit_unfeasable_route_len;
    std::bitset<16> bit_total_path_atr_len;

    std::vector<uint8_t> marker(128);

    stream >> bit_marker >> bit_length >> bit_type >> bit_unfeasable_route_len;

    for (int i = 0; i < (int)marker.size(); i++) {
        marker[i] = (bit_marker[i] == 1) ? '1' : '0';
    }

    msg.marker = marker;
    msg.lenght = (uint16_t)bit_length.to_ulong();
    msg.type = (uint16_t)bit_type.to_ulong();

    msg.unfeasable_route_len = (uint16_t)bit_unfeasable_route_len.to_ulong();

    // insert unfeasable_routes
    if (msg.unfeasable_route_len > 0) {
        for(int i = 0; i < (int)msg.unfeasable_route_len; i++) {
            NLRIs new_withdrawn_route;

            std::bitset<8> length;
            std::string prefix;

            stream >> length >> prefix;

            new_withdrawn_route.prefix_lenght = (uint8_t)length.to_ulong();

            for (int j = 0; j < 3; j++) {
                new_withdrawn_route.prefix.append(prefix.substr(j,1));
                new_withdrawn_route.prefix.append(".");
            }
            stream >> prefix;
            new_withdrawn_route.prefix.append(prefix.substr(3,1));

            msg.withdrawn_routes.push_back(new_withdrawn_route);
        }
    }

    stream >> bit_total_path_atr_len;

    msg.total_path_atr_len = (uint16_t)bit_total_path_atr_len.to_ulong();

    // insert path_atr
    if (msg.total_path_atr_len > 0) {
        for (int i = 0; i < (int)msg.total_path_atr_len; i++) {
            Path_atrs new_path_atr;

            std::bitset<8> bit_atr_flags;
            std::bitset<8> bit_atr_type_code;
            std::bitset<8> bit_atr_len;

            std::string bit_atr_value;

            stream >> bit_atr_flags >> bit_atr_type_code >> bit_atr_len >> bit_atr_value;

            std::string flags = bit_atr_flags.to_string();

            new_path_atr.optional = atoi(flags.substr(0,1).c_str());
            new_path_atr.transitive = atoi(flags.substr(1,1).c_str());;
            new_path_atr.partial = atoi(flags.substr(2,1).c_str());;
            new_path_atr.extended_lenght = atoi(flags.substr(3,1).c_str());;

            new_path_atr.lenght = (uint8_t)bit_atr_len.to_ulong();
            new_path_atr.type = (uint8_t)bit_atr_type_code.to_ulong();
            new_path_atr.value = bit_atr_value;

            msg.path_atr.push_back(new_path_atr);
        }

        // insert NLRI of the path_atr
        for (int i = 0; i < msg.total_path_atr_len/6; i++) {
            NLRIs new_nlri;

            std::bitset<8> length;
            std::string prefix;

            stream >> length >> prefix;

            new_nlri.prefix_lenght = (uint8_t)length.to_ulong();

            for (int j = 0; j < 3; j++) {
                new_nlri.prefix.append(prefix.substr(j,1));
                new_nlri.prefix.append(".");
            }
            new_nlri.prefix.append(prefix.substr(3,1));

            msg.NLRI.push_back(new_nlri);
        }
    }

    return stream;
}

/**
 * @brief Function to check if the router is a neighbour
 * @param router the router to check
 * @param req_router index of router to check
 * @return true if the router is a neighbour, false otherwise
*/
bool MessageUpdate::check_neighbour(Router router, int req_router) {
    for (int n : router.get_router_neigh()) {
        if (n == req_router) 
            return true;
    }
    return false;
}

/**
 * @brief Function to check local the preferences of the router
 * @param new_routes the new routes to check
 * @param routing_table the routing table of the router
 * @return the new routes with the preferences
 */
vector<Route> MessageUpdate::check_preferences(std::vector<Route> new_routes, std::vector<Peer> routing_table) {
    vector<Route> loc_RIB;
    int is_new = 0;
    for (Route r : new_routes) {
        for (Peer p : routing_table) {
            if (r.nlri.prefix == p.network) {
                // compare preferences
                is_new = 1;
                for (Path_atrs atr : r.path_atr) {
                    switch (atr.type) {
                        case 1: //weight
                            if (std::stoi(atr.value) < p.weight) {
                                loc_RIB.push_back(r);
                            }
                            break;
                        case 2: //loc_pref
                            if (std::stoi(atr.value) < p.loc_pref) {
                                loc_RIB.push_back(r);
                            }
                            break;
                        case 3: //originate/next hop
                            if (atr.value == "0.0.0.0") {
                                loc_RIB.push_back(r);
                            }
                            break;
                        case 4: //AS path length
                            if (std::stoi(atr.value) < p.AS_path_len) {
                                loc_RIB.push_back(r);
                            }
                            break;
                        case 5: // MED
                            if (std::stoi(atr.value) < p.MED) {
                                loc_RIB.push_back(r);
                            }
                            break;
                        case 6: // trust
                            if (std::stoi(atr.value)/100.0 > p.trust) {
                                loc_RIB.push_back(r);
                            }
                            break;
                        default:
                            std::cout << "No prefereces met.\n\n";
                    }
                }
            } 
        }

        if(is_new == 0) {
            loc_RIB.push_back(r);
        } else {
            is_new = 0;
        }
    }

    return loc_RIB;
}

/**
 * @brief Function to add the new routes to the RIBin
 * @param path_atr the path attributes of the new routes
 * @param nlri the nlri of the new routes
 * @return the new routes
 */
std::vector<Route> MessageUpdate::add_to_RIBin(std::vector<Path_atrs> path_atr, std::vector<NLRIs> nlri) {
    std::vector<Route> rib_in;
    int i = 0;

    for (NLRIs n : nlri) {
        Route new_route;
        new_route.nlri = n;
        for(int j = i; j < i+6; j++) {
            new_route.path_atr.push_back(path_atr[j]);
        }
        i+=6;
        rib_in.push_back(new_route);
    }

    return rib_in;
}