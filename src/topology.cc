/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

//
// Network topology: Topology 1 image on Drive
// All links are point-to-point links
// To run the simulation: ./ns3 run scratch/PPS_project/src/topology.cc
// To view the logs: export NS_LOG=Topology=info

#include "../include/AS.h"
#include "../include/BGP-client.h"
#include "../include/BGP-server.h"
#include "../include/MessageHeader.h"
#include "../include/MessageNotification.h"
#include "../include/MessageUpdate.h"
#include "../include/MessageOpen.h"
#include "../include/configuration_parser.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Topology");

std::stringstream createOpenMsg(Router r) {
    std::stringstream msgStream;
    MessageOpen msg = MessageOpen(r.get_router_AS(), 0, r.get_router_ID());
    msgStream << msg << '\0';
    return msgStream;
}

void sendOpenMsg(std::vector<Router> routers) {
    //Every client sends an open message to the server
    //The server will reply with an open message

    for(int i=0; i<(int)routers.size(); i++) {

        std::vector<Interface> interfaces = routers[i].get_router_int();

        //NS_LOG_INFO("Router " << routers[i].get_router_AS() << " has " << routers[i].get_router_ID() << " as router ID");
        
        for (int j=0; j<(int)interfaces.size(); j++) {
            if(!interfaces[j].isServer && interfaces[j].client.has_value()) {
                //send msg
                std::stringstream msgStream = createOpenMsg(routers[i]);
                interfaces[j].client.value()->AddPacketsToQueue(msgStream, Seconds(0.0));
            }
        }
        

    }
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

            //std::cout << "Router " << routers[i].get_router_AS() << " - Vicino: " << routers[neighbourIndex].get_router_AS() << std::endl;

            NodeContainer nc = NodeContainer(routers[i].get_router_node().Get(0), routers[neighbourIndex].get_router_node().Get(0));

            PointToPointHelper p2p;
            p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
            p2p.SetChannelAttribute("Delay", StringValue("1ms"));

            NetDeviceContainer ndc = p2p.Install(nc);

            Ipv4AddressHelper ipv4;
            const char * ip  = ("160.0.0." + std::to_string(base)).c_str();
            base += 4;
            ipv4.SetBase(ip, "255.255.255.252");

            Ipv4InterfaceContainer iic = ipv4.Assign(ndc);

            Interface if1 = Interface("eth"+std::to_string(neighbours[j]), iic.GetAddress(0), iic.GetAddress(1), Ipv4Address("255.255.255.252"));
            Interface if2 = Interface("eth"+std::to_string(routers[i].get_router_AS()), iic.GetAddress(1), iic.GetAddress(0), Ipv4Address("255.255.255.252"));

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
                //NS_LOG_INFO("Adding interface " << if1.name << " to router " << routers[i].get_router_AS());
                routers[i].add_interface(if1);
            }
            
            std::vector<Interface> ifacesJ = routers[neighbourIndex].get_router_int();
            if((std::find_if(ifacesJ.begin(), ifacesJ.end(),find_interface(if1.name)) == ifacesJ.end()) && (std::find_if(ifacesJ.begin(), ifacesJ.end(),find_interface(if2.name)) == ifacesJ.end())) {
                //NS_LOG_INFO("Adding remote interface " << if2.name << " to router " << routers[neighbourIndex].get_router_AS());
                routers[neighbourIndex].add_interface(if2);
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

void startHoldTimer(std::vector<Router> routers) {
    for(int i=0; i<(int)routers.size(); i++) {
        std::vector<Interface> interfaces = routers[i].get_router_int();
        for(int j = 0; j < (int) interfaces.size(); j++) {
            Simulator::Schedule (Seconds(5.0), &Interface::increment_hold_time, &interfaces[j]);
        }
    }
}

void startKeepAlive(std::vector<Router> routers) {
    //Every client sends an keepalive message to the server
    //The server will reply with a keepalive message

    for(int i=0; i<(int)routers.size(); i++) {

        std::vector<Interface> interfaces = routers[i].get_router_int();

        //NS_LOG_INFO("Router " << routers[i].get_router_AS() << " has " << routers[i].get_router_ID() << " as router ID");
        
        for (int j=0; j<(int)interfaces.size(); j++) {
            if(!interfaces[j].isServer && interfaces[j].client.has_value()) {
                //send msg
                //std::stringstream msgStream = createOpenMsg(routers[i]);
                //interfaces[j].client.value()->AddPacketsToQueue(msgStream, Seconds(20.0));
                //std::stringstream msgStream;
                //MessageHeader msg = MessageHeader(0);
                //msgStream << msg << '\0';

                //interfaces[j].client.value()->AddPacketsToQueue(msgStream, Seconds(2.0));
                interfaces[j].client.value()->AddPacketsToQueuePeriodically();
            }
        }
        

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
                serverApp->SetRouter(&routers[i]);
                serverApp->SetStartTime(startServer);
                serverApp->SetStopTime(stopServer);

                routers[i].setServer(neighbourInterfaceIndex, serverApp);

                // Tcp SENDER -> now on the as2
                Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(routers[neighbourIndex].get_router_node().Get(0), TcpSocketFactory::GetTypeId());
                Ptr<BGPClient> clientApp = CreateObject<BGPClient>();
                clientApp->Setup(ns3TcpSocket, receiverAddress, startClient, stopClient);

                // Install and load the client on as2
                routers[neighbourIndex].get_router_node().Get(0)->AddApplication(clientApp);
                clientApp->SetRouter(&routers[neighbourIndex]);
                clientApp->SetStartTime(startClient);
                clientApp->SetStopTime(stopClient);

                int routerInterfaceIndex = routers[neighbourIndex].get_router_int_num_from_name(routers[i].get_router_AS());
                routers[neighbourIndex].setClient(routerInterfaceIndex, clientApp);
            }
        
        } 
    }
    return routers;
}

int send_ip_message() {
    return 0;
}

int disable_router_link() {
    return 0;
}

int enable_router_link() {
    return 0;
}


int main() {

    Time::SetResolution(Time::NS);
    Packet::EnablePrinting();
    
    //char c;

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

    NS_LOG_INFO("\nStarting sending KEEPALIVE every 30 seconds");
    startKeepAlive(network);

    //NS_LOG_INFO("\nBGP state: ESTABLISHED\n");

//    while (1)
//    {
//        printf("\n\nSelect one of the following: \n\n"
//               "[1] Add a new policy\n"
//               "[2] Disable a link \n"
//               "[3] Enable a link \n"
//               "[4] Quit\n\n"
//               "Choose a number: ");
//
//        std::cin >> c;
//
//        if (isdigit(c))
//        {
//            switch (c)
//            {
//            case '1':
//                send_ip_message();
//                break;
//
//            case '2':
//                disable_router_link();
//                break;
//
//            case '3':
//                enable_router_link();
//                break;
//
//            case '4':
//                exit(EXIT_SUCCESS);
//                break;
//
//            default:
//                printf("Invalid number, choose a number between 1 and 4. ");
//                break;
//            }
//        }
//        else
//        {
//            printf("Invalid input, choose a number between 1 and 4. ");
//        }
//    }

    Simulator::Stop(Seconds(500.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
