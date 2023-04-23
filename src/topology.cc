/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Network topology: described in the configuration file
// All links are point-to-point links
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

inline bool isSetBit(int n, int index){
	return n & (1 << index);
}

int popcount1(int n){
	int count = 0;
	int digits = static_cast<int>(std::floor(std::log2(n))) + 1;
	for(int i = 0 ; i < digits ; ++i){
		if(isSetBit(n,i)) ++count;
	}
	return count;
}

std::vector<Router> createLinks(std::vector<Router> routers) {
    int base = 0;
    for(int i=0; i<(int)routers.size(); i++) {
        std::vector<int> neighbours = routers[i].get_router_neigh();

        std::string debug;
        for(int i=0; i<(int)neighbours.size(); i++) {
            debug += std::to_string(neighbours[i]) + " ";
        }
        std::cout << "Router "  << routers[i].get_router_AS() << " - Neighbours: " << debug << std::endl;

        for(int j=0; j<(int)neighbours.size(); j++) {

            int neighbourIndex = 0;
            // find the index of the neighbour in the vector of routers with a specified AS
            while(routers[neighbourIndex].get_router_AS() != neighbours[j]) {
                neighbourIndex++;
            };

            NodeContainer nc = NodeContainer(routers[i].get_router_node().Get(0), routers[neighbourIndex].get_router_node().Get(0));

            PointToPointHelper p2p;
            p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
            p2p.SetChannelAttribute("Delay", StringValue("1ms"));
            p2p.SetDeviceAttribute("Mtu", UintegerValue (5000));

            NetDeviceContainer ndc = p2p.Install(nc);

            Ipv4AddressHelper ipv4;
            const char * ip  = ("160.0.0." + std::to_string(base)).c_str();
            base += 4;
            ipv4.SetBase(ip, "255.255.255.252");

            Ipv4InterfaceContainer iic = ipv4.Assign(ndc);

            Interface if1 = Interface("eth"+std::to_string(neighbours[j]), iic.GetAddress(0), /*iic.GetAddress(1),*/ Ipv4Address("255.255.255.252"));
            Interface if2 = Interface("eth"+std::to_string(routers[i].get_router_AS()), iic.GetAddress(1), /*iic.GetAddress(0),*/ Ipv4Address("255.255.255.252"));

            //function to check if the interface is already there
            struct find_interface : std::unary_function<Interface, bool> {
                std::string name;
                find_interface(std::string name):name(name) { }
                bool operator()(Interface const& i) const {
                    return i.name == name;
                }
            };

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

        //std::cout << "----------------------------------------------------------" << std::endl;
    }
    
    return routers;
}

void printTopology (std::vector<Router> routers) {
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


std::vector<Router> createBGPConnections(std::vector<Router> routers) {
    int serverPort = 179;
    Time startClient = Seconds(1.);
    Time stopClient = Seconds(300000000000.0);
    Time startServer = Seconds(0.);
    Time stopServer = Seconds(300000000000.0);

    for(int i=0; i< (int)routers.size(); i++) {
        std::vector<int> neighbours = routers[i].get_router_neigh();

        for(int j=0; j<(int)neighbours.size(); j++) {
            
            int neighbourIndex = 0;
            // find the index of the neighbour in the vector of routers with a specified AS
            while(routers[neighbourIndex].get_router_AS() != neighbours[j]) {
                neighbourIndex++;
            };

            int neighbourInterfaceIndex = routers[i].get_router_int_num_from_name(neighbours[j]);

            // To control:
            // se è client e il vicino è server non devo fare nulla 
            // se non è client o il vicino non è server devo installare il client e il server

            if(!(routers[i].get_router_int_from_name(neighbours[j])).client || !(routers[neighbourIndex].get_router_int_from_name(routers[i].get_router_AS())).server) {

                std::cout << "Router " << routers[i].get_router_AS() << " is server on interface eth" << neighbours[j] << " and router " << neighbours[j] << " is client on interface eth" << routers[i].get_router_AS() << std::endl;
                
                Address receiverAddress(InetSocketAddress(routers[i].get_router_int_from_name(neighbours[j]).ip_address, serverPort));
                Ptr<BGPServer> serverApp = CreateObject<BGPServer>();
                serverApp->Setup(serverPort, startClient);

                // Install and load the server 
                routers[i].get_router_node().Get(0)->AddApplication(serverApp);
                serverApp->set_router(&routers[i]);
                serverApp->SetStartTime(startServer);
                serverApp->SetStopTime(stopServer);

                routers[i].set_server(neighbourInterfaceIndex, serverApp);

                // Tcp SENDER -> now on the as2
                Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(routers[neighbourIndex].get_router_node().Get(0), TcpSocketFactory::GetTypeId());
                ns3TcpSocket->SetAttribute("SegmentSize", UintegerValue(1448));
                Ptr<BGPClient> clientApp = CreateObject<BGPClient>();
                clientApp->Setup(ns3TcpSocket, receiverAddress, startClient, stopClient);

                // Install and load the client on as2
                routers[neighbourIndex].get_router_node().Get(0)->AddApplication(clientApp);
                clientApp->set_router(&routers[neighbourIndex]);
                clientApp->SetStartTime(startClient);
                clientApp->SetStopTime(stopClient);

                int routerInterfaceIndex = routers[neighbourIndex].get_router_int_num_from_name(routers[i].get_router_AS());
                routers[neighbourIndex].set_client(routerInterfaceIndex, clientApp);
            }
        
        } 
    }
    return routers;
}

int send_ip_message() {
    return 0;
}

void disable_router_link(std::vector<Router>* routers, int AS1, int AS2) {
    //Step:
    // Modificare il send in modo che controlli che il link sia attivo prima di mandare il messaggio 
    //    -> ho modificato il codice con il controllo prima di ogni addTOQueue e send nelle risposte
    // Leggere in input il numeero degli AS che formano il link da disabilitare

    // Disabilitare il link
    // Se il link è disabilitato mandare un notification msg e chiuedere la connessione
    // Se il link è disabilitato non mandare il keepalive msg (droppare client e)
    std::vector<NLRIs> wr1, wr2;

    for(int i=0; i<(int)routers->size(); i++) {
        if ((*routers)[i].get_router_AS() == AS1) {

            int interfaceIndex = (*routers)[i].get_router_int_num_from_name(AS2);
            NS_LOG_INFO("Disabling interface " << interfaceIndex << " on router " << (*routers)[i].get_router_AS());

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

            for(int j = 0; j < (int)routers->size(); j++) {
                if ((*routers)[j].get_router_AS() == AS2) {
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

                    wr1.push_back(remove_route);
                }

                

                // for (int j = 0; j < (*routers)[i].get_router_rt().size(); j++) {
                //     if((*routers)[i].get_router_rt()[j].next_hop == 
                //         (*routers)[i].make_string_from_IP((*routers)[i].get_router_int_from_name(AS1).ip_address)) {
                //                 std::cout << (*routers)[i].get_router_rt()[j].next_hop << " " << (*routers)[i].make_string_from_IP((*routers)[i].get_router_int_from_name(AS1).ip_address) << std::endl;
                //         }
                // }
            }

            (*routers)[i].remove_route(wr1);

            // for (int i = 0; i < (int)wr1.size(); i++) {
            //     std::cout << "Rute de eliminat " << wr1[i].prefix << " " << wr1[i].prefix_lenght << std::endl;
            // }
        }
        
        if ((*routers)[i].get_router_AS() == AS2) {
            //std::vector<NLRIs> wr;

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
                }

                // for (int j = 0; j < (*routers)[i].get_router_rt().size(); j++) {
                //     if((*routers)[i].get_router_rt()[j].next_hop == 
                //         (*routers)[i].make_string_from_IP((*routers)[i].get_router_int_from_name(AS1).ip_address)) {
                //                 std::cout << (*routers)[i].get_router_rt()[j].next_hop << " " << (*routers)[i].make_string_from_IP((*routers)[i].get_router_int_from_name(AS1).ip_address) << std::endl;
                //         }
                // }
            }

            (*routers)[i].remove_route(wr2);
            
            //for (int i = 0; i < (int)wr2.size(); i++) {
            //    std::cout << "Rute de eliminat " << wr2[i].prefix << " " << wr2[i].prefix_lenght << std::endl;
            //}
        }
    }

    for(int i=0; i<(int)routers->size(); i++) {
        if ((*routers)[i].get_router_AS() == AS1) {    
            std::vector<Interface> interfaces = (*routers)[i].get_router_int();
            for (int j=0; j<(int)interfaces.size(); j++) {
                //std::cout << interfaces[j].name << std::endl;
                if(interfaces[j].status) {
                    //send msg
                    MessageUpdate msg = MessageUpdate(wr1.size(), wr1);
                    std::stringstream msgStream;
                    msgStream << msg << "\0";
                    //std::cout << msgStream.str() << std::endl;

                    Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);

                    if(interfaces[j].client) { 
                        interfaces[j].client.value()->Send(interfaces[j].client.value()->get_socket(), packet);
                    }
                } else {
                    NS_LOG_INFO("Interface " << interfaces[j].name << " of router " << (*routers)[i].get_router_AS() << " is down [sendOpenMsg]");
                    std::stringstream msgStream;
                    MessageNotification msg = MessageNotification(6,0);
                    msgStream << msg << '\0';
                    Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
                    //Simulator::Schedule (Simulator::Now(), &BGPClient::Send, this, m_socket, packet);
                    //interfaces[j].client.value()->AddPacketsToQueue(msgStream, Seconds(0.0));

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
                    //send msg
                    MessageUpdate msg = MessageUpdate(wr2.size(), wr2);
                    std::stringstream msgStream;
                    msgStream << msg << "\0";

                    Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);

                    if(interfaces[j].client) { 
                        interfaces[j].client.value()->Send(interfaces[j].client.value()->get_socket(), packet);
                    }
                } else {
                    NS_LOG_INFO("Interface " << interfaces[j].name << " of router " << (*routers)[i].get_router_AS() << " is down [sendOpenMsg]");
                    std::stringstream msgStream;
                    MessageNotification msg = MessageNotification(6,0);
                    msgStream << msg << '\0';
                    Ptr<Packet> packet = Create<Packet>((uint8_t*) msgStream.str().c_str(), msgStream.str().length()+1);
                    if(interfaces[j].client) { 
                        interfaces[j].client.value()->Send(interfaces[j].client.value()->get_socket(), packet);
                    }
                    //Simulator::Schedule (Simulator::Now(), &BGPClient::Send, this, m_socket, packet);
                    //interfaces[j].client.value()->AddPacketsToQueue(msgStream, Seconds(0.0));
                    interfaces[j].client.reset();
                    interfaces[j].server.reset();
                }
            }
        }
    }
}

