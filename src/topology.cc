/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Network topology: described in the configuration file configuration_file.json (other examples in the folder testConfig)
// To run the simulation: ./ns3 run scratch/PPS_project/src/topology.cc
// To view the logs: export NS_LOG=Topology=info

#include "../include/BGP-client.h"
#include "../include/BGP-server.h"
#include "../include/MessageHeader.h"
#include "../include/MessageNotification.h"
#include "../include/MessageUpdate.h"
#include "../include/MessageOpen.h"
#include "../include/MessageTrustrate.h"
#include "../include/configuration_parser.h"
#include "ns3/core-module.h"

using namespace ns3;

// Define the main loggin component of the simulator
NS_LOG_COMPONENT_DEFINE("Topology");

/**
 * @brief Create the stream for the open message starting from the value of router
 * @param r the router that include the parameters for the open message
 * @return the stream containing the open message
*/
std::stringstream createOpenMsg(Router r) {
    std::stringstream msgStream;
    MessageOpen msg = MessageOpen(r.get_router_AS(), 0, r.get_router_ID());
    msgStream << msg << '\0';
    return msgStream;
}


/**
 * @brief Every client send the open message to the server where it is connected
 * The server will reply with an open message
 * @param routers the vector containing all the routers of the topology
*/
void sendOpenMsg(std::vector<Router> routers) {
    // iterate over all the routers
    for(int i=0; i<(int)routers.size(); i++) {
        // get the interfaces of the router and iterate over them
        std::vector<Interface> interfaces = routers[i].get_router_int();        
        for (int j=0; j<(int)interfaces.size(); j++) {
            // check if the interface is a client
            if(!interfaces[j].isServer && interfaces[j].client.has_value()) {
                // check if the interface is up
                if(interfaces[j].status) {
                    //send msg
                    std::stringstream msgStream = createOpenMsg(routers[i]);
                    interfaces[j].client.value()->AddPacketsToQueue(msgStream, Seconds(0.0));
                } else {
                    // send notification message becase the interface is down
                    NS_LOG_INFO("Interface " << interfaces[j].name << " of router " << routers[i].get_router_AS() << " is down [sendOpenMsg]");

                    std::stringstream msgStream;
                    MessageNotification msg = MessageNotification(6,0);
                    msgStream << msg << '\0';

                    Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
                    interfaces[j].client.value()->Send(interfaces[j].client.value()->get_socket(), packet);
                    
                    // reset the client and the server and set the router interface
                    interfaces[j].client.reset();
                    interfaces[j].server.reset();
                    routers[i].set_interface(interfaces[j], j);
                }
            }
        }
    }
}

/**
 * @brief Method to test if the nth bit of a number is set
*/
inline bool isSetBit(int n, int index){
	return n & (1 << index);
}

/**
 * @brief Method to count the number of set bits in a number
 * @param n Number whose set bits are to be counted
 * @return Number of set bits in n
*/
int popcount1(int n){
	int count = 0;
	int digits = static_cast<int>(std::floor(std::log2(n))) + 1;
	for(int i = 0 ; i < digits ; ++i){
		if(isSetBit(n,i)) ++count;
	}
	return count;
}


