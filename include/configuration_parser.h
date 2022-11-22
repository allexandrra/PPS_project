#ifndef CONFIGURATION_PARSER_H
#define CONFIGURATION_PARSER_H
    
    #include <vector>
    #include "AS.h"
    #include "Router.h"

    /**
     * @brief Load configuration file and return a structure of parser parameters
     * 
     */
    std::vector<Router> load_configuration();

#endif