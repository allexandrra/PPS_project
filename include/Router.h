#ifndef NS3_ROUTER_H
#define NS3_ROUTER_H

#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <ctime> 

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
#include "ns3/random-variable-stream.h"

#include "../include/BGP-server.h"
#include "../include/BGP-client.h"

using namespace ns3;
using namespace std;

namespace ns3 {

    /**
     * @brief Struct for storing information about the peers of a router
    */
    struct Peer {
        std::string network;
        std::string mask;
        std::string next_hop;
        std::string path;
        int weight;
        int loc_pref;
        int AS_path_len;
        int MED;
        float trust;
    };


    /**
    * @brief Struct for storing information about the interfaces of a router
    */
    struct Interface {
        /**
        * @brief Name of the interface, starting with eth (e.g. eth0)
        */
        std::string name;

        /**
         * @brief local IP address of the interface
        */
        Ipv4Address ip_address;

        /**
         * @brief local mask of the interface
        */
        Ipv4Address mask;

        /**
         * @brief Status of the interface (true if the interface is up, false otherwise)
        */
        bool status;

        /**
         * @brief Pointer to the client application (initilizad only if the interface is a client - implemented through the use of the optional construct)
        */
        std::optional<Ptr<BGPClient>> client;

        /**
         * @brief Pointer to the server application (initilizad only if the interface is a server - implemented through the use of the optional construct)
        */
        std::optional<Ptr<BGPServer>> server;

        /**
         * @brief Boolean indicating if the interface is a server (true) or a client (false)
        */
        bool isServer;

        /**
         * @brief Hold time defined for the interface during the opening phase of the BGP session
        */
        int max_hold_time;

        /**
         * @brief Time at which the hold time was refreshed the last time
        */
        double last_update_time;


        /**
         * @brief Inherited trust value discovered from the trust value of the interface's neighbour
        */
        float inherited_trust;

        /**
         * @brief Direct trust value composed by inherited trust discovered from the trust value of the interface's neighbour
         * and the observed trust value of the interface
        */
        float direct_trust;

        /**
         * @brief Trust value obtained from the neighbour of the peers
        */
        float voted_trust;

        /**
         * @brief Total trust value of the interface, composed by the direct trust value and the voted trust value
        */
       float total_trust;

        /**
         * @brief Constructor for the Interface struct
        */
        Interface(std::string name, Ipv4Address ip_address, Ipv4Address mask) : status(true), isServer(false) {
            this->name = name;
            this->ip_address = ip_address;
            this->mask = mask;
            this->max_hold_time = 0;
            this->last_update_time = 0;
            this->direct_trust = 0;
            this->voted_trust = 0;
            this->total_trust = 0;

            // randomly generate the inherited trust value between 0 and 1
            this->inherited_trust = (float) rand() / (float) RAND_MAX;
        }

        /**
         * @brief Getters and setters for the Interface hold time management 
        */
        int get_max_hold_time() {
            return this->max_hold_time;
        }

        void set_max_hold_time(int max_hold_time) {
            this->max_hold_time = max_hold_time;
        }

        double get_last_update_time() {
            return this->last_update_time;
        }

        void set_last_update_time(double last_update_time) {
            this->last_update_time = last_update_time;
        }
    };

    /**
     * @brief Struct for storing information about the NLRIs of a router
    */
    struct NLRIs {
        uint8_t prefix_lenght;
        std::string prefix;
    };

    /**
     * @brief Struct for storing information about the path attributes of a router
    */
    struct Path_atrs {
        int type;
        int lenght;
        std::string value;
        uint8_t optional;
        uint8_t transitive;
        uint8_t partial;
        uint8_t extended_lenght;
    };

    /**
     * @brief Struct for storing information about the routes of a router
    */
    struct Route {
        NLRIs nlri;
        std::vector<Path_atrs> path_atr;
    };


    /**
     * @brief Class that simulate the behaviour of a router. 
     * It is a subclass of the NS3 NodeContainer class in order to have access to all the commmunication features of the NS3 Node class
    */
    class Router : public NodeContainer{
        public:

            /**
             * @brief Constructors for the Router class
            */
            Router(std::string router_ID, int AS, std::vector<Interface> interfaces, std::vector<int> neighbours, std::vector<Peer> routing_table, NodeContainer node, Ipv4Address AS_ip, Ipv4Mask AS_mask);
            Router(int AS, NodeContainer node, Ipv4Address AS_ip, Ipv4Mask AS_mask);