/**
 * @brief Create the p2p links between the routers of the topology to start share information
 * @param routers the vector containing all the routers of the topology
 * @return the vector containing all the routers of the topology with the p2p links
*/
std::vector<Router> createLinks(std::vector<Router> routers) {
    // iterate over all the routers
    int base = 0;
    for(int i=0; i<(int)routers.size(); i++) {
        std::vector<int> neighbours = routers[i].get_router_neigh();

        std::string debug;
        for(int i=0; i<(int)neighbours.size(); i++) {
            debug += std::to_string(neighbours[i]) + " ";
        }
        std::cout << "Router "  << routers[i].get_router_AS() << " - Neighbours: " << debug << std::endl;

        // iterate over all the neighbours of the router
        for(int j=0; j<(int)neighbours.size(); j++) {

            int neighbourIndex = 0;
            // find the index of the neighbour in the vector of routers with a specified AS
            while(routers[neighbourIndex].get_router_AS() != neighbours[j]) {
                neighbourIndex++;
            };

            // create a node container with the two routers that are peers
            NodeContainer nc = NodeContainer(routers[i].get_router_node().Get(0), routers[neighbourIndex].get_router_node().Get(0));

            // create the ns-3 p2p link 
            PointToPointHelper p2p;
            p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
            p2p.SetChannelAttribute("Delay", StringValue("1ms"));
            p2p.SetDeviceAttribute("Mtu", UintegerValue (5000));

            // install the p2p link between the two routers
            NetDeviceContainer ndc = p2p.Install(nc);

            // create a subnet for the p2p link (incremenally, /30 subnet used)
            Ipv4AddressHelper ipv4;
            const char * ip  = ("160.0.0." + std::to_string(base)).c_str();
            base += 4;
            ipv4.SetBase(ip, "255.255.255.252");

            // create the ns3 interface container and assign the subnet to the p2p link
            Ipv4InterfaceContainer iic = ipv4.Assign(ndc);

            // create the interfaces of the two routers with the interfaces in the container
            Interface if1 = Interface("eth"+std::to_string(neighbours[j]), iic.GetAddress(0), /*iic.GetAddress(1),*/ Ipv4Address("255.255.255.252"));
            Interface if2 = Interface("eth"+std::to_string(routers[i].get_router_AS()), iic.GetAddress(1), /*iic.GetAddress(0),*/ Ipv4Address("255.255.255.252"));

            // subfunction to check if the interface is already there for the router
            struct find_interface : std::unary_function<Interface, bool> {
                std::string name;
                find_interface(std::string name):name(name) { }
                bool operator()(Interface const& i) const {
                    return i.name == name;
                }
            };

            // add the interface to the router if it is not already there
            std::vector<Interface> ifacesI = routers[i].get_router_int();
            if((std::find_if(ifacesI.begin(), ifacesI.end(),find_interface(if1.name)) == ifacesI.end()) && (std::find_if(ifacesI.begin(), ifacesI.end(),find_interface(if2.name)) == ifacesI.end())) {
                //NS_LOG_INFO("Adding interface " << if1.name << " with " << if1.ip_address << " to router " << routers[i].get_router_AS());
                routers[i].add_interface(if1);

                std::stringstream neigh_IP;
                std::stringstream int1_IP;
                ns3::Ipv4Address nip_address = routers[neighbourIndex].get_router_ASip();
                nip_address.Print(neigh_IP);
                ns3::Ipv4Address i1IP = if1.ip_address;
                i1IP.Print(int1_IP);
                routers[i].set_next_hop(neigh_IP.str(), int1_IP.str(), routers[i].make_string_from_IP(if2.ip_address));
            }
            
            // adding the interface to the neighbour router if it is not already there
            std::vector<Interface> ifacesJ = routers[neighbourIndex].get_router_int();
            if((std::find_if(ifacesJ.begin(), ifacesJ.end(),find_interface(if1.name)) == ifacesJ.end()) && (std::find_if(ifacesJ.begin(), ifacesJ.end(),find_interface(if2.name)) == ifacesJ.end())) {
                //NS_LOG_INFO("Adding remote interface " << if2.name << " to router " << routers[neighbourIndex].get_router_AS());
                routers[neighbourIndex].add_interface(if2);

                std::stringstream int2_IP;
                std::stringstream router_IP;
                ns3::Ipv4Address rip_address = routers[i].get_router_ASip();
                rip_address.Print(router_IP);
                ns3::Ipv4Address i2IP = if2.ip_address;
                i2IP.Print(int2_IP);
                routers[neighbourIndex].set_next_hop(router_IP.str(), int2_IP.str(), routers[neighbourIndex].make_string_from_IP(if1.ip_address));
            }
        }
    }
    
    return routers;
}


/**
 * @brief Function to print the topology of the network in the cli
 * @param routers Vector of routers in the network
*/
void printTopology (std::vector<Router> routers) {
    // Iterate over all the routers in the network and print their interfaces with ip addresses
    for(int i=0; i<(int)routers.size(); i++) {
        std::string s = "";
        for(int j=0; j<(int)routers[i].get_router_int().size(); j++) {
            std::stringstream ip;
            s += routers[i].get_router_int()[j].name; 
            s += " (";
            routers[i].get_router_int()[j].ip_address.Print(ip);
            s += ip.str();
            s += ") ";
        }
        NS_LOG_INFO("Router " << routers[i].get_router_AS() << " has " << routers[i].get_router_int().size() << " interfaces: " << s);
    }
}

