#ifndef NS3_ROUTER_H
#define NS3_ROUTER_H

#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>
#include <vector>

#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/tcp-header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-address.h"
#include "../include/BGP-server.h"
#include "../include/BGP-client.h"


using namespace ns3;
using namespace std;
// include the namespace ns3 for it to wotk
// look at examples

namespace ns3 {
    struct Peer {
        std::string network;
        std::string next_hop;
        std::string path;
        int weight;
        int loc_pref;
        int AS_path_len;
        int MED;
        float trust;
    };

    struct Interface {
        std::string name;
        //std::string ip;
        //std::string mask;
        Ipv4Address ip_address;
        Ipv4Address mask;
        Ipv4Address remote_ip;
        bool status;
        std::optional<Ptr<BGPClient>> client;
        std::optional<Ptr<BGPServer>> server;
        bool isServer;
        int max_hold_time;
        int current_hold_time;

        Interface(std::string name, Ipv4Address ip_address, Ipv4Address remote_ip, Ipv4Address mask) : status(true), isServer(false) {
            this->name = name;
            this->ip_address = ip_address;
            this->remote_ip = remote_ip;
            this->mask = mask;
            this->max_hold_time = 0;
            this->current_hold_time = 0;
        }

        int get_max_hold_time() {
            return this->max_hold_time;
        }

        void set_max_hold_time(int max_hold_time) {
            //std::cout << "Setting max hold time of interface " << name << " with ip " << ip_address << " to " << max_hold_time << std::endl;
            this->max_hold_time = max_hold_time;
        }

        void increment_hold_time() {
            //std::cout << "this pointer: " << this << std::endl;
            this->current_hold_time++;
            //std::cout << " interface " << this->name << " with ip " << this->ip_address << " Max Hold time " << this->max_hold_time << " current hold time " << this->current_hold_time << std::endl;

            if(this->current_hold_time > this->max_hold_time) {
                //TODO send a notfication msg
                Time rightNow = Simulator::Now();
                std::cout << "Hold time of interface with ip " << this->ip_address << " expired at time " <<  rightNow.GetSeconds() << std::endl;
            } else {
                std::cout << "Current hold time of interface " << this->name << " with ip " << this->ip_address << " incremented: value = " << this->current_hold_time << std::endl;
                Simulator::Schedule (Seconds(1.0), &Interface::increment_hold_time, this);
            } 
        }

        int get_current_hold_time() {
            return this->current_hold_time;
        }

        void set_current_hold_time(int current_hold_time) {
            this->current_hold_time = current_hold_time;
        }
    };

    struct NLRIs {
        uint8_t prefix_lenght;
        std::string prefix;
    };

    struct Path_atrs {
        int type;
        int lenght;
        std::string value;
        uint8_t optional;
        uint8_t transitive;
        uint8_t partial;
        uint8_t extended_lenght;
    };

    struct Route {
        NLRIs nlri;
        std::vector<Path_atrs> path_atr;
    };


    class Router : public NodeContainer{
        public:
            Router(std::string router_ID, int AS, std::vector<Interface> interfaces, std::vector<int> neighbours, std::vector<Peer> routing_table, NodeContainer node, Ipv4Address ASip, Ipv4Mask ASmask);
            Router(int AS, NodeContainer node, Ipv4Address ASip, Ipv4Mask ASmask);

            std::string get_router_ID();
            int get_router_AS();
            float get_router_trust();
            std::vector<Interface> get_router_int();
            std::vector<int> get_router_neigh();
            std::vector<Peer> get_router_rt();
            void push_new_route(Peer new_route);
            void update_routing_table(std::string network, std::vector<Path_atrs> atribs);
            NodeContainer get_router_node();
            Ipv4Address get_router_ASip();
            Ipv4Mask get_router_ASmask();
            void add_interface(Interface interface);
            void add_neighbour(int neighbour);
            Interface get_router_int_from_name(int num);
            int get_router_int_num_from_name(int num);
            void setServer(int int_num, Ptr<BGPServer> server);
            void setClient(int int_num, Ptr<BGPClient> client);
            int get_router_int_num_from_ip(Ipv4Address ip);

        private:
            std::string router_ID;
            int AS;
            std::vector<Interface> interfaces;
            std::vector<int> neighbours;
            std::vector<Peer> routing_table;
            //NodeContainer node;
            Ipv4Address ASip;
            Ipv4Mask ASmask;
    };

} // namespace ns3

#endif