void disable_router(std::vector<Router>* routers, int AS) {
    bool found = false;
    
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

    if (!found)
        NS_LOG_INFO("Router with AS " << AS << " not found. Please insert a router with a valid AS.\n");   
}

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



void checkHoldTime(std::vector<Router>* routers) {
    for(int i=0; i<(int)routers->size(); i++) {
        std::vector<Interface> interfaces = (*routers)[i].get_router_int();

        for(int j=0; j<(int)interfaces.size(); j++) {
            /*NS_LOG_INFO("Interface " << interfaces[j].name << " of router " << (*routers)[i].get_router_AS() << " has status " << interfaces[j].status << " and has client " << interfaces[j].client.has_value() << " and has server " << interfaces[j].server.has_value() <<  " at time " << Simulator::Now().GetSeconds() << " [PERIODIC HOLD TIME CHECK]");
            if(interfaces[j].client) {
                Address to;
                to = interfaces[j].client.value()->m_peer;
                InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);
                NS_LOG_INFO("Socket " << toAddress.GetIpv4() << " m_running " << interfaces[j].client.value()->m_running);
            } else if (interfaces[j].server){
                Address to;
                //interfaces[j].server.value()->m_socket->GetPeerName(to);
                //InetSocketAddress toAddress = InetSocketAddress::ConvertFrom(to);
                NS_LOG_INFO("Socket server" << interfaces[j].server.value()->m_socket << " list len " << interfaces[j].server.value()->m_socketList.size()); //<< " add " << toAddress.GetIpv4()) ;
            }*/

            if(Simulator::Now().GetSeconds() - interfaces[j].get_last_update_time() > interfaces[j].get_max_hold_time()) {
                if (interfaces[j].status) {
                    NS_LOG_INFO("Interface " << interfaces[j].name << " of router " << (*routers)[i].get_router_AS() << " has expired hold time  at time " << Simulator::Now().GetSeconds() << " [PERIODIC HOLD TIME CHECK]");
            
                    std::stringstream msgStreamNotification;
                    MessageNotification msg = MessageNotification(4,0);
                    msgStreamNotification << msg << '\0';

                    Ptr<Packet> packetNotification = Create<Packet>((uint8_t*) msgStreamNotification.str().c_str(), msgStreamNotification.str().length()+1);

                    if(interfaces[j].client) {
                        interfaces[j].client.value()->Send(interfaces[j].client.value()->get_socket(), packetNotification);
                    } else if (interfaces[j].server){
                        interfaces[j].server.value()->Send(interfaces[j].server.value()->get_socket(), packetNotification);
                    }

                    interfaces[j].client.reset();
                    interfaces[j].server.reset();

                    (*routers)[i].set_interface_status(j, false);
                    (*routers)[i].set_interface(interfaces[j], j);
                    //this->StopApplication();
                }
            }
        }
    } 

    Simulator::Schedule(Seconds(60.0), &checkHoldTime, routers); 

}