/**
 * @brief Function to create the BGP connections based on the ns3 tcp sockets between the routers
 * @param routers Vector of routers in the network
 * @return Vector of routers in the network with the BGP connections established
*/
std::vector<Router> createBGPConnections(std::vector<Router> routers) {
    // server is listening on port 179 (default for BGP) for incoming connections
    int serverPort = 179;
    // both the client and servers have really long stop times to avoid the connection to be closed
    Time startClient = Seconds(1.);
    Time stopClient = Seconds(300000000000.0);
    Time startServer = Seconds(0.);
    Time stopServer = Seconds(300000000000.0);

    // Iterate over all the routers in the network
    for(int i=0; i< (int)routers.size(); i++) {
        std::vector<int> neighbours = routers[i].get_router_neigh();

        // Iterate over all the neighbours of the router
        for(int j=0; j<(int)neighbours.size(); j++) {
            
            int neighbourIndex = 0;
            // find the index of the neighbour in the vector of routers with a specified AS
            while(routers[neighbourIndex].get_router_AS() != neighbours[j]) {
                neighbourIndex++;
            };

            int neighbourInterfaceIndex = routers[i].get_router_int_num_from_name(neighbours[j]);

            // condition to check if the router is a client or a server on the interface and the status of the peer accordingly
            if(!(routers[i].get_router_int_from_name(neighbours[j])).client || !(routers[neighbourIndex].get_router_int_from_name(routers[i].get_router_AS())).server) {

                std::cout << "Router " << routers[i].get_router_AS() << " is server on interface eth" << neighbours[j] << " and router " << neighbours[j] << " is client on interface eth" << routers[i].get_router_AS() << std::endl;
                
                // Create the server on the router
                Address receiverAddress(InetSocketAddress(routers[i].get_router_int_from_name(neighbours[j]).ip_address, serverPort));
                Ptr<BGPServer> serverApp = CreateObject<BGPServer>();
                serverApp->Setup(serverPort, startClient);

                // Install and load the server 
                routers[i].get_router_node().Get(0)->AddApplication(serverApp);
                serverApp->set_router(&routers[i]);
                serverApp->SetStartTime(startServer);
                serverApp->SetStopTime(stopServer);

                // Set the server on the router
                routers[i].set_server(neighbourInterfaceIndex, serverApp);

                // Create the client that connects to the server created
                Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(routers[neighbourIndex].get_router_node().Get(0), TcpSocketFactory::GetTypeId());
                ns3TcpSocket->SetAttribute("SegmentSize", UintegerValue(1448));
                Ptr<BGPClient> clientApp = CreateObject<BGPClient>();
                clientApp->Setup(ns3TcpSocket, receiverAddress, startClient, stopClient);

                // Install and load the client on the neighbour router
                routers[neighbourIndex].get_router_node().Get(0)->AddApplication(clientApp);
                clientApp->set_router(&routers[neighbourIndex]);
                clientApp->SetStartTime(startClient);
                clientApp->SetStopTime(stopClient);

                // Set the client on the neighbour router
                int routerInterfaceIndex = routers[neighbourIndex].get_router_int_num_from_name(routers[i].get_router_AS());
                routers[neighbourIndex].set_client(routerInterfaceIndex, clientApp);
            }
        
        } 
    }
    return routers;
}

