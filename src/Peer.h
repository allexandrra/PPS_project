#ifndef PEER_H
#define PEER_H

#include <fstream>
#include <iostream>
#include <sstream> 
#include <iomanip>
#include <cstring>

using namespace std;

class Peer {
    public:
        Peer(std::string nw, std::string nh, std::string path);
            
        std::string get_peer_nw();
        std::string get_peer_nh();
        std::string get_peer_path();

    private:
        std::string network;
        std::string next_hop;
        std::string path;
};

#endif