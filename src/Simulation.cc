/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

//
// Network topology: Topology 1 image on Drive
// All links are point-to-point links 
// To run the simulation: ./ns3 run scratch/PPS_project/Topology.cc
// To view the logs: export NS_LOG=Topology=info

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <algorithm> 

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"

#include "../include/TCP-common.h"
#include "../include/BGP-client.h"
#include "../include/BGP-server.h"
#include "../include/MessageHeader.h"
#include "../include/MessageOpen.h"
#include "../include/MessageNotification.h"
#include "../include/Router.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Simulation");

int main(int argc, char *argv[])
{
    Time::SetResolution(Time::NS); 

    LogComponentEnable("Router", LOG_LEVEL_INFO);

    InternetStackHelper internet;

    int num_nodes = 10;
    std::vector<Router> routers;
    std::vector<std::string> links;
    links.push_back("AS2");
    links.push_back("AS3");

    for(int i=0; i<num_nodes; i++) {
        Ipv4Address ip = Ipv4Address("101.25.0.1");
        Ipv4Mask mask = Ipv4Mask("255.255.255.0");
        NodeContainer node;
        node.Create(1);
        internet.Install(node);
        Router r = Router(i, i, node, ip, mask);

        // Parsing of router links
        for (int j = 0; j < (int)links.size(); j++) {
            int ASnum = std::stoi(links[j].substr(2, links[j].size()-2));
            r.add_neighbour(ASnum);  

            //NS_LOG_INFO("Adding link " << ASnum << " to router with id " << r.get_router_ID()); 
        }

        routers.push_back(r);
    }   

    //Need to create the point to point links between the nodes
    for(int i=0; i<(int)routers.size(); i++) {
        
        std::vector<int> neighbours = routers[i].get_router_neigh();

        std::string debug;
        for(int i=0; i<(int)neighbours.size(); i++) {
            debug += std::to_string(neighbours[i]) + " ";
        }
        //NS_LOG_INFO("Router " << routers[i].get_router_ID() << " has neighbours " << debug);
        
        for(int j=0; j<(int)neighbours.size(); j++) {

            NodeContainer nc = NodeContainer(routers[i].get_router_node().Get(0), routers[j].get_router_node().Get(0));
            
            PointToPointHelper p2p;
            p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
            p2p.SetChannelAttribute("Delay", StringValue("2ms"));

            NetDeviceContainer ndc = p2p.Install(nc);

            Ipv4AddressHelper ipv4;
            std::string s = "160.0.0." + std::to_string(i*4*neighbours.size()+j*4);
            const char * ip = s.c_str();
            //NS_LOG_INFO("IP address is " << ip);
            ipv4.SetBase(ip, "255.255.255.252");

            Ipv4InterfaceContainer iic = ipv4.Assign(ndc);

            //NS_LOG_INFO("Creating link between router " << routers[i].get_router_ID() << " and router " << neighbours[j]);

            Interface if1 = Interface("eth"+std::to_string(neighbours[j]), iic.GetAddress(0), iic.GetAddress(1), Ipv4Address("255.255.255.252"));
            Interface if2 = Interface("eth"+std::to_string(routers[i].get_router_ID()), iic.GetAddress(1), iic.GetAddress(0), Ipv4Address("255.255.255.252"));

            //TODO: check if the interface is already there
            struct find_interface : std::unary_function<Interface, bool> {
                std::string name;
                find_interface(std::string name):name(name) { }
                bool operator()(Interface const& i) const {
                    return i.name == name;
                }
            };


            std::vector<Interface> ifacesI = routers[i].get_router_int();
            if((std::find_if(ifacesI.begin(), ifacesI.end(),find_interface(if1.name)) == ifacesI.end()) && (std::find_if(ifacesI.begin(), ifacesI.end(),find_interface(if2.name)) == ifacesI.end())) {
                //NS_LOG_INFO("Adding interface " << if1.name << " to router " << routers[i].get_router_ID());
                routers[i].add_interface(if1);
            }
    
            std::vector<Interface> ifacesJ = routers[neighbours[j]].get_router_int();
            
            //std::string test = "";
            //for(int k=0; k<(int)routers[neighbours[j]].get_router_int().size(); k++) {
            //    test += routers[neighbours[j]].get_router_int()[k].name + " ";
            //}
            //NS_LOG_INFO("   Router " << routers[neighbours[j]].get_router_ID() << " has " << routers[neighbours[j]].get_router_int().size() << " interfaces: " << test);

            if((std::find_if(ifacesJ.begin(), ifacesJ.end(),find_interface(if1.name)) == ifacesJ.end()) || (std::find_if(ifacesJ.begin(), ifacesJ.end(),find_interface(if2.name)) == ifacesJ.end())) {
                //NS_LOG_INFO("Adding remote interface " << if2.name << " to router " << routers[neighbours[j]].get_router_ID());
                routers[neighbours[j]].add_interface(if2);
            }
           

        }

    }


    //Need to create the point to point links between the nodes
    for(int i=0; i<(int)routers.size(); i++) {
        std::string s = "";
        for(int j=0; j<(int)routers[i].get_router_int().size(); j++) {
            std::stringstream ip;
            s += routers[i].get_router_int()[j].name; 
            //s += " (";
            //routers[i].get_router_int()[j].local_ip.Print(ip);
            //s += ip.str();
            //s += ") ";
        }
        NS_LOG_INFO("Router " << routers[i].get_router_ID() << " has " << routers[i].get_router_int().size() << " interfaces: " << s);
        NS_LOG_INFO("--------------------------------------");
    }

}