/**
 * @brief Function to disable a single link between two routers (test the error handling)
 * @param routers Vector of routers in the network
 * @param AS1 AS number of the first router
 * @param AS2 AS number of the second router
*/
void disable_router_link(std::vector<Router>* routers, int AS1, int AS2) {
    std::vector<NLRIs> wr1, wr2;
    std::vector<NLRIs> remove_router1, remove_router2;

    // Iterate over all the routers in the network
    for(int i=0; i<(int)routers->size(); i++) {
        // check if the current router has the AS number equal to AS1
        if ((*routers)[i].get_router_AS() == AS1) {

            // get the interface index of the router with the AS number equal to AS2
            int interfaceIndex = (*routers)[i].get_router_int_num_from_name(AS2);
            NS_LOG_INFO("Disabling interface " << interfaceIndex << " on router " << (*routers)[i].get_router_AS());

            // set the interface status to false and reset the client and server on the interface
            (*routers)[i].set_interface_status(interfaceIndex, false);
            (*routers)[i].reset_client(interfaceIndex);
            (*routers)[i].reset_server(interfaceIndex);
            
            std::vector<int> neighbours = (*routers)[i].get_router_neigh();

            // remove the router from the neighbor list
            auto it = std::find(neighbours.begin(), neighbours.end(), AS2);
            if (it != (*routers)[i].get_router_neigh().end()) {
                neighbours.erase(it);
            }
            (*routers)[i].set_router_neigh(neighbours);

            // start the update process of BGP
            for(int j = 0; j < (int)routers->size(); j++) {
                if ((*routers)[j].get_router_AS() == AS2) {
                    std::stringstream tmp1;
                    std::stringstream tmp2;
                    NLRIs remove_route;

                    // get ip address and mask of the router
                    ns3::Ipv4Address ip_address = (*routers)[j].get_router_ASip();
                    ip_address.Print(tmp1);
                    remove_route.prefix = tmp1.str();
                        
                    ns3::Ipv4Mask ip_mask = (*routers)[j].get_router_ASmask();
                    ip_mask.Print(tmp2);

                    std::string mask = tmp2.str();
                    std::string token;
                    int len = 0;
                    int pos = mask.find(".");

                    // remove the routes from the router
                    while ((pos = mask.find(".")) != (int) std::string::npos) {
                        token = mask.substr(0,pos);
                        if (stoi(token) == 255) 
                            len += 8;
                        else {
                            len += popcount1(stoi(token));
                        }

                        mask.erase(0, pos+1);
                    }
                    remove_route.prefix_lenght = len;

                    wr1.push_back(remove_route);
                    remove_router1.push_back(remove_route);

                    std::string intf;

                    for (int o = 0; o < (int) (*routers)[i].get_router_rt().size(); o++) {
                        if ((*routers)[i].get_router_rt()[o].network == remove_route.prefix) {
                            intf = (*routers)[i].get_router_rt()[o].int_ip;
                            break;
                        }
                    }
                    for (int k = 0; k < (int) (*routers)[i].get_router_rt().size(); k++) {
                        if((*routers)[i].get_router_rt()[k].int_ip == intf && (*routers)[i].get_router_rt()[k].network != remove_route.prefix) {
                            NLRIs rmv_route;

                            rmv_route.prefix = (*routers)[i].get_router_rt()[k].network;
                            std::string mask = (*routers)[i].get_router_rt()[k].mask;
                            std::string token;
                            int len = 0;
                            int pos = mask.find(".");
                            while ((pos = mask.find(".")) != (int) std::string::npos) {
                                token = mask.substr(0,pos);
                                if (stoi(token) == 255) 
                                    len += 8;
                                else {
                                    len += popcount1(stoi(token));
                                }

                                mask.erase(0, pos+1);
                            }
                            rmv_route.prefix_lenght = len;

                            wr1.push_back(rmv_route);                        
                        }
                    }
                }
            }

            (*routers)[i].remove_route(remove_router1);

            for (int i = 0; i < (int)wr1.size(); i++) {
                std::cout << "Route to delete " << wr1[i].prefix << " " << unsigned(wr1[i].prefix_lenght) << std::endl;
            }
        }
        
        // check if the current router has the AS number equal to AS2
        if ((*routers)[i].get_router_AS() == AS2) {

            // repeat the same process as before

            int interfaceIndex = (*routers)[i].get_router_int_num_from_name(AS1);

            NS_LOG_INFO("Disabling interface " << interfaceIndex << " on router " << (*routers)[i].get_router_AS());

            (*routers)[i].set_interface_status(interfaceIndex, false);
            (*routers)[i].reset_client(interfaceIndex);
            (*routers)[i].reset_server(interfaceIndex);

            std::vector<int> neighbours = (*routers)[i].get_router_neigh();

            // remove the router from the neighbor list
            auto it = std::find(neighbours.begin(), neighbours.end(), AS1);
            if (it != (*routers)[i].get_router_neigh().end()) {
                neighbours.erase(it);
            }
            (*routers)[i].set_router_neigh(neighbours);

            for(int j = 0; j < (int)routers->size(); j++) {
                if ((*routers)[j].get_router_AS() == AS1) {
                    std::stringstream tmp1;
                    std::stringstream tmp2;
                    NLRIs remove_route;

                    ns3::Ipv4Address ip_address = (*routers)[j].get_router_ASip();
                    ip_address.Print(tmp1);
                    remove_route.prefix = tmp1.str();
                        
                    ns3::Ipv4Mask ip_mask = (*routers)[j].get_router_ASmask();
                    ip_mask.Print(tmp2);

                    std::string mask = tmp2.str();
                    std::string token;
                    int len = 0;
                    int pos = mask.find(".");
                    while ((pos = mask.find(".")) != (int) std::string::npos) {
                        token = mask.substr(0,pos);
                        if (stoi(token) == 255) 
                            len += 8;
                        else {
                            len += popcount1(stoi(token));
                        }

                        mask.erase(0, pos+1);
                    }
                    remove_route.prefix_lenght = len;

                    wr2.push_back(remove_route);
                    remove_router2.push_back(remove_route);

                    std::string intf;

                    for (int o = 0; o < (int) (*routers)[i].get_router_rt().size(); o++) {
                        if ((*routers)[i].get_router_rt()[o].network == remove_route.prefix) {
                            intf = (*routers)[i].get_router_rt()[o].int_ip;
                            break;
                        }
                    }
                    for (int k = 0; k < (int) (*routers)[i].get_router_rt().size(); k++) {
                        if((*routers)[i].get_router_rt()[k].int_ip == intf && (*routers)[i].get_router_rt()[k].network != remove_route.prefix) {
                            NLRIs rmv_route;

                            rmv_route.prefix = (*routers)[i].get_router_rt()[k].network;
                            std::string mask = (*routers)[i].get_router_rt()[k].mask;
                            std::string token;
                            int len = 0;
                            int pos = mask.find(".");
                            while ((pos = mask.find(".")) != (int) std::string::npos) {
                                token = mask.substr(0,pos);
                                if (stoi(token) == 255) 
                                    len += 8;
                                else {
                                    len += popcount1(stoi(token));
                                }

                                mask.erase(0, pos+1);
                            }
                            rmv_route.prefix_lenght = len;

                            wr2.push_back(rmv_route);                        
                        }
                    }
                }
            }

            (*routers)[i].remove_route(remove_router2);
            
            for (int i = 0; i < (int)wr2.size(); i++) {
                std::cout << "Route to delete " << wr2[i].prefix << " " << unsigned(wr2[i].prefix_lenght) << std::endl;
            }
        }
    }


    // send the update message to the neighbors of AS1 and AS2
    for(int i=0; i<(int)routers->size(); i++) {
        if ((*routers)[i].get_router_AS() == AS1) {    
            std::vector<Interface> interfaces = (*routers)[i].get_router_int();
            for (int j=0; j<(int)interfaces.size(); j++) {

                if(interfaces[j].status) {
                    //create msg
                    MessageUpdate msg = MessageUpdate(wr1.size(), wr1);
                    std::stringstream msgStream;
                    msgStream << msg << "\0";

                    //create packet
                    Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);

                    // send the packet
                    if(interfaces[j].client) { 
                        interfaces[j].client.value()->Send(interfaces[j].client.value()->get_socket(), packet);
                    }

                } else {
                    // send the notification message if the interface is down
                    NS_LOG_INFO("Interface " << interfaces[j].name << " of router " << (*routers)[i].get_router_AS() << " is down [sendOpenMsg]");
                    std::stringstream msgStream;
                    MessageNotification msg = MessageNotification(6,0);
                    msgStream << msg << '\0';
                    Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);

                    if(interfaces[j].client) { 
                        interfaces[j].client.value()->Send(interfaces[j].client.value()->get_socket(), packet);
                    }

                    interfaces[j].client.reset();
                    interfaces[j].server.reset();
                }
            }
        }
        if ((*routers)[i].get_router_AS() == AS2) {
            std::vector<Interface> interfaces = (*routers)[i].get_router_int();
            for (int j=0; j<(int)interfaces.size(); j++) {
                if(interfaces[j].status) {
                    
                    //create msg
                    MessageUpdate msg = MessageUpdate(wr2.size(), wr2);
                    std::stringstream msgStream;
                    msgStream << msg << "\0";

                    //create packet
                    Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
                    NS_LOG_INFO(msgStream.str());

                    // send the packet
                    if(interfaces[j].client) { 
                        interfaces[j].client.value()->Send(interfaces[j].client.value()->get_socket(), packet);
                    }

                } else {
                    // send the notification message if the interface is down
                    NS_LOG_INFO("Interface " << interfaces[j].name << " of router " << (*routers)[i].get_router_AS() << " is down [sendOpenMsg]");
                    std::stringstream msgStream;
                    MessageNotification msg = MessageNotification(6,0);
                    msgStream << msg << '\0';
                    Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
                    if(interfaces[j].client) { 
                        interfaces[j].client.value()->Send(interfaces[j].client.value()->get_socket(), packet);
                    }
                    interfaces[j].client.reset();
                    interfaces[j].server.reset();
                }
            }
        }
    }
}

