#ifndef PEER_H
#define PEER_H

#include <string.h>
#include <stdint.h>
#include <vector.h>

namespace Peer {
    class Peer {
        public:
            Peer(string nw, string nh, string path);
            
            string get_peer_nw();
            string get_peer_nh();
            string get_peer_path();

        private:
            string network;
            string next_hop;
            string path;
    };
}

#endif