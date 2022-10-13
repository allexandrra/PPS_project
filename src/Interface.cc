#include "Interface.h"

Interface::Interface(std::string name, std::string IP, std::string mask) {
    this->name = name;
    this->IP_addr = IP;
    this->mask = mask;
}   

void Interface::set_status(bool status) { this->status = status; }
std::string Interface::get_int_name() { return this->name; }
std::string Interface::get_int_IP() { return this->IP_addr; }
std::string Interface::get_int_mask() { return this->mask; }