/**
 * @brief Disable a complete router of the topology to simulate an error condition
 * @param routers vector of routers in the topology
 * @param AS autonomous system of the router to disable
*/
void disable_router(std::vector<Router>* routers, int AS) {
    bool found = false;
    
    // iterate all over the interface of the router and disable them using the disable_router_link function already defined
    for(int i=0; i<(int)routers->size(); i++) {

        if ((*routers)[i].get_router_AS() == AS) {

            found = true;

            //get the neighbours of the router
            std::vector<int> neighbours = (*routers)[i].get_router_neigh();

            for(int j=0; j<(int)neighbours.size(); j++) {
                disable_router_link(routers, AS, neighbours[j]);
            }  
        }

    }  

    // handle input errors
    if (!found)
        NS_LOG_INFO("Router with AS " << AS << " not found. Please insert a router with a valid AS.\n");   
}


/**
 * @brief Function to check if the router with a specific AS is present in the topology
 * @param routers vector of routers in the topology
 * @return true if the router is present, false otherwise
 */
bool checkAS(std::vector<Router>* routers, int AS) {
    bool found = false;
    
    for(int i=0; i<(int)routers->size(); i++) {
        if ((*routers)[i].get_router_AS() == AS)
            found = true;
    }  

    if (!found)
        NS_LOG_INFO("Router with AS " << AS << " not found. Please insert a router with a valid ASes.\n");   

    return found;
}


