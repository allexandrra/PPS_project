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

void start_simulation() {

  // Create the 10 nodes for the topology
//    NS_LOG_INFO("Create nodes");
//    NodeContainer c;
//    c.Create(11);
//    NodeContainer as1as2 = NodeContainer(c.Get(0), c.Get(1));
//    NodeContainer as2as3 = NodeContainer(c.Get(1), c.Get(2));
//    NodeContainer as3as4 = NodeContainer(c.Get(2), c.Get(3));
//    NodeContainer as4as5 = NodeContainer(c.Get(3), c.Get(4));
//    NodeContainer as5as6 = NodeContainer(c.Get(4), c.Get(5));
//    NodeContainer as2as7 = NodeContainer(c.Get(1), c.Get(6));
//    NodeContainer as7as5 = NodeContainer(c.Get(6), c.Get(4));
//    NodeContainer as2as8 = NodeContainer(c.Get(1), c.Get(7));
//    NodeContainer as8as9 = NodeContainer(c.Get(7), c.Get(8));
//    NodeContainer as9as10 = NodeContainer(c.Get(8), c.Get(9));
//    NodeContainer as10as6 = NodeContainer(c.Get(9), c.Get(5));
//
//    InternetStackHelper internet;
//    internet.Install(c);
//
//    // Create the 11 peer-to-peer channels without any IP addressing information
//    NS_LOG_INFO("Create channels");
//    PointToPointHelper p2p;
//    p2p.SetDeviceAttribute("DataRate", StringValue("1024Mbps"));
//    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
//
//    NetDeviceContainer d1d2 = p2p.Install(as1as2);
//    NetDeviceContainer d2d3 = p2p.Install(as2as3);
//    NetDeviceContainer d3d4 = p2p.Install(as3as4);
//    NetDeviceContainer d4d5 = p2p.Install(as4as5);
//    NetDeviceContainer d5d6 = p2p.Install(as5as6);
//    NetDeviceContainer d2d7 = p2p.Install(as2as7);
//    NetDeviceContainer d7d5 = p2p.Install(as7as5);
//    NetDeviceContainer d2d8 = p2p.Install(as2as8);
//    NetDeviceContainer d8d9 = p2p.Install(as8as9);
//    NetDeviceContainer d9d10 = p2p.Install(as9as10);
//    NetDeviceContainer d10d6 = p2p.Install(as10as6);
//
//    // Add IP addresses to the channels
//    NS_LOG_INFO("Assign IP Addresses");
//    Ipv4AddressHelper ipv4;
//
//    ipv4.SetBase("160.0.0.0", "255.255.255.252");
//    Ipv4InterfaceContainer i1i2 = ipv4.Assign(d1d2);
//    ipv4.SetBase("160.0.0.4", "255.255.255.252");
//    Ipv4InterfaceContainer i2i3 = ipv4.Assign(d2d3);
//    ipv4.SetBase("160.0.0.12", "255.255.255.252");
//    Ipv4InterfaceContainer i3i4 = ipv4.Assign(d3d4);
//    ipv4.SetBase("160.0.0.20", "255.255.255.252");
//    Ipv4InterfaceContainer i4i5 = ipv4.Assign(d4d5);
//    ipv4.SetBase("160.0.0.24", "255.255.255.252");
//    Ipv4InterfaceContainer i5i6 = ipv4.Assign(d5d6);
//    ipv4.SetBase("160.0.0.8", "255.255.255.252");
//    Ipv4InterfaceContainer i2i7 = ipv4.Assign(d2d7);
//    ipv4.SetBase("160.0.0.16", "255.255.255.252");
//    Ipv4InterfaceContainer i7i5 = ipv4.Assign(d7d5);
//    ipv4.SetBase("160.0.0.40", "255.255.255.252");
//    Ipv4InterfaceContainer i2i8 = ipv4.Assign(d2d8);
//    ipv4.SetBase("160.0.0.36", "255.255.255.252");
//    Ipv4InterfaceContainer i8i9 = ipv4.Assign(d8d9);
//    ipv4.SetBase("160.0.0.32", "255.255.255.252");
//    Ipv4InterfaceContainer i9i10 = ipv4.Assign(d9d10);
//    ipv4.SetBase("160.0.0.28", "255.255.255.252");
//    Ipv4InterfaceContainer i10i6 = ipv4.Assign(d10d6);
//
//    // Create router nodes, initialize routing database and set up the routing tables in the nodes
//    //Ipv4GlobalRoutingHelper::PopulateRoutingTables();
//    // Enable packet printing
//    Packet::EnablePrinting();
//
//    // LogComponentEnable("MessageBGP", LOG_LEVEL_INFO);
//
//    // Settings
//    uint16_t serverPort = 179;
//    // Set timings
//    Time startClient = Seconds(1.);
//    Time stopClient = Seconds(300.);
//    Time startServer = Seconds(0.);
//    Time stopServer = Seconds(300.0);
//    Time stopSimulation = Seconds(310.0); // giving some more time than the server stop
//
//    NS_LOG_INFO("IP address of server on as2: " << i1i2.GetAddress(1));
//    NS_LOG_INFO("IP address of client1 on as1: " << i1i2.GetAddress(0));
//    NS_LOG_INFO("IP address of client2 on as7: " << i2i7.GetAddress(1));
//
//    // Applications
//    // TCP RECEIVER -> now on as2 which is directly linked (p2p) with as1
//    Address receiverAddress(InetSocketAddress(i1i2.GetAddress(1), serverPort));
//    // Ptr<TcpReceiver> receiverApp = CreateObject<TcpReceiver>();
//    Ptr<BGPServer> serverApp = CreateObject<BGPServer>();
//    serverApp->Setup(serverPort, startClient);
//
//    // Install and load the server on as2
//    as1as2.Get(1)->AddApplication(serverApp);
//    serverApp->SetStartTime(startServer);
//    serverApp->SetStopTime(stopServer);
//
//    // Tcp SENDER -> now on the as1
//    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(as1as2.Get(0), TcpSocketFactory::GetTypeId());
//    Ptr<BGPClient> clientApp = CreateObject<BGPClient>();
//    clientApp->Setup(ns3TcpSocket, receiverAddress, startClient, stopClient);
//
//    // Install and load the client on as1
//    as1as2.Get(0)->AddApplication(clientApp);
//    clientApp->SetStartTime(startClient);
//    clientApp->SetStopTime(stopClient);
//
//    std::stringstream msg1;
//    // MessageOpen msg = MessageOpen(3556, 0, 16908288, 0);
//    // msg.set_lenght(1050);
//    // msg.set_type('1');
//    MessageNotification msg = MessageNotification(1, 1, "test");
//    msg1 << msg << '\0';
//
//    /*MessageNotification msgRic;
//    msg1 >> msgRic;
//    std::cout << "Lunghezza: " << msgRic.get_lenght() << std::endl;
//    std::cout << "Type: " << msgRic.get_type() << std::endl;
//    std::vector<uint8_t> vec = msgRic.get_marker();
//    std::cout << "Marker: ";
//    for(int i = 0; i < 128; i++)
//        std::cout << vec[i];
//    std::cout << std::endl;
//    std::cout << "Error code: " << msgRic.get_error_code() << std::endl;
//    std::cout << "Error subcode: " << msgRic.get_error_subcode() << std::endl;
//    std::cout << "Data: " << msgRic.get_data() << std::endl; */
//    // std::cout << "Version: " << msgRic.get_version() << std::endl;
//    // std::cout << "AS: " << msgRic.get_AS() << std::endl;
//    // std::cout << "Hold Time: " << msgRic.get_hold_time() << std::endl;
//    // std::cout << "BGP ID: " << msgRic.get_BGP_id() << std::endl;
//    // std::cout << "Opt Parm Len: " << msgRic.get_opt_param_len() << std::endl;
//
//    clientApp->AddPacketsToQueue(msg1, Seconds(2.0));
//
//    std::stringstream msg2;
//    msg2 << "Ciao" << '\0';
//    clientApp->AddPacketsToQueue(msg2, Seconds(3.0));
//
//    // Tcp SENDER -> now on the as7
//    Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket(as2as7.Get(1), TcpSocketFactory::GetTypeId());
//    Ptr<BGPClient> clientApp2 = CreateObject<BGPClient>();
//    clientApp2->Setup(ns3TcpSocket2, receiverAddress, startClient, stopClient);
//
//    // Install and load the client on as7
//    as2as7.Get(1)->AddApplication(clientApp2);
//    clientApp2->SetStartTime(startClient);
//    clientApp2->SetStopTime(stopClient);
//
//    std::stringstream msg3;
//    msg3 << "Hello World client 2" << '\0';
//    clientApp2->AddPacketsToQueue(msg3, Seconds(4.0));
//
//    std::stringstream msg4;
//    msg4 << "Ciao client 2" << '\0';
//    clientApp2->AddPacketsToQueue(msg4, Seconds(6.0));

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


//void testUdp(){
//    //inport
//    std::vector<Router> routers;
//    InternetStackHelper internet;
//
//
//    //node 1 creation
//    NodeContainer node;
//    node.Create(1);
//    internet.Install(node);
//    string name = "AS1";
//    Ipv4Address ip = Ipv4Address(string("1.1.1.0").c_str());
//    Ipv4Mask netmask = Ipv4Mask(string("255.255.255.0").c_str());
//    int ASnum = std::stoi(name.substr(2, name.size()-2));
//    Router router = Router(ASnum, node, ip, netmask); 
//    router.add_neighbour(2);  
//    routers.push_back(router);
//
//    std::cout << "Router " << ASnum << " created, with IP: " << ip
//                << ", and netmask: " << netmask << std::endl;
//
//    NodeContainer node2;
//    node2.Create(1);
//    internet.Install(node2);
//    string name2 = "AS2";
//    Ipv4Address ip2 = Ipv4Address(string("2.2.2.0").c_str());
//    Ipv4Mask netmask2 = Ipv4Mask(string("255.255.255.0").c_str());
//    int ASnum2 = std::stoi(name2.substr(2, name2.size()-2));
//    Router router2 = Router(ASnum2, node2, ip2, netmask2); 
//    router2.add_neighbour(1);  
//    routers.push_back(router2);
//
//    std::cout << "Router " << ASnum2 << " created, with IP: " << ip2
//                << ", and netmask: " << netmask2 << std::endl;
//
//
////    NodeContainer nc = NodeContainer(routers[0].get_router_node().Get(0), routers[1].get_router_node().Get(0));
////
////    PointToPointHelper p2p;
////    p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
////    p2p.SetChannelAttribute("Delay", StringValue("1ms"));
////
////    NetDeviceContainer ndc = p2p.Install(nc);
////
////    Ipv4AddressHelper ipv4;
////    int base = 0;
////    const char * ipLink  = ("160.0.0." + std::to_string(base)).c_str();
////    base += 4;
////    ipv4.SetBase(ipLink, "255.255.255.252");
////
////    Ipv4InterfaceContainer iic = ipv4.Assign(ndc);
//
//
//    routers = createLinks(routers);
//
//    //printTopology(routers);
//
//    //NS_LOG_INFO("IP address of server: " << iic.GetAddress(0));
//    //NS_LOG_INFO("IP address of client: " << iic.GetAddress(1));
//
//
//    // Create the UDP echo server to test the communication between 2 adjacent nodes
//    NS_LOG_INFO("Create UDP echo server");
//
//    UdpEchoServerHelper echoServer(9);
//    ApplicationContainer serverApps = echoServer.Install(routers[0].get_router_node().Get(0));
//    serverApps.Start(Seconds(1.0));
//    serverApps.Stop(Seconds(10.0));
//
//
//    // Create the UDP echo client echo to test the communication between 2 adjacent nodes
//    NS_LOG_INFO("Create UDP echo client");
//
//    // Set information about the server on the client application
//    UdpEchoClientHelper echoClient(routers[0].get_router_int_from_num(1).ip_address, 9);
//    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
//    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
//    echoClient.SetAttribute("PacketSize", UintegerValue(1024));
//
//    ApplicationContainer clientApps = echoClient.Install(routers[1].get_router_node().Get(0));
//    clientApps.Start(Seconds(2.0));
//    clientApps.Stop(Seconds(10.0));
//
//    //return routers;
//
//}

std::vector<Router> createBGPConnections(std::vector<Router> routers) {
    int serverPort = 179;
    Time startClient = Seconds(1.);
    Time stopClient = Seconds(3000.0);
    Time startServer = Seconds(0.);
    Time stopServer = Seconds(3000.0);

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
            // 1. the server is installed on the router with i as AS number
            // 2. the client is installed on the router with i as AS number
            // 3. the server is installed on the interface with the AS number of the neighbour
            // 4. the client is installed on the interface with the AS number of the neighbour

            // se è client e il vicino è server non devo fare nulla 
            // se non è client o il vicino non è server devo installare il client e il server

            if(!(routers[i].get_router_int_from_name(neighbours[j])).client || !(routers[neighbourIndex].get_router_int_from_name(routers[i].get_router_AS())).server) {

                std::cout << "Router " << routers[i].get_router_AS() << " is server on interface eth" << neighbours[j] << " and router " << neighbours[j] << " is client on interface eth" << routers[i].get_router_AS() << std::endl;
                
                Address receiverAddress(InetSocketAddress(routers[i].get_router_int_from_name(neighbours[j]).ip_address, serverPort));
                Ptr<BGPServer> serverApp = CreateObject<BGPServer>();
                serverApp->Setup(serverPort, startClient);

                // Install and load the server 
                routers[i].get_router_node().Get(0)->AddApplication(serverApp);
                serverApp->SetStartTime(startServer);
                serverApp->SetStopTime(stopServer);

                routers[i].setServer(neighbourInterfaceIndex, serverApp);

                // Tcp SENDER -> now on the as2
                Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(routers[neighbourIndex].get_router_node().Get(0), TcpSocketFactory::GetTypeId());
                Ptr<BGPClient> clientApp = CreateObject<BGPClient>();
                clientApp->Setup(ns3TcpSocket, receiverAddress, startClient, stopClient);

                // Install and load the client on as2
                routers[neighbourIndex].get_router_node().Get(0)->AddApplication(clientApp);
                clientApp->SetStartTime(startClient);
                clientApp->SetStopTime(stopClient);

                int routerInterfaceIndex = routers[neighbourIndex].get_router_int_num_from_name(routers[i].get_router_AS());
                routers[neighbourIndex].setClient(routerInterfaceIndex, clientApp);
            }
        
        } 
    }
    return routers;
}

