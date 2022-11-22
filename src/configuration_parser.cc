#include "../include/configuration_parser.h"

#include "../include/json.hpp"

#include <fstream>
#include <iostream>

using json = nlohmann::json;
using namespace std;

std::vector<AS>
load_configuration()
{
    std::vector<AS> routers;
    std::ifstream f(string(filesystem::current_path()) +
                    "/scratch/PPS_project/configuration_file.json");
    try
    {
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
            string ip = string(element["ip"]);

            // parsing of router names
            if (!element.contains("netmask"))
            {
                std::cerr << "configuration_parse : error while retrieving netmask "
                          << "of router " << std::endl;
                exit(EXIT_FAILURE);
            }
            string netmask = string(element["netmask"]);

            AS router = AS(name, ip, netmask);

            // Now we add the links
            if (element.contains("links"))
            {
                for (auto& e_link : element["links"])
                {
                    router.add_link(e_link);
                }
            }
            routers.push_back(router);
            cout << "Router  " << name << " created, with IP: " << ip
                 << ", and netmask: " << netmask << ".\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return routers;
}
