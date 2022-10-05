#include "Peer.h"

using namespace std;
using std::vector;

namespace Peer {

    Peer::Peer(string nw, string nh, string path) {
        this->network = nw;
        this->next_hop = nh;
        this->path = path;
    }

        string Peer::get_peer_nw() { return this->network; }
        string Peer::get_peer_nh() { return this->next_hop; }
        string Peer::get_peer_path() { return this->path; }
};