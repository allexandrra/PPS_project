#include "../include/configuration_parser.h"
#include "../include/json.hpp"

#include <fstream>
#include <iostream>
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

using json = nlohmann::json;
using namespace std;
using namespace ns3;

/**
 * @brief Load the configuration file and create the routers of the topology described in the file
*/
std::vector<Router> load_configuration() {
    // vector containing the final routers
    std::vector<Router> routers;

    // Open the configuration file and create a stream with that
    std::ifstream f(string(filesystem::current_path()) +
                    "/scratch/PPS_project/configuration_file.json");

    // define the NS3 internet stack
    InternetStackHelper internet;

    try {
        // instance the json parser
        json config_raw = json::parse(f);

        // Iterate over AS
        for (auto& element : config_raw)
        {
            // parsing of router names
            if (!element.contains("name"))
            {
                std::cerr << "configuration_parse : error while retrieving name "
                        << "of router " << std::endl;
                exit(EXIT_FAILURE);
            }
            string name = string(element["name"]);

            // parsing of router address
            if (!element.contains("ip"))
            {
                std::cerr << "configuration_parse : error while retrieving ip address "
                        << "of router " << std::endl;
                exit(EXIT_FAILURE);
            }

            Ipv4Address ip = Ipv4Address(string(element["ip"]).c_str());

            // parsing of router netmask
            if (!element.contains("netmask"))
            {
                std::cerr << "configuration_parse : error while retrieving netmask "
                        << "of router " << std::endl;
                exit(EXIT_FAILURE);
            }
            Ipv4Mask netmask = Ipv4Mask(string(element["netmask"]).c_str());


            // create the NS3 node and install the internet stack
            NodeContainer node;
            node.Create(1);
            internet.Install(node);
            // convert the string ASx to int x
            int ASnum = std::stoi(name.substr(2, name.size()-2));
            // router id = IP address
            Router router = Router(ASnum, node, ip, netmask);

            // Now we add the links
            if (element.contains("links"))
            {
                for (auto& e_link : element["links"])
                {
                    // convert the string ASx to int x 
                    // x is the AS number of the neighbour
                    int neighNum = std::stoi(((std::string)e_link).substr(2, e_link.size()-2));
                    router.add_neighbour(neighNum);  
                }
            }

            routers.push_back(router);

            // print the router info
            std::cout << "Router " << ASnum << " created, with IP: " << ip
                << ", and netmask: " << netmask << std::endl;
                
        }  
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return routers;
}

