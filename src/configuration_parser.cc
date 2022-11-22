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


std::vector<Router> load_configuration() {
    //std::vector<AS> routers;
    std::vector<Router> routers;
    std::ifstream f(string(filesystem::current_path()) +
                    "/scratch/PPS_project/configuration_file.json");
    InternetStackHelper internet;

    try
    {
        //std::cout << "Loading configuration file" << std::endl;
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

            // parsing of router names
            if (!element.contains("netmask"))
            {
                std::cerr << "configuration_parse : error while retrieving netmask "
                        << "of router " << std::endl;
                exit(EXIT_FAILURE);
            }
            Ipv4Mask netmask = Ipv4Mask(string(element["netmask"]).c_str());

            //AS router = AS(name, ip, netmask);

            NodeContainer node;
            node.Create(1);
            internet.Install(node);
            int ASnum = std::stoi(name.substr(2, name.size()-2));
            // TODO: discuss about router_id
            Router router = Router(ASnum, ASnum, node, ip, netmask);

            // Now we add the links
            if (element.contains("links"))
            {
                for (auto& e_link : element["links"])
                {
                    //router.add_link(e_link);
                    int neighNum = std::stoi(((std::string)e_link).substr(2, e_link.size()-2));
                    router.add_neighbour(neighNum);  
                }
            }

            routers.push_back(router);
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

