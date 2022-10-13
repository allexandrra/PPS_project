#include "Peer.h"

Peer::Peer(std::string nw, std::string nh, std::string path) {
    this->network = nw;
    this->next_hop = nh;
    this->path = path;
}

std::string Peer::get_peer_nw() { return this->network; }
std::string Peer::get_peer_nh() { return this->next_hop; }
std::string Peer::get_peer_path() { return this->path; }