#ifndef INTERFACE_H
#define INTERFACE_H

#include <fstream>
#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>

using namespace std;

class Interface {
    public:
        Interface(std::string name, std::string IP, std::string mask);
            
        void set_status(bool status);
        std::string get_int_name();
        std::string get_int_IP();
        std::string get_int_mask();

    private:
        std::string name;
        std::string IP_addr;
        std::string mask;
        bool status;

};

#endif