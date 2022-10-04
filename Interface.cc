#include "Interface.h"

using namespace std;
using std::vector;

namespace Interface {
    Interface::Interface(string name, string IP, string mask) {
        this->name = name;
        this->IP_addr = IP;
        this->mask = mask;
    }   

    void Interface::set_status(bool status) { this->status = status; }
    string Interface::get_int_name() { return this->name; }
    string Interface::get_int_IP() { return this->IP_addr; }
    string Interface::get_int_mask() { return this->mask; }
}