int get_router_index_from_AS(std::vector<Router>* routers, int AS) {
    for(int i=0; i<(int)routers->size(); i++) {
        if ((*routers)[i].get_router_AS() == AS) {
            return i;
        }
    }
    return -1;
}

//Option 1: inserire una ref del neigh nell'interface e poi scorrere a catena
//Option 2: fare una cosa periodica come il controllo dell'holt time in topology e aggiornare la voted trust nelle interfaces
void exchangeVotedTrust(std::vector<Router>* routers) {
    std::cout << "\n\n--------------- START VOTING TRUST SHARING ------------\n" << std::endl;

    for(int i=0; i<(int)routers->size(); i++) {
        std::vector<Interface> interfaces = (*routers)[i].get_router_int();
        std::cout << "--------------- ROUTER: " << (*routers)[i].get_router_AS() << "----------------" << std::endl;

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

    Simulator::Schedule(Seconds(200.0), &exchangeVotedTrust, routers); 

}

void userInputCallback(std::vector<Router>* routers) {
    char c;
    
    std::cout << "\n\nSelect one of the following to be executed at time " << (Simulator::Now().GetSeconds()) <<
        ": \n\n"
        "[1] Add a new policy\n"
        "[2] Disable a link \n"
        "[3] Disable a router \n"
        "[4] Continue the simulation \n"
        "[5] Print Routing tables \n"
        "[6] Quit\n\n"
        "Choose a number: ";

    std::cin >> c;
    std::cout << std::endl;

    if (isdigit(c)) {
        switch (c) {
            case '1':
                {
                    std::string ip1, ip2;
                    int type, value, lenght;
                
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
                            //pos = mask.find(".");
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

                    std::cout << "Enter the two AS numbes of the routers that forms the link: ";
                    std::cin >> AS1 >> AS2;
                    std::cout << std::endl;

                    //std::cout << "You entered: " << AS1 << " and " << AS2 << std::endl;
                    if(checkAS(routers, AS1) && checkAS(routers, AS2))
                        disable_router_link(routers, AS1, AS2);
                    break;
                }
            case '3':
                {
                    int AS;

                    std::cout << "Enter the AS number of the router to disable: ";
                    std::cin >> AS;
                    std::cout << std::endl;

                    disable_router(routers, AS);
                    break;
                }
            case '4':
                break;

            case '6':
                exit(EXIT_SUCCESS);
                break;
            
            case '5':
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
                std::cout << "Invalid number, choose a number between 1 and 6.\n " << std::endl;
                break;
        }
    }
    else {
        std::cout << "Invalid input, choose a number between 1 and 6.\n\n ";
    }

    // schedule the next user input callback
    Simulator::Schedule(Seconds(60.0), &userInputCallback, routers);
}

//int enable_router_link() {
//    return 0;
//}

int main() {

    Time::SetResolution(Time::NS);
    Packet::EnablePrinting();

    LogComponentEnable("Router", LOG_LEVEL_INFO);
    LogComponentEnable("BGPServer", LOG_LEVEL_INFO);
    LogComponentEnable("BGPClient", LOG_LEVEL_INFO);
    LogComponentEnable("TCPCommon", LOG_LEVEL_INFO);
    //LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    //LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NS_LOG_INFO("Loading topology");
    std::vector<Router> network = load_configuration();

    NS_LOG_INFO("\nCreating links");
    network = createLinks(network);

    // Create router nodes, initialize routing database and set up the routing tables in the nodes
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    NS_LOG_INFO("\nPrinting topology");
    printTopology(network);

    NS_LOG_INFO("\nBGP state: IDLE");

    NS_LOG_INFO("\nCreate BGP applications");
    network = createBGPConnections(network);

    NS_LOG_INFO("\nBGP state: CONNECT");

    NS_LOG_INFO("\nSending open messages");
    sendOpenMsg(network);


    NS_LOG_INFO("\nBGP state: OPEN SENT");
    NS_LOG_INFO("BGP state: OPEN CONFIRM\n");

    //NS_LOG_INFO("\nStarting incrementing the hold timer");
    //startHoldTimer(network);

    NS_LOG_INFO("\nStart sending KEEPALIVE messages every 30 seconds");
    //startKeepAlive(network);

    NS_LOG_INFO("\nBGP state: ESTABLISHED\n");

    NS_LOG_INFO("\nSending update messages\n");

    // schedule the first user input callback to run after the simulation starts
    Simulator::Schedule(Seconds(45.0), &userInputCallback, &network);

    Simulator::Schedule(Seconds(100.0), &checkHoldTime, &network);

    Simulator::Schedule(Seconds(150.0), &exchangeVotedTrust, &network);

    Simulator::Stop(Seconds(800.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}