            /**
             * @brief Standard getters for the Router class attributes
            */
            std::string get_router_ID();
            int get_router_AS();
            float get_router_trust();
            std::vector<Interface> get_router_int();
            std::vector<int> get_router_neigh();
            std::vector<Peer> get_router_rt();
            NodeContainer get_router_node();
            Ipv4Address get_router_ASip();
            Ipv4Mask get_router_ASmask();
            void apply_policy(Router router, Route update_route);
            
            std::vector<NLRIs> remove_route(std::vector<NLRIs> withdrawnRoutes);

            /**
             * @brief Method for adding a new interface to the router Infaces vector
             * @param interface Interface struct instance to be added
            */
            void add_interface(Interface interface);

            /**
             * @brief Method for adding a new neighbour to the router Neighbours vector
             * @param neighbour AS number of the neighbour router to be added
            */
            void add_neighbour(int neighbour);
            
            /**
             * @brief Methods for updating the routing table of the router
            */
            void push_new_route(Peer new_route);
            void update_routing_table(std::string network, std::vector<Path_atrs> atribs);

            /**
             * @brief Method for getting an interface struct instance from the interface name 
             * @param num name of the interface (the name is composed by the number passed as parameter and the prefix string "eth")
             * @return Interface struct instance
            */
            Interface get_router_int_from_name(int num);

            /**
             * @brief Method for getting the interface index inside the vector of Interfaces of the router from the interface name
             * @param num mumber of the interface (the name is composed by the number passed as parameter and the prefix string "eth")
             * @return Interface index inside the vector of Interfaces of the router
             */            
            int get_router_int_num_from_name(int num);

            /**
             * @brief Method for getting the interface index inside the vector of Interfaces of the router from the interface IP address
             * @param ip IP address of the interface
             * @return Interface index inside the vector of Interfaces of the router
            */
            int get_router_int_num_from_ip(Ipv4Address ip);

            /**
             * @brief Method for getting the interface trust value from the interface name
             * @param num name of the interface (the name is composed by the number passed as parameter and the prefix string "eth")
             * @return Interface trust value
            */
            float get_trust_from_interface_name(int num);

            /**
             * @brief Methods for setting the server and client applications of a single router interface
             * @param int_num number of the interface in which the server or client application is to be set
             * @param server pointer to the new server application
             * @param client pointer to the new client application
            */
            void set_server(int int_num, Ptr<BGPServer> server);
            void set_client(int int_num, Ptr<BGPClient> client);

            /**
             * @brief Methods for resetting the server and client applications of a single router interface
             * @param int_num number of the interface in which the server or client application is to be reset
            */
            void reset_server(int int_num);
            void reset_client(int int_num);

            /**
             * @brief Method for setting the status of a single router interface
             * @param int_num number of the interface in which the status is to be set
             * @param status boolean indicating the new status of the interface
            */
            void set_interface_status(int int_num, bool status);

            /**
             * @brief Method for updating and setting of new router interface
             * @param interface new interface struct instance
             * @param num index of the interface to be updated
             * 
            */
            void set_interface(Interface interface, int num);

            /**
             * @brief Method for updating and setting of new router neighbours vector
             * @param neighbours new vector of neighbours AS numbers
            */
            void set_router_neigh(std::vector<int> neighbours);

        private:

            /**
             * @brief BGP router ID (IPv4 address format for the value)
            */
            std::string router_ID;

            /**
             * @brief BGP Autonomous System number (missing the "AS" prefix in the value, only the numeric part added)
            */
            int AS;

            /**
             * @brief Vector of the actual interfaces of the router
            */
            std::vector<Interface> interfaces;

            /**
             * @brief Vector of the AS numbers of the neighbours of the router
            */
            std::vector<int> neighbours;

            /**
             * @brief Vector of the peers of the router which rapresent the final BGP routing table of the router
            */
            std::vector<Peer> routing_table;

            /**
             * @brief Ip address associated to the AS, which is propageted to the peers
            */
            Ipv4Address AS_ip;

            /**
             * @brief Ip mask associated to the AS, which is propageted to the peers
            */
            Ipv4Mask AS_mask;
    };

} // namespace ns3

#endif