#ifndef CONFIGURATION_PARSER_H
#define CONFIGURATION_PARSER_H
    
#include "Router.h"

/**
 * @brief Load the configuration from a JSON file and creates the routers with the information read
 * @return Vector of routers created based on the specs of the configuration file
 */
std::vector<Router> load_configuration();

#endif