/**
 * @brief Function to simulate an external agent that periodically check the hold time of the interfaces of the routers 
 * and understand if they are expired or not (and in this case, send a notification message to the neighbors)
 * The max hold time is set to 3 times the keep alive frequency
 * @param routers vector of routers in the topology
*/
void checkHoldTime(std::vector<Router>* routers) {
    for(int i=0; i<(int)routers->size(); i++) {
        std::vector<Interface> interfaces = (*routers)[i].get_router_int();

        for(int j=0; j<(int)interfaces.size(); j++) {

            // check if the hold time is expired by comparing the current minus the last update time and the max hold time
            if(Simulator::Now().GetSeconds() - interfaces[j].get_last_update_time() > interfaces[j].get_max_hold_time()) {

                // if the interface is up, send a notification message to the neighbors as the hold time is expired
                // switch the status of the interface to down
                if (interfaces[j].status) {
                    NS_LOG_INFO("Interface " << interfaces[j].name << " of router " << (*routers)[i].get_router_AS() << " has expired hold time  at time " << Simulator::Now().GetSeconds() << " [PERIODIC HOLD TIME CHECK]");
            
                    // create the notification message
                    std::stringstream msgStreamNotification;
                    MessageNotification msg = MessageNotification(4,0);
                    msgStreamNotification << msg << '\0';

                    Ptr<Packet> packetNotification = Create<Packet>((uint8_t*) msgStreamNotification.str().c_str(), msgStreamNotification.str().length()+1);

                    // send the notification message to the peer on the interface
                    if(interfaces[j].client) {
                        interfaces[j].client.value()->Send(interfaces[j].client.value()->get_socket(), packetNotification);
                    } else if (interfaces[j].server){
                        interfaces[j].server.value()->Send(interfaces[j].server.value()->get_socket(), packetNotification);
                    }

                    // set the status of the interface to down
                    interfaces[j].client.reset();
                    interfaces[j].server.reset();

                    (*routers)[i].set_interface_status(j, false);
                    (*routers)[i].set_interface(interfaces[j], j);
                    //this->StopApplication();
                }
            }
        }
    } 

    // call the function again after 120 seconds
    // run the same agent again
    Simulator::Schedule(Seconds(60.0), &checkHoldTime, routers); 

}


/**
 * @brief Function to search the index of a router in the topology given its AS
 * @param routers vector of routers in the topology
 * @param AS autonomous system of the first router
 * @return the index of the router in the topology, -1 if not found
*/
int get_router_index_from_AS(std::vector<Router>* routers, int AS) {
    for(int i=0; i<(int)routers->size(); i++) {
        if ((*routers)[i].get_router_AS() == AS) {
            return i;
        }
    }
    return -1;
}


/**
 * @brief Agent based function to exchange the voted trust between the routers in the topology
 * The voted trust is exchanged only between the second grade neighbors (first grade neighbors of first grade neighbors)
 * The agent has the ability to access all the trust values of the routers in the topology and set values of voted trust and run periodically 
 * Concept based on the paper "HYBRID TRUST MODEL FOR INTERNET ROUTING" by Pekka Rantala, Seppo Virtanen and Jouni Isoaho"
 * @param routers vector of routers in the topology
*/
void exchangeVotedTrust(std::vector<Router>* routers) {
    std::cout << "\n\n--------------- START VOTING TRUST SHARING ------------\n" << std::endl;

    // iterate all over the routers in the topology
    for(int i=0; i<(int)routers->size(); i++) {
        std::vector<Interface> interfaces = (*routers)[i].get_router_int();
        std::cout << "--------------- ROUTER: " << (*routers)[i].get_router_AS() << "----------------" << std::endl;

        // iterate over the interfaces of the router
        for(int j=0; j<(int)interfaces.size(); j++) {
            // split the string eth0 and the number
            int neighborASNum = std::stoi(interfaces[j].name.substr(3,1));
            std::cout << "First grade neighbor " << neighborASNum << std::endl;
            
            // find the router index in the routers vector from an AS number
            int firstGradeNeighborIndex = get_router_index_from_AS(routers, neighborASNum);

            if(firstGradeNeighborIndex != -1) {
                // get the neighbor list of the peer router
                std::vector<int> peerNeighbours = (*routers)[firstGradeNeighborIndex].get_router_neigh();

                // iterate over the neighbors of the peer
                float mediumVotedTrust = 0;
                float ti = 0.25;
                float sumTi = 0.25 * peerNeighbours.size();
                for(int k=0; k<(int)peerNeighbours.size(); k++) {
                    // if they are equal the second grade neighbor is the router itself
                    if(peerNeighbours[k] != (*routers)[i].get_router_AS()) {
                        // get the index of the neights of the peer
                        int secondGradeNeighborIndex = get_router_index_from_AS(routers, peerNeighbours[k]);
                        float vi = (*routers)[secondGradeNeighborIndex].get_trust_from_interface_name(neighborASNum);
                        std::cout << "Second grade neighbor " <<  peerNeighbours[k] << " with trust: " << vi << " [eth"  << neighborASNum <<"]" <<  std::endl;

                        // compute the medium voted trust using the following formula:
                        // Vt = sum from 1 to n of (Vi *(Ti/ sum from 1 to n of Ti))
                        // where Vi is the individual vote of neighbor i and Ti is the weight factor of neighbor i
                        // Choices: Ti = 0.25 for every value of i (as they are all second grade neighbors)
                        mediumVotedTrust += vi * (ti/sumTi);
                    }
                }

                // update the voted trust of the interface
                interfaces[j].voted_trust = mediumVotedTrust;
                (*routers)[i].set_interface(interfaces[j], j);
               
            }
        }
    } 

     std::cout << "\n\n--------------- END VOTED TRUST SHARING ------------\n" << std::endl;

    // call the function again after 200 seconds
    Simulator::Schedule(Seconds(200.0), &exchangeVotedTrust, routers); 

}


