#include <iostream>

#include "../include/libconfig.h++"
#include "../include/configuration_parser.h"

using namespace libconfig;
using namespace std;

std::vector<AS> load_configuration()
{
    Config cfg;
    std::vector<AS> routers;

    /* Reading the whole configuration file and checking for structure errors */
    try
    {
        cfg.readFile("./configuration_file.cfg");
    }
    catch (const FileIOException &fioex)
    {
        std::cerr << "configuration_parser : I/O error while reading file."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    catch (const ParseException &pex)
    {
        std::cerr << "configuration_parser : Parse error at " << pex.getFile()
                  << ":" << pex.getLine()
                  << " - " << pex.getError() << std::endl;
        exit(EXIT_FAILURE);
    }

    /* Extracting data from cf */
    const Setting &root = cfg.getRoot();
    try
    {
        const Setting &as_list = root["network"];
        int as_count = as_list.getLength();

        for (int i = 0; i < as_count; i++)
        {
            const Setting &as_item = as_list[i];
            string name;
            vector<string> links;

            //parsing of router names
            if (!as_item.lookupValue("name", name))
            {
                std::cerr << "configuration_parse : error while retrieving "
                          << "of router" << std::endl;
                exit(EXIT_FAILURE);
            }
            AS router = AS(name);

            // Parsing of router links
            const Setting &link_settings = as_item.lookup("links");
            for (int j = 0; j < link_settings.getLength(); j++)
            {
                links.push_back(link_settings[j]);
            }
            router.set_links(links);

            routers.push_back(router);
            cout << "> Autonomous system  \"" << name << "\" created" << endl;
        }
        
    }
    catch (const SettingNotFoundException &nfex)
    {
        std::cerr << "configuration_parser : couldn't retrieve list of AS in"
                  << "configuration file." << std::endl;
    }

    return routers;
}