void testTCP(){
    //inport
    std::vector<Router> routers;
    InternetStackHelper internet;


    //node 1 creation
    NodeContainer node;
    node.Create(1);
    internet.Install(node);
    string name = "AS1";
    Ipv4Address ip = Ipv4Address(string("1.1.1.0").c_str());
    Ipv4Mask netmask = Ipv4Mask(string("255.255.255.0").c_str());
    int ASnum = std::stoi(name.substr(2, name.size()-2));
    Router router = Router(ASnum, node, ip, netmask); 
    router.add_neighbour(2);  
    router.add_neighbour(3);  
    routers.push_back(router);

    std::cout << "Router " << ASnum << " created, with IP: " << ip
                << ", and netmask: " << netmask << std::endl;

    NodeContainer node2;
    node2.Create(1);
    internet.Install(node2);
    string name2 = "AS2";
    Ipv4Address ip2 = Ipv4Address(string("2.2.2.0").c_str());
    Ipv4Mask netmask2 = Ipv4Mask(string("255.255.255.0").c_str());
    int ASnum2 = std::stoi(name2.substr(2, name2.size()-2));
    Router router2 = Router(ASnum2, node2, ip2, netmask2); 
    router2.add_neighbour(1);  
    routers.push_back(router2);

    std::cout << "Router " << ASnum2 << " created, with IP: " << ip2
                << ", and netmask: " << netmask2 << std::endl;


    NodeContainer node3;
    node3.Create(1);
    internet.Install(node3);
    string name3 = "AS3";
    Ipv4Address ip3 = Ipv4Address(string("3.3.3.0").c_str());
    Ipv4Mask netmask3 = Ipv4Mask(string("255.255.255.0").c_str());
    int ASnum3 = std::stoi(name3.substr(2, name3.size()-2));
    Router router3 = Router(ASnum3, node3, ip3, netmask3); 
    router3.add_neighbour(1);  
    routers.push_back(router3);

    std::cout << "Router " << ASnum3 << " created, with IP: " << ip3
                << ", and netmask: " << netmask3 << std::endl;


    routers = createLinks(routers);

    printTopology(routers);

    //NS_LOG_INFO("IP address of server: " << iic.GetAddress(0));
    //NS_LOG_INFO("IP address of client: " << iic.GetAddress(1));


    // Applications
    /*int serverPort = 179;
    Time startClient = Seconds(1.);
    Time stopClient = Seconds(3000.);
    Time startServer = Seconds(0.);
    Time stopServer = Seconds(3000.0);

    // TCP RECEIVER -> now on as1 which is directly linked (p2p) with as2
    std::vector<int> neighbours = routers[0].get_router_neigh();
    //NS_LOG_INFO("INDEX: " << neighbours[0]);
    Address receiverAddress(InetSocketAddress(routers[0].get_router_int_from_name(neighbours[0]).ip_address, serverPort));
    //NS_LOG_INFO("INDEX: " << neighbours[1]);
    Address receiverAddress2(InetSocketAddress(routers[0].get_router_int_from_name(neighbours[1]).ip_address, serverPort));
    Ptr<BGPServer> serverApp = CreateObject<BGPServer>();
    serverApp->Setup(serverPort, startClient);

    // Install and load the server on as1
    routers[0].get_router_node().Get(0)->AddApplication(serverApp);
    serverApp->SetStartTime(startServer);
    serverApp->SetStopTime(stopServer);

    // Tcp SENDER -> now on the as2
    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(routers[1].get_router_node().Get(0), TcpSocketFactory::GetTypeId());
    Ptr<BGPClient> clientApp = CreateObject<BGPClient>();
    clientApp->Setup(ns3TcpSocket, receiverAddress, startClient, stopClient);

    // Tcp SENDER 2 -> now on the as3
    Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket(routers[2].get_router_node().Get(0), TcpSocketFactory::GetTypeId());
    Ptr<BGPClient> clientApp2 = CreateObject<BGPClient>();
    clientApp2->Setup(ns3TcpSocket2, receiverAddress2, startClient, stopClient);

    // Install and load the client on as2
    routers[1].get_router_node().Get(0)->AddApplication(clientApp);
    clientApp->SetStartTime(startClient);
    clientApp->SetStopTime(stopClient);

    // Install and load the client on as1
    routers[2].get_router_node().Get(0)->AddApplication(clientApp2);
    clientApp->SetStartTime(startClient);
    clientApp->SetStopTime(stopClient); */

    routers = createBGPConnections(routers);


    std::stringstream msg1;
    MessageOpen msg = MessageOpen(3556, 0, 16908288, 0);
    msg.set_lenght(1050);
    msg.set_type('1');
    msg1 << msg << '\0';

    NS_LOG_INFO("\nSending test message 1");
    Ptr<BGPClient> clientApp = routers[1].get_router_int_from_name(1).client.value();
    clientApp->AddPacketsToQueue(msg1, Seconds(5.0));

    //NS_LOG_INFO("\nSending test message 2");
    //clientApp2->AddPacketsToQueue(msg1, Seconds(5.0)); 

    //return routers;

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
    
    char c;

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


    NS_LOG_INFO("\nCreate BGP applications");
    network = createBGPConnections(network);

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

    Simulator::Stop(Seconds(15.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