/**
 * @brief Function used to print the input menu and execute the selected option
 * The input is syncronus and blocks the execution of the simulation
 * The menu is displayed every 60 seconds and the user can choose between different options
 * If the user selects the option 4 the simulation continues without being altered 
 * @param routers vector of routers in the topology
*/
void userInputCallback(std::vector<Router>* routers) {
    char c;
    
    // print the various options
    std::cout << "\n\nSelect one of the following to be executed at time " << (Simulator::Now().GetSeconds()) <<
        ": \n\n"
        "[1] Add a new policy\n"
        "[2] Disable a link \n"
        "[3] Disable a router \n"
        "[4] Continue the simulation \n"
        "[5] Print Routing tables \n"
        "[6] Quit\n\n"
        "Choose a number: ";

    // read the input
    std::cin >> c;
    std::cout << std::endl;

    if (isdigit(c)) {
        // execute the selected option
        switch (c) {
            case '1':
                {
                    std::string ip1, ip2;
                    int type, value, lenght;
                
                    // get input from the user to add a new local policy for routing
                    std::cout << "The policy contains the value modification of one preference for a given link. The options are 1 for weight, 2 for local preference, 5 for MED, 6 for trust. \n";
                    std::cout << "Enter the two routers that the policy applies to: ";
                    std::cin >> ip1 >> ip2 >> type >> value >> lenght;

                    NLRIs updated_nlri;
                    std::vector<Path_atrs> path_atr;
                    Path_atrs pa;
                    bool is_r1 = false;
                    bool is_r2 = false;
                    bool is_neigh = false;
                    int AS2 = 0;
                    std::vector<int> r1;

                    // check if the input is valid
                    if(type < 1 || type > 6) {
                        std::cout << "Wrong path attribute. It has to be a value between 1 and 5.\n\n";
                        break;
                    }

                    for(int i = 0; i < (int)routers->size(); i++) {
                        if((*routers)[i].make_string_from_IP((*routers)[i].get_router_ASip()) == ip1) {
                            is_r1 = true;
                            r1 = (*routers)[i].get_router_neigh();
                        }
                        if((*routers)[i].make_string_from_IP((*routers)[i].get_router_ASip()) == ip2) {
                            is_r2 = true;
                            AS2 = (*routers)[i].get_router_AS();
                        }
                    }

                    if(is_r1 == false || is_r2 == false) {
                        std::cout << "One of the IP addresses introduced does not exist in the topology. Please add valid IP addresses.\n\n";
                        break;
                    } 

                    for (int i = 0; i < (int)r1.size(); i++) {
                        if (r1[i] == AS2) {
                            is_neigh = true;
                            break;
                        }
                    }

                    if (is_neigh == false) {
                        std::cout << "The two routers are not connected by a link. Please add two neighbouring routers.\n\n";
                        break;
                    }

                    // create the new policy 
                    for(int i = 0; i < (int)routers->size(); i++) {
                        std::stringstream tmp1;
                        std::stringstream tmp2;

                        ns3::Ipv4Address ip_address = (*routers)[i].get_router_ASip();
                        ip_address.Print(tmp1);
                        if (tmp1.str() == ip2) {
                            updated_nlri.prefix = tmp1.str();
                            ns3::Ipv4Mask ip_mask = (*routers)[i].get_router_ASmask();
                            ip_mask.Print(tmp2);
                            
                            int len = 0;
                            size_t pos = 0;
                            std::string token;
                            std::string mask = tmp2.str();
                            std::cout << mask << "\n";
                            while ((pos = mask.find(".")) != std::string::npos) {
                                token = mask.substr(0,pos);
                                std::cout << token << "\n";
                                if (stoi(token) == 255) 
                                    len += 8;
                                else {
                                    len += popcount1(stoi(token));
                                }

                                mask.erase(0, pos+1);
                            }

                            std::cout << len << "\n";

                            updated_nlri.prefix_lenght = len;
                            break;
                        }
                    }

                    pa.type = type;
                    pa.value = to_string(value);
                    pa.lenght = lenght;
                    pa.optional = 0;
                    pa.transitive = 0;
                    pa.partial = 0;
                    pa.extended_lenght = 0;
                    path_atr.push_back(pa);
                    Route updated_route;
                    updated_route.nlri = updated_nlri;
                    updated_route.path_atr = path_atr;

                    // enforce the policy in the routing table of the router
                    for(int i = 0; i < (int)routers->size(); i++) {
                        std::stringstream tmp1;
                        ns3::Ipv4Address ip_address = (*routers)[i].get_router_ASip();
                        ip_address.Print(tmp1);
                        if (tmp1.str() == ip1) {
                            (*routers)[i].apply_policy(updated_route);
                            break;
                        }
                    }

                    break;
                }
            case '2':
                {
                    int AS1, AS2;

                    // get input from the user to understand which link to disable
                    std::cout << "Enter the two AS numbes of the routers that forms the link: ";
                    std::cin >> AS1 >> AS2;
                    std::cout << std::endl;

                    // check if the input is valid and disable the link if it is
                    if(checkAS(routers, AS1) && checkAS(routers, AS2))
                        disable_router_link(routers, AS1, AS2);
                    break;
                }
            case '3':
                {
                    int AS;

                    // get input from the user to understand which router to disable
                    std::cout << "Enter the AS number of the router to disable: ";
                    std::cin >> AS;
                    std::cout << std::endl;

                    // check if the input is valid and disable the router if it is
                    disable_router(routers, AS);
                    break;
                }
            case '4':
                // simply continue the simulation without doing anything and modifying the topology
                break;

            case '6':
                // exit the simulation
                exit(EXIT_SUCCESS);
                break;
            
            case '5':
                // print the routing tables of all the routers in the topology
                for(Router r : *routers) {
                    std::cout << "\n\n--------------- ROUTING TABLES ------------\n" << std::endl;
                    for(Router r : *routers) {
                        std::cout << "Router " << r.get_router_AS() << ": \n";
                        std::cout << "NET      MASK      WEI LEN   L.INT     N.HOP   P.LEN  PATH  MED  TRUST\n";
                        r.print_RT();
                        std::cout << "\n";
                    }
                    std::cout << "\n-------------------------------------------\n" << std::endl;
                    break;
                }
                break;

            default:
                // if the input is not valid, print an error message
                std::cout << "Invalid number, choose a number between 1 and 6.\n " << std::endl;
                break;
        }
    }
    else {
        // if the input is not valid, print an error message
        std::cout << "Invalid input, choose a number between 1 and 6.\n\n ";
    }

    // schedule the next user input callback after 60 seconds
    Simulator::Schedule(Seconds(60.0), &userInputCallback, routers);
}


/**
 * @brief Main function of the simulator, used as entry point of the entire program
*/
int main() {

    // Set ns3 parameters
    Time::SetResolution(Time::NS);
    Packet::EnablePrinting();

    // Enable logging components for ns3 classes
    LogComponentEnable("Router", LOG_LEVEL_INFO);
    LogComponentEnable("BGPServer", LOG_LEVEL_INFO);
    LogComponentEnable("BGPClient", LOG_LEVEL_INFO);
    LogComponentEnable("TCPCommon", LOG_LEVEL_INFO);

    // Starting the simulation

    // Create the nodes of the network
    NS_LOG_INFO("Loading topology");
    std::vector<Router> network = load_configuration();

    // Create links between the nodes
    NS_LOG_INFO("\nCreating links");
    network = createLinks(network);

    // initialize routing database of ns3
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    NS_LOG_INFO("\nPrinting topology");
    printTopology(network);

    NS_LOG_INFO("\nBGP state: IDLE");

    // Create BGP applications (client and server)
    NS_LOG_INFO("\nCreate BGP applications");
    network = createBGPConnections(network);

    NS_LOG_INFO("\nBGP state: CONNECT");

    // Start the flow of the messages (it continues in the handle read functions of both client and server)
    NS_LOG_INFO("\nSending open messages");
    sendOpenMsg(network);


    NS_LOG_INFO("\nBGP state: OPEN SENT");
    NS_LOG_INFO("BGP state: OPEN CONFIRM\n");

    NS_LOG_INFO("\nStart sending KEEPALIVE messages every 30 seconds");
    //startKeepAlive(network);

    NS_LOG_INFO("\nBGP state: ESTABLISHED\n");

    NS_LOG_INFO("\nSending update messages\n");

    // Schedule the first user input callback to run after the simulation starts (after 45 seconds)
    Simulator::Schedule(Seconds(45.0), &userInputCallback, &network);

    // Schedule the first checkHoldTime callback to run after the simulation starts (after 100 seconds)
    Simulator::Schedule(Seconds(100.0), &checkHoldTime, &network);

    // Schedule the first exchangeVotedTrust callback to run after the simulation starts (after 150 seconds)
    Simulator::Schedule(Seconds(150.0), &exchangeVotedTrust, &network);

    // Start the simulation
    Simulator::Stop(Seconds